//******************************************************************************
//  MSP-FET430X110 Demo - Demonstrate LPM3, WDT Interrupt   
//
//  Description; This program operates MSP430 normally in LPM3, pulsing P1.0 
//  at 4 second intervals. WDT ISR used to wake-up system. All I/O configured 
//  as low outputs to eliminate floating inputs. Current consumption does 
//  increase when LED is powered on P1.0. Demo for measuring LPM3 current. 
//  //*External watch crystal installed on XIN XOUT is required for ACLK*//	  
//
//
//           MSP430F1121
//         ---------------
//     /|\|            XIN|-  
//      | |               | 32kHz
//      --|RST        XOUT|-
//        |               |
//        |           P1.0|-->LED
//    
//  M. Buccini
//  Texas Instruments, Inc
//  January 2002
//******************************************************************************

//#include <msp430x11x1.h>
//#include  <msp430x13x.h>
#include  <msp430x14x.h>
#include "global.h"

#define disable_int_no_interrupt() _DINT()
#define enable_int_no_interrupt() _EINT()

static unsigned int results[4];         // Needs to be global in this example
                                        // Otherwise, the compiler removes it
                                        // because it is not used for anything.


//инициализация SPI1
void init_spi1_master(void){

 ME2=USPIE1; //включаем SPI

// Bit 0: Master mode:
// The transmitter-empty flag TXEPT is set when the transmitter
// shift register and UxTXBUF are empty, and reset when data are
// written to UxTXBUF. It is set again by a SWRST.
// Slave mode:
// The transmitter-empty flag TXEPT is not set when the transmitter
// shift register and UxTXBUF are empty.
// Bit 1: The slave transmit-control bit STC selects if the STE pin is used
// for master and slave mode:
// STC = 0: The four-pin mode of SPI is selected. The STE
// signal is used by the master to avoid bus conflicts,
// or is used in slave mode to control transmit and
// receive enable.
// STC = 1: The three-pin SPI mode is selected. STE is not
// used in master or slave mode.
// Bit 2: Unused
// Bit 3: Unused
// Bits 4, 5: Source select 0 and 1
// The source-select bits define which clock source is used for
// baud-rate generation only when master mode is selected:
// SSEL1,SSEL0 0 External clock UCLK selected
// 1 Auxiliary clock ACLK selected
// 2, 3 SMCLK
// In master mode (MM = 1), an external clock at UCLK cannot be
// selected since the master supplies the UCLK signal for any
// slave. In slave mode, bits SSEL1 and SSEL0 are not relevant.
// The external clock UCLK is always used.
// Bits 6, 7: Clock polarity CKPL and clock phase CKPH
// The CKPL bit controls the polarity of the SPICLK signal.
// CKPL = 0: The inactive level is low; data is output with the
// rising edge of UCLK; input data is latched with
// the falling edge of UCLK.
// CKPL = 1: The inactive level is high; data is output with the
// falling edge of UCLK; input data is latched with
// the rising edge of SPICLK.
// The CKPH bit controls the polarity of the SPICLK signal as
// shown in Figure 13-17.
// CKPH = 0: Normal UCLK clocking scheme
// CKPH = 1: UCLK is delayed by one half cycle
 UTCTL1=CKPH|SSEL0|STC;


// Bit 0: The USART state machines and operating flags are initialized
// to the reset condition (URXIFG=USPIIE=0, UTXIFG=1) if the
// software reset bit is set. Until the SWRST bit is reset, all affected
// logic is held in the reset state. This implies that after a system
// reset the USART must be reenabled by resetting this bit.
// Note:
// The USART initialization sequence should be:
// - Initialize per application requirements while leaving SWRST=1
// - Clear SWRST
// - Enable interrupts if desired.
// Bit 1: Master mode is selected when the MM bit is set. The USART
// module slave mode is selected when the MM bit is reset.
// Bit 2: Peripheral module mode select
// The SYNC bit sets the function of the USART peripheralinterface
// module. Some of the USART control bits have different
// functions in UART and SPI modes.
// SYNC = 0: UART function is selected
// SYNC = 1: SPI function is selected
// Bit 3: The listen bit determines the transmitted data to feed back
// internally to the receiver. This is commonly called loopback
// mode.
// Bit 4: Character length
// This register bit sets the length of the character to be transmitted
// as either seven or eight bits.
// CHAR = 0: 7-bit data
// CHAR = 1: 8-bit data
// Bit 5: Unused
// Bit 6: Unused
// Bit 7: Unused 

 UCTL1=CHAR|SYNC|MM;

 U1BR0=2;
 U1BR1=0;
 UMCTL0=0;
}

