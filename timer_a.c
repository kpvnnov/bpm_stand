// $Id: timer_a.c,v 1.9 2003-05-22 20:00:27 peter Exp $
#include  <msp430x14x.h>
#include <stdlib.h>
#include "global.h"


//ADC
extern int end_adc_conversion;
extern int error_adc;
extern unsigned int results[];         // Needs to be global in this example

int gradus_to_show;
int volt_to_show;
int capture_timer;

int change_to_mode;
	// 0 - ACLK 1 - SMCLK
int mode_timer;
int counter_timer;
int refresh_timer;
int sub_counter_timer;
	//�� �६� �� �ਭ����
time_in GlobalTime;
int second_point;
int invert;
	//�᫨ �� 0, � ����᪠��� �� ������ ᪮���
int run_full_speed;
	//��४��砥��� �� ᪮��⭮� ०�� ⠩���
int switch_speed_timer;

// ��ᯫ��
int mode_display;
	//�� �६� � �ଠ� long ��� ������ �� ��������
time_in time_to_show;
int symbl[4];
int displ[4];

extern int mode_work;
extern long time_to_change;



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
 	//��頥� ⠩��� � �⠢�� ���筨��� ACLK
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

int power_good(void){
return 1;
}
interrupt[TIMERA0_VECTOR] void timer_a_0 (void)
{
 P1OUT |= 0x01;                      // Set P1.0 LED on
 switch(mode_timer){
  case 0: //ࠡ�� �� ACLK (c�� ⠩��� �� ACLK)
   show_display(0x01);
	//��� WatchDog
   WDTCTL = (WDTCTL&0x00FF)+WDTPW+WDTCNTCL;
   counter_timer++;
   update_display=1;
   if (counter_timer>3){
    GlobalTime++;
    counter_timer-=4;
    second_point^=0xFFFF;
    }
   if (switch_speed_timer){
    mode_timer=1;
    TACTL&=~(MC0|MC1); //stop
    TACTL=ID_0|TASSEL_2;
    CCR0=5266;	 //1400,07595898 �� 0x1492(5266)
    TACTL|=MC_1;
    }
   break; //case 0
  case 1: //1400 �� (��� ⠩��� �� SMCLK)
   //  BCSCTL2=(BCSCTL2&(~(SELM0)))|SELM1;
   if (capture_timer>=70){
    capture_timer-=70;
    if (end_adc_conversion){
     end_adc_conversion=0;
     ADC12CTL0 |= ADC12SC;                 // Start conversion
     }
    else {error_adc=1;}
    }
   capture_timer++;
   if (refresh_timer>139){
    show_display(0x00);
	//��� WatchDog
    WDTCTL = (WDTCTL&0x00FF)+WDTPW+WDTCNTCL;
    refresh_timer-=140;
    update_display=1;
    }
   refresh_timer++;
   sub_counter_timer++;  // 1400 �� (����⥫� 5266)
   if (sub_counter_timer>(1399)){
    GlobalTime++;
    sub_counter_timer-=1400;
    second_point^=0xFFFF;
     }
   break; //case 1 - 1400 ��
   }
 _BIC_SR_IRQ(CPUOFF);             // Clear CPUOFF bits from 0(SR)
 P1OUT &= ~0x01;                     // Reset P1.0 LED off
} 
interrupt[WDT_VECTOR] void wd_int (void)
{
 _BIC_SR_IRQ(CPUOFF);             // Clear CPUOFF bits from 0(SR)
 show_display(0x00);
 counter_timer++;
 if (counter_timer>3){
  GlobalTime++;
  counter_timer-=4;
  second_point^=0xFFFF;
  }
} 


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


void tick_timer(void){
int forcedly=0;
 if (change_to_mode!=mode_display){
  forcedly=1;
  mode_display=change_to_mode;
  }
 switch(mode_display){
  case 0:
   redraw_display_second(forcedly);
   break;
  case 1:
   redraw_display_minutes(forcedly);
   break;
  case 2:
   redraw_display_voltage(forcedly);
   break;
  case 3:
   redraw_display_celciy(forcedly);
   break;
  }
}