int symbl[4];
int displ[4];
const char symbols[]={
      	0x3F,	// 0 Описание символа 0           00111111    A
	0x06,	// 1 Описание символа 1           00000110  F   B
	0x5B,   // 2 Описание символа 2           01011011    G
	0x4F,   // 3 Описание символа 3           01001111  E   C
	0x66,   // 4 Описание символа 4           01100110    D
	0x6D,   // 5 Описание символа 5           01101101
	0x7D,   // 6 Описание символа 6           01111101
	0x07,   // 7 Описание символа 7           00000111
	0x7F,   // 8 Описание символа 8           01111111
	0x6F,   // 9 Описание символа 9           01101111
	0x00,   // A Описание символа "Пустота"   00000000
	0x01,   // B Описание символа (верх.тире) 00000001
	0x40,   // C Описание символа (сред.тире) 01000000
	0x08,   // D Описание символа (нижн.тире) 00001000
	0x73,   // E Описание символа P           01110011
	0x79,   // F Описание символа E           01111001
	0x3F,   //10 Описание символа O           00111111
	0x76,   //11 Описание символа E           01110110
	0x71,   //12 Описание символа F           01110001
	0x38,   //13 Описание символа F           00111000
	0x37,   //14 Описание символа П           00110111
	0x77,   //15 Описание символа А           01110111
	0x6E   //16 Описание символа У           01101110
	};

//преобразует symbl в соответствии с symbols в displ
void update_diplay(void){
 displ[3]=symbols[symbl[0]];
 displ[2]=symbols[symbl[1]];
 displ[1]=symbols[symbl[2]];
 displ[0]=symbols[symbl[3]];
}

void cs_on_display(void){
P4OUT|=BIT7;
}
void cs_off_display(void){
P4OUT&=~BIT7;
}
//высылает displ по SPI в индикатор
int invert;
void show_display(void){
int x;
 invert^=0xFFFF;
 init_spi1_master();
 cs_on_display();
 for (x=0;x<4;x++){
  while ((IFG2&UTXIFG1)==0) ;
  TXBUF1=invert ? displ[x]:displ[x]^0xFF;
  }
 while ((UTCTL1&TXEPT)==0) ;
 cs_off_display();
}
struct tm_in {
	int sec;
	int min;
	int hour;
	int day;
};

typedef long time_in ;

#define SECS_IN_MIN (time_in)60
#define MINS_IN_HR  (time_in)60
#define HRS_IN_DAY  (time_in)24
#define SECS_IN_HR  (SECS_IN_MIN * MINS_IN_HR)
#define SECS_IN_DAY (SECS_IN_HR * HRS_IN_DAY)


time_in mktime_in(struct tm_in *t){
 time_in result =
  t->day  * SECS_IN_DAY   +
  t->hour * SECS_IN_HR    +
  t->min  * SECS_IN_MIN   +
  t->sec;
 return result;
}
void local_time(time_in t1,struct tm_in *t2){
unsigned int t;
 t2->day=0;
 while (t1>SECS_IN_DAY){
  t2->day++;
  t1-=SECS_IN_DAY;
  }
 t2->sec=t1&0x0001;
 t=t1>>1;

 t2->hour=t / 1800;
 t=t - t2->hour*1800;
 t2->min= t / 30;
 t2->sec=t2->sec + (t<<1) - t2->min*60;

}
//это время в формате long для показа на индикатор
time_in time_to_show;
//это время по гринвичу
time_in GlobalTime=0;
int counter_timer;
int refresh_timer;
int sub_counter_timer;

void make_view_time_min(time_in to_show){
 struct tm_in decoded;
 local_time(to_show,&decoded);
 symbl[0]=decoded.min % 10;
 symbl[1]=decoded.min / 10;
 symbl[2]=decoded.hour% 10;
 symbl[3]=decoded.hour/ 10;
 update_diplay();
}
void make_view_time_sec(time_in to_show){
 struct tm_in decoded;
 local_time(to_show,&decoded);
 symbl[0]=decoded.sec % 10;
 symbl[1]=decoded.sec / 10;
 symbl[2]=decoded.min % 10;
 symbl[3]=decoded.min / 10;
 update_diplay();
}
void make_view_time_vol(void){

int res;
 res=results[0];
 
 symbl[0]=res%10;
 res/=10;
 symbl[1]=res%10;
 res/=10;
 symbl[2]=res%10;
 res/=10;
 symbl[3]=res%10;

// symbl[1]=results[0]%10;
// results[1]/=10;
// symbl[2]=results[0]%10;
// results[2]/=10;
// symbl[3]=results[0]%10;
 update_diplay();
}

void redraw_display_second(int force){
  if (force || GlobalTime!=time_to_show){
   time_to_show=GlobalTime;
   make_view_time_sec(time_to_show);
   }
}
void redraw_display_minutes(int force){
  if (force || (GlobalTime>>2)!=(time_to_show>>2)){
   time_to_show=GlobalTime;
   make_view_time_min(time_to_show);
   }
}
void redraw_display_voltage(int force){
  if (force || (GlobalTime!=time_to_show)){
   time_to_show=GlobalTime;
   make_view_time_vol();
   }
}


int change_to_mode;
int mode_display;

void tick_timer(void){
int forcedly=0;
 if (change_to_mode!=mode_display){
  forcedly=1;
  mode_display=change_to_mode;
  }
 switch(mode_display){
  case 0:
   redraw_display_voltage(forcedly);
//   redraw_display_second(forcedly);
   break;
  case 1:
   redraw_display_minutes(forcedly);
   break;
  case 2:
   redraw_display_second(forcedly);
//   redraw_display_voltage(forcedly);
   break;
//  case 3:
//   redraw_display_second(forcedly);
//   redraw_display_voltage(forcedly);
//   break;
  }
}


//Инициализация для перехода в режим power_down
void init_wdt(void){
  // делитель часового кварца = 1
  BCSCTL1 = (BCSCTL1& (~(DIVA0|DIVA1)) );// ACLK/1
  // интервал срабатывания прерываний WDT ставим равным 1 сек
  WDTCTL = WDT_ADLY_250;                // WDT 1s interval timer
  IE1 |= WDTIE;                         // Enable WDT interrupt

}

//возвращаем 1, если часовой кварц работает
int test_run_LFXT1CLK(void){
int i;
 for (i = 100; i>0; i--);           // Delay
 return 1;
}