//���樠������ SPI1
//regim 0x01 - ACLK, ���� SMCLK
// 
void init_spi1_master(int regim){

 ME2=USPIE1; //����砥� SPI

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
 UTCTL1=CKPH|STC|(regim&0x01 ? SSEL0:SSEL0|SSEL1);


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

//���뫠�� �ணࠬ��� SPI1
void send_one_byte_displ(u8 data){
int x;
for (x=0;x<8;x++){
 if (data&0x80)
  P5OUT|=BIT1;
 else
  P5OUT&=~BIT1;
 P5OUT|=BIT3;
 P5OUT&=~BIT3;
 data<<=1;
 }
}


#define cs_on_display()	 P4OUT|=BIT7
#define cs_off_display() P4OUT&=~BIT7


//���뫠�� displ �� SPI � ��������
void show_display(int regim){
int x;
 invert^=0xFFFF;
 if (second_point & (mode_display==1)) 
  displ[1]|=0x80;
 else
  displ[1]&=~0x80;
/*
���� ࠡ��� � ��ᯫ��� �१ ������� SPI1
 init_spi1_master(regim);
 cs_on_display();
 for (x=0;x<4;x++){
  while ((IFG2&UTXIFG1)==0) ;

  TXBUF1=invert ? displ[x]:displ[x]^0xFF;
  }
 while ((UTCTL1&TXEPT)==0) ;
 cs_off_display();
*/
// ���� ࠡ��� � ��ᯫ��� �१ ����� SPI1 
 cs_on_display();
 for (x=0;x<4;x++)
  send_one_byte_displ(invert ? displ[x]:displ[x]^0xFF);
 cs_off_display();

}

const char symbols[]={
                //                                 GFEDCBA
      	0x3F,	// 0 ���ᠭ�� ᨬ���� 0           00111111    A
	0x06,	// 1 ���ᠭ�� ᨬ���� 1           00000110  F   B
	0x5B,   // 2 ���ᠭ�� ᨬ���� 2           01011011    G
	0x4F,   // 3 ���ᠭ�� ᨬ���� 3           01001111  E   C
	0x66,   // 4 ���ᠭ�� ᨬ���� 4           01100110    D
	0x6D,   // 5 ���ᠭ�� ᨬ���� 5           01101101
	0x7D,   // 6 ���ᠭ�� ᨬ���� 6           01111101
	0x07,   // 7 ���ᠭ�� ᨬ���� 7           00000111
	0x7F,   // 8 ���ᠭ�� ᨬ���� 8           01111111
	0x6F,   // 9 ���ᠭ�� ᨬ���� 9           01101111
	0x77,   // A ���ᠭ�� ᨬ���� �           01110111
	0x7C,   // B ���ᠭ�� ᨬ���� b           01111100
	0x39,   // C ���ᠭ�� ᨬ���� C           00111001
	0x5E,   // D ���ᠭ�� ᨬ���� d           01011110
	0x79,   // E ���ᠭ�� ᨬ���� E           01111001
	0x71,   // F ���ᠭ�� ᨬ���� F           01110001
	0x00,   //10 ���ᠭ�� ᨬ���� "�����"   00000000
	0x01,   //11 ���ᠭ�� ᨬ���� (����.��) 00000001
	0x40,   //12 ���ᠭ�� ᨬ���� (�।.��) 01000000
	0x08,   //13 ���ᠭ�� ᨬ���� (����.��) 00001000
	0x73,   //14 ���ᠭ�� ᨬ���� P           01110011
	0x38,   //15 ���ᠭ�� ᨬ���� L           00111000
	0x37,   //16 ���ᠭ�� ᨬ���� �           00110111
	0x6E    //17 ���ᠭ�� ᨬ���� �           01101110
	};

//�८�ࠧ�� symbl � ᮮ⢥��⢨� � symbols � displ
void update_diplay(void){
 displ[3]=symbols[symbl[0]];
 displ[2]=symbols[symbl[1]];
 displ[1]=symbols[symbl[2]];
 displ[0]=symbols[symbl[3]];
}






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
void make_view_time_vol(int volt){

 
 symbl[0]=volt%10;
 volt/=10;
 symbl[1]=volt%10;
 volt/=10;
 symbl[2]=volt%10;
 volt/=10;
 symbl[3]=volt%10;

// symbl[1]=results[0]%10;
// results[1]/=10;
// symbl[2]=results[0]%10;
// results[2]/=10;
// symbl[3]=results[0]%10;
 update_diplay();
}
void make_view_time_celciy(int adc_result){
int DegC;
int abs_C;
//   DegC = ((((long)adc_result-1615)*704)/4095);
   abs_C = ((((long)adc_result-2692)*4225)/4095);
   DegC= abs(abs_C);
// DegC=adc_result;
 symbl[0]=DegC%10;
 DegC/=10;
 symbl[1]=DegC%10;
 DegC/=10;
 symbl[2]=DegC%10;
 if (abs_C<0)
  symbl[3]=0x12;
 else 
  symbl[3]=0x10;
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
  if (force || (results[0]!=volt_to_show)){
   volt_to_show=results[0];
   make_view_time_vol(volt_to_show);
   }
}
void redraw_display_celciy(int force){
  if (force || (results[4]!=gradus_to_show)){
   gradus_to_show=results[4];
   make_view_time_celciy(gradus_to_show);
   }
}

void work_with_display(void){
    switch(mode_work){
     case 0: //�뢮� �� �࠭ �६��� (������ ᥪ㭤�)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode++;
       mode_work++;
       time_to_change=GlobalTime;
       }
      break;
     case 1: //�뢮� �� �࠭ �६��� (��� ������)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode++;
       mode_work++;
       time_to_change=GlobalTime;
       }
      break;
     case 2: //�뢮� �� �࠭ ����殮��� (0-4095)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode++;
       mode_work++;
       time_to_change=GlobalTime;
       }
      break;
     case 3: //�뢮� �� �࠭ ⥬������� (�ࠤ��� 楫���)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode=0;
       mode_work=0;
       time_to_change=GlobalTime;
       }
      break;
     }
}

// WDT

//���樠������ ��� ���室� � ०�� power_down
void init_wdt(void){
  // ����⥫� �ᮢ��� ����� = 1
  BCSCTL1 = (BCSCTL1& (~(DIVA0|DIVA1)) );// ACLK/1
  // ���ࢠ� �ࠡ��뢠��� ���뢠��� WDT �⠢�� ࠢ�� 1 ᥪ
  WDTCTL = WDT_ADLY_250;                // WDT 1s interval timer
  IE1 |= WDTIE;                         // Enable WDT interrupt

}