void init_timer_a(void){
 // Bit 0: TAIFG: This flag indicates a timer overflow event.
 // Up mode: TAIFG is set if the timer counts from CCR0
 // value to 0000h.
 // Continuous mode: TAIFG is set if the timer counts from
 // 0FFFFh to 0000h.
 // Up/down mode: TAIFG is set if the timer counts down from
 // 0001h to 0000h.
 // Bit 1: Timer overflow interrupt enable (TAIE) bit. An interrupt request from
 // the timer overflow bit is enabled if this bit is set, and is disabled if
 // reset.
 // Bit 2: Timer clear (CLR) bit. The timer and input divider are reset with the
 // POR signal, or if bit CLR is set. The CLR bit is automatically reset
 // and is always read as zero. The timer starts in the upward direction
 // with the next valid clock edge, unless halted by cleared mode
 // control bits.
 // Bit 3: Not used
 // Bits 4, 5: Mode control: Table 10-4 describes the mode control bits.
 // 0 0 Stop Timer is halted.
 // 0 1 Up to CCR0 Timer counts up to CCR0 and restarts at 0.
 // 1 0 Continuous up Timer counts up to 0FFFFh and restarts at 0.
 // 1 1 Up/down Timer continuously counts up to CCR0 and back
 // down to 0.
 // Bits 6, 7: Input divider control bits. Table 10-5 describes the input divider
 // control bits.
 // 0 0 /1 Input clock source is passed to the timer.
 // 0 1 /2 Input clock source is divided by two.
 // 1 0 /4 Input clock source is divided by four.
 // 1 1 /8 Input clock source is divided by eight.
 // Bits 8, 9: Clock source selection bits. Table 10-6 describes the clock source
 // selections.
 // 0 0 TACLK See data sheet device description.
 // 0 1 ACLK Auxiliary clock ACLK is used.
 // 1 0 SMCLK System clock SMCLK.
 // 1 1 INCLK See device description in data sheet.
 	//очищаем таймер и ставим источником ACLK
 TACTL=TASSEL0+TACLR;


 // Bit 0: Capture/compare interrupt flag CCIFGx
 // Capture mode:
 // If set, it indicates that a timer value was captured in the
 // CCRx register.
 // Compare mode:
 // If set, it indicates that a timer value was equal to the data
 // in the CCRx register.
 // CCIFG0 flag:
 // CCIFG0 is automatically reset when the interrupt request
 // is accepted.
 // CCIFG1 to CCIFG4 flags:
 // The flag that caused the interrupt is automatically reset
 // after the TAIV word is accessed. If the TAIV register is not
 // accessed, the flags must be reset with software.
 // No interrupt is generated if the corresponding interrupt
 // enable bit is reset, but the flag will be set. In this scenario,
 // the flag must be reset by the software.
 // Setting the CCIFGx flag with software will request an
 // interrupt if the interrupt-enable bit is set.
 // Bit 1: Capture overflow flag COV
 // Compare mode selected, CAP = 0:
 // Capture signal generation is reset. No compare event will
 // set COV bit.
 // Capture mode selected, CAP = 1:
 // The overflow flag COV is set if a second capture is
 // performed before the first capture value is read. The
 // overflow flag must be reset with software. It is not reset by
 // reading the capture value.
 // Bit 2: The OUTx bit determines the value of the OUTx signal if the
 // output mode is 0.
 // Bit 3: Capture/compare input signal CCIx:
 // The selected input signal (CCIxA, CCIxB, VCC. or GND) can be
 // read by this bit. See Figure 10-18.
 // Bit 4: Interrupt enable CCIEx: Enables or disables the interrupt
 // request signal of capture/compare block x. Note that the GIE bit
 // must also be set to enable the interrupt.
 // 0: Interrupt disabled
 // 1: Interrupt enabled
 // Bits 5 to 7: Output mode select bits:
 // Table 10-7 describes the output mode selections.
 // 0 Output only The OUTx signal reflects the value of the OUTx bit
 // 1 Set EQUx sets OUTx
 // 2 PWM toggle/reset EQUx toggles OUTx. EQU0 resets OUTx.
 // 3 PWM set/reset EQUx sets OUTx. EQU0 resets OUTx
 // 4 Toggle EQUx toggles OUTx signal.
 // 5 Reset EQUx resets OUTx.
 // 6 PWM toggle/set EQUx toggles OUTx. EQU0 sets OUTx.
 // 7 PWM reset/set EQUx resets OUTx. EQU0 sets OUTx.
 // Note: OUTx updates with rising edge of timer clock for all modes except mode 0.
 // Modes 2, 3, 6, 7 not useful for output unit 0.
 // Bit 8: CAP sets capture or compare mode.
 // 0: Compare mode
 // 1: Capture mode
 // Bit 9: Read only, always read as 0.
 // Bit 10: SCCIx bit:
 // The selected input signal (CCIxA, CCIxB, VCC, or GND) is
 // latched with the EQUx signal into a transparent latch and can be
 // read via this bit.
 // Bit 11: SCSx bit:
 // This bit is used to synchronize the capture input signal with the
 // timer clock.
 // 0: asynchronous capture
 // 1: synchronous capture
 // Bits 12, 13: Input select, CCIS0 and CCIS1:
 // These two bits define the capture signal source. These bits are
 // not used in compare mode.
 // 0 Input CCIxA is selected
 // 1 Input CCIxB is selected
 // 2 GND
 // 3 VCC
 // Bits 14, 15: Capture mode bits:
 // Table 10-8 describes the capture mode selections.
 // 0 Disabled The capture mode is disabled.
 // 1 Positive Edge Capture is done with rising edge.
 // 2 Negative Edge Capture is done with falling edge.
 // 3 Both Edges Capture is done with both rising and falling edges.
 CCTL0=CCIE;	// CCR0 interrupt enabled
 CCR0=32768/4; 	//8192=0x2000
 TACTL|=MC0;	// Start Timer_a in upmode
}


//Включаем основной частотой часовой таймер
// если часовой таймер запустился, то:
//  - отключаем XT2 (если mode&0x01)
//  - отключаем DC Generator (SCG0=1 если mode&0x02)
//  - отключаем SMCLK (SCG1=1 если mode&0x04)
// и возвращаем 1 (True) иначе продолжаем работать на внутреннем генераторе
// и возвращаем 0 (False)
int run_LFXT1CLK( int mode){
	//XTS = 0: The low-frequency oscillator is selected.
 if (mode&0x04) _BIS_SR(SCG1);
 BCSCTL1&=~XTS;

 _BIC_SR(OSCOFF);			// Включаем часовой кварц
	//проверяем запуск часового кварца
 if (!test_run_LFXT1CLK()){	//часовой не запустился
  return 0;
  }
	// делитель (ACLK/1)
	// DIVA = 0: 1
	// DIVA = 1: 2
	// DIVA = 2: 4
	// DIVA = 3: 8
	// если запрошено отключение XT2 кварца - отключаем
	// XT2Off = 1: the oscillator is off if it is not used for MCLK or SMCLK.
 BCSCTL1 = (BCSCTL1& (~(DIVA0|DIVA1))) | (mode&0x01 ? XT2OFF:0); 
	// включаем источник основной частоты - LFXT1CLK
	// Bit0, DCOR: The DCOR bit selects the resistor for injecting current into the
	// dc generator. Based on this current, the oscillator operates if
	// activated.
	// DCOR = 0: Internal resistor on, the oscillator can operate. The failsafe
	// mode is on.
	// DCOR = 1: Internal resistor off, the current must be injected
	// externally if the DCO output drives any clock using
	// the DCOCLK.
	// Bit1, Bit2: The selected source for SMCLK is divided by:
	// DIVS.1 .. DIVS.0 DIVS = 0: 1
	// DIVS = 1: 2
	// DIVS = 2: 4
	// DIVS = 3: 8
	// Bit3, SELS: Selects the source for generating SMCLK:
	// SELS = 0: Use the DCOCLK
	// SELS = 1: Use the XT2CLK signal (in three-oscillator systems)
	// or
	// LFXT1CLK signal (in two-oscillator systems)
	// Bit4, Bit5: The selected source for MCLK is divided by:
	// DIVM.0 .. DIVM.1 DIVM = 0: 1
	// DIVM = 1: 2
	// DIVM = 2: 4
	// DIVM = 3: 8
	// Bit6, Bit7: Selects the source for generating MCLK:
	// SELM.0 .. SELM.1 SELM = 0: Use the DCOCLK
	// SELM = 1: Use the DCOCLK
	// SELM = 2: Use the XT2CLK (x13x and x14x devices) or
	// Use the LFXT1CLK (x11xx and x12xx devices)
	// SELM = 3: Use the LFXT1CLK

 BCSCTL2=(BCSCTL2&(~(DIVM0|DIVM1|DIVS0|DIVS1)))|SELM0|SELM1|SELS;
 if (mode&0x02) _BIS_SR(SCG0);
 return 1;
}

void set_pin_directions(void){
  P1DIR = 0xFF;                         // All P1.x outputs
  P1OUT = 0;                            // All P1.x reset
  P2DIR = 0xFF;                         // All P2.x outputs
  P2OUT = 0;                            // All P2.x reset
  P3DIR = 0xFF;                         // All P3.x outputs
  P3OUT = 0;                            // All P3.x reset
  P4DIR = 0xFF;                         // All P4.x outputs
  P4OUT = 0;                            // All P4.x reset
  P5DIR = 0xFF;                         // All P5.x outputs
  P5OUT = 0;                            // All P5.x reset
  P5SEL = 0x0E;
  P6DIR = 0xFF;                         // All P6.x outputs
  P6OUT = 0;                            // All P6.x reset
  P6SEL = 0x0F;                         // Enable A/D channel inputs

}
//если не 0, то запускаемя на полной скорости
int run_full_speed;
int current_speed;
// 0 - ACLK 1 - SMCLK
int current_timer;
//переключаемся на скоростной режим таймера
int switch_speed_timer;

int run_xt2(void){
int i;
int count=10;
  BCSCTL1 &= ~XT2OFF;                   // XT2 = HF XTAL
  do 
  {
  IFG1 &= ~OFIFG;                       // Clear OSCFault flag
  for (i = 0xFF; i > 0; i--);           // Time for flag to set
  count--;
  if (count==0) break;
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?                

  if ((IFG1 & OFIFG)==0){
   BCSCTL2 = (BCSCTL2&(~SELM0))|SELM1;   // MCLK = XT2 (safe)
   current_speed=1;
   }
  return count;
}


void switch_xt2(void){
switch_speed_timer=1;
}

void init_adc(void){
  ADC12CTL0 = ADC12ON+MSC+SHT0_2;       // Turn on ADC12, set sampling time
  ADC12CTL1 = SHP+CONSEQ_1;             // Use sampling timer, single sequence
  ADC12MCTL0 = INCH_0;                  // ref+=AVcc, channel = A0
  ADC12MCTL1 = INCH_1;                  // ref+=AVcc, channel = A1
  ADC12MCTL2 = INCH_2;                  // ref+=AVcc, channel = A2    
  ADC12MCTL3 = INCH_3+EOS;              // ref+=AVcc, channel = A3, end seq.
  ADC12IE = 0x08;                       // Enable ADC12IFG.3
  ADC12CTL0 |= ENC;                     // Enable conversions
}
void init_uart0(void){
  UCTL0 = CHAR;                         // 8-bit character
  UTCTL0 = SSEL0;                       // UCLK = ACLK
  UBR00 = 0x45;                         // 8MHz 115200
  UBR10 = 0x00;                         // 8MHz 115200
  UMCTL0 = 0x00;                        // 8MHz 115200 modulation
  ME1 |= UTXE0 + URXE0;                 // Enable USART0 TXD/RXD
  IE1 |= URXIE0;                        // Enable USART0 RX interrupt
  P3SEL |= 0x30;                        // P3.4,5 = USART0 TXD/RXD
  P3DIR |= 0x10;                        // P3.4 output direction
}
#define  SERIAL_FIFO_RCV_LEN  64           /* size of fifo serial buffer   */
#define  SERIAL_FIFO_TRN_LEN  32           /* size of fifo serial buffer   */

volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
u8    asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */

unsigned int  asp_rcv_fifo_start;      /* serial receive buffer start index      */

volatile unsigned int  asp_rcv_fifo_end;        /* serial receive flash buffer end index        */

u8    asp_rcv_fifo_buf[SERIAL_FIFO_RCV_LEN];           /* storage for receive transmit  buffer      */

u8 write_asp_trn_fifo(u8 data_wr){

 if ( ( (asp_trn_fifo_end+1)&(SERIAL_FIFO_TRN_LEN-1))== (asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)) )
  return 0;
 disable_int_no_interrupt();
 asp_trn_fifo_buf[asp_trn_fifo_end++ & (SERIAL_FIFO_TRN_LEN-1)]=data_wr;

 if ((IFG1 & UTXIFG0) != 0)        // USART0 TX buffer ready?
  TXBUF0 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
 enable_int_no_interrupt();
 return 1;
}

u16 send_serial_massiv(u8* data,u16 len){
u16 counter;
u16 counter1;
u16 t_end;
u16 t_start;
// do {
  t_end=  asp_trn_fifo_end&(SERIAL_FIFO_TRN_LEN-1);
  t_start=asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1);
  if (t_end>=t_start) //очередь еще не перешла через край и хвост впереди
   {
   counter=SERIAL_FIFO_TRN_LEN-t_end;
   if (t_start==0) counter--;
   }
  else
   counter=t_start-t_end-1;
//  if (counter==0) break;
  if (counter==0) return len;
  if (counter>len) counter=len;
  counter1=counter;
  while(counter){
   asp_trn_fifo_buf[t_end++]=*data++;
   counter--;
   }
//  memcpy(&asp_trn_fifo_buf[t_end],data,counter);
  len-=counter1;
//  len-=counter;
  asp_trn_fifo_end+=counter1;
//  asp_trn_fifo_end+=counter;
  disable_int_no_interrupt();
  //если фифошка не пустая и прерывания запрещены, то разрешаем их
  if ( ((asp_trn_fifo_end&(SERIAL_FIFO_TRN_LEN-1))!=(asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)))&&
        ((IFG1 & UTXIFG0) != 0)  ){	//прерывания передачи запрещены?
   TXBUF0 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
   }
  enable_int_no_interrupt();
// }while(len);
return len;
}
u16 read_asp_rcv_fifo(void){ 
 if (asp_rcv_fifo_start==asp_rcv_fifo_end) return 0;
 return (asp_rcv_fifo_buf[asp_rcv_fifo_start++ & (SERIAL_FIFO_RCV_LEN-1)]|0x0100);
}

int end_adc_conversion;
long time_to_change;
void main(void)
{ 
int i;
	// останавливаем watchdog
 WDTCTL=WDTPW|WDTHOLD;  		// Stop WDT
	// конфигурируем ноги ввода вывода
// WDTCTL=WDTHOLD;  		// Stop WDT
 set_pin_directions();
 run_full_speed=0;
 current_speed=0;
 current_timer=0;
 switch_speed_timer=0;
 end_adc_conversion=1;
 change_to_mode=2;
 mode_display=2;
 time_to_change=0;
 invert=0;
	//переходим на работу от часового кварца
 run_LFXT1CLK(0x02);
// init_wdt();

//запускаем таймер A и часы от него
 init_timer_a();
 init_adc();

  _EINT();                              // Enable interrupts
  for (i = 0xFFF; i > 0; i--);           // Time for flag to set

    if (current_speed==0 && run_full_speed && run_xt2()){
     switch_xt2();
     }
  for (i = 0xFF; i > 0; i--);           // Time for flag to set
  
  while(1)
  {
//    BCSCTL2|=SELM0|SELM1;
    _BIS_SR(CPUOFF);                 // входим в режим спячки
    P1OUT |= 0x01;                      // Set P1.0 LED on
    tick_timer();
//    P1OUT |= 0x01;                      // Set P1.0 LED on
    if (current_speed==0 && run_full_speed ) { //&& run_xt2()){
     switch_xt2();
     }
    if (current_speed && end_adc_conversion){
     end_adc_conversion=0;
     ADC12CTL0 |= ADC12SC;                 // Start conversion
//     if ((time_to_change+10)<GlobalTime){
//      if (change_to_mode>=3) 
//       change_to_mode=0; 
//      else 
//       change_to_mode++;
//      time_to_change=GlobalTime;
//      }
     }
    P1OUT &= ~0x01;                     // Reset P1.0 LED off
  }

}

int power_good(void){
return 1;
}

interrupt[TIMERA0_VECTOR] void timer_a_0 (void)
{
 if (current_timer){
//  BCSCTL2=(BCSCTL2&(~(SELM0)))|SELM1;
  	//счет таймера от SMCLK
  if (refresh_timer>(139/2)){
   show_display();
   refresh_timer-=140/2;
   }
  refresh_timer++;
  sub_counter_timer++;  // 1400 Гц (делитель 5266)
  if (sub_counter_timer>(1399)){
   GlobalTime++;
   sub_counter_timer-=1400;

   }
  
  }
 else{
  	//счет таймера от ACLK
  if (switch_speed_timer){
   current_timer=1;
   TACTL&=~(MC0|MC1); //stop
   TACTL=ID_0|TASSEL_2;
   CCR0=5266;	 //1400,07595898 Гц 0x1492(5266)
   TACTL|=MC_1;
   }
  counter_timer++;
  show_display();
  if (counter_timer>3){
   GlobalTime++;
   counter_timer-=4;
   }
  if (power_good()) run_full_speed=1;
  }
 _BIC_SR_IRQ(CPUOFF);             // Clear CPUOFF bits from 0(SR)

} 
interrupt[WDT_VECTOR] void wd_int (void)
{
 _BIC_SR_IRQ(CPUOFF);             // Clear CPUOFF bits from 0(SR)
 show_display();
 counter_timer++;
 if (counter_timer>3){
  GlobalTime++;
  counter_timer-=4;
  }
} 


interrupt[ADC_VECTOR] void ADC12ISR (void)
{
//    P1OUT |= 0x01;                      // Set P1.0 LED on
  results[0] = ADC12MEM0;               // Move results, IFG is cleared
  results[1] = ADC12MEM1;               // Move results, IFG is cleared
  results[2] = ADC12MEM2;               // Move results, IFG is cleared    
  results[3] = ADC12MEM3;               // Move results, IFG is cleared  
//    P1OUT &= ~0x01;                     // Reset P1.0 LED off
  _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
  end_adc_conversion=1;
//  end_adc_conversion=1;
}
interrupt[UART0RX_VECTOR] void usart0_rx (void)
{

//  while ((IFG1 & UTXIFG0) == 0);        // USART0 TX buffer ready?
//  TXBUF0 = ;                      // RXBUF0 to TXBUF0


//так как нет inline, то раскрываем  write_asp_rcv_fifo(RXBUF0);
//u8 write_asp_rcv_fifo(u8 data_wr){
// if ( ( (asp_rcv_fifo_end+1)&(SERIAL_FIFO_RCV_LEN-1))== (asp_rcv_fifo_start&(SERIAL_FIFO_RCV_LEN-1)) )
//  return 0;
// asp_rcv_fifo_buf[asp_rcv_fifo_end++ & (SERIAL_FIFO_RCV_LEN-1)]=data_wr;
// return 1;
//}
 if ( ( (asp_rcv_fifo_end+1)&(SERIAL_FIFO_RCV_LEN-1))!= (asp_rcv_fifo_start&(SERIAL_FIFO_RCV_LEN-1)) )
  asp_rcv_fifo_buf[asp_rcv_fifo_end++ & (SERIAL_FIFO_RCV_LEN-1)]=RXBUF0;

// if (temp_iosr_serial&( (1<<bit_OE)| //Overrun occurs - переполнение приема
//                 (1<<bit_FE)| //framing error
//                 (1<<bit_BI)   //break
//               )
//    ) { 
//  portADTR;
//  portADTR;	//два раза читаем порт, чтобы освободиться от данных
//  rDIOSR&=0x00FF;
//  rDIOSR|=0x2600;	//сброс флагов BI(break interrupt)
//                        //FE(framing error)
//                        //переполнения(OE-overrun error) в IOSR
//  portIOSR=rDIOSR;

}

interrupt[UART0TX_VECTOR] void usart0_tx (void)
{
 if (asp_trn_fifo_start!=asp_trn_fifo_end)
  TXBUF0 =asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
}
interrupt[NMI_VECTOR] void nmi(void)
{
}