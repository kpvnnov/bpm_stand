//********************************************************
// $Id: msp_main.c,v 1.26 2003-11-03 17:01:48 peter Exp $
//********************************************************

//#include <msp430x11x1.h>
//#include  <msp430x13x.h>
#include  <msp430x14x.h>
#include "global.h"
#include "change.log"

static const char* rev=REVISION;

u16 temp_hold;
u16 timer_hold;
u16 timer_sum;
u16 timer_sum_int;
u16 timer_sum_sleep;
u16 sleep;
u16 timer_sum_display;
u16 timer_sum_serial;
u16 timer_sum_adc;
u16 timer_sum_stat;
u16 timer_diff_min;
u16 timer_diff_max;
u16 why_job;

extern u16 analog_on;
extern u16 temperature;
extern u16 stop_adc;
extern u16 send_correction_temperature;


extern unsigned int what_doing;


extern unsigned int  stat_rcv_fifo_start;      /* stat receive buffer start index      */
extern volatile unsigned int  stat_rcv_fifo_end;        /* stat receive  buffer end index        */
extern unsigned int  stat1_rcv_fifo_start;      /* stat receive buffer start index      */
extern volatile unsigned int  stat1_rcv_fifo_end;        /* stat receive  buffer end index        */


#define power_good()  1


// АЦП
extern int end_adc_conversion;
//extern unsigned int results[5];         // Needs to be global in this example
extern int error_adc;
extern unsigned int  adc_rcv_fifo_start;      /* ADC receive buffer start index      */

extern volatile unsigned int  adc_rcv_fifo_end;        /* ADC receive  buffer end index        */


//таймер
extern int change_to_mode;
	// 0 - ACLK 1 - SMCLK
extern int mode_timer;
	//это время по гринвичу
extern time_in GlobalTime;
	//если не 0, то запускаемя на полной скорости
//extern int run_full_speed;
	//переключаемся на скоростной режим таймера
extern int switch_speed_timer;


// дисплей
extern int mode_display;
extern int second_point;
extern int invert;
extern int symbl[4];
extern int update_display;

	//это время в формате long для показа на индикатор
extern time_in time_to_show;



#ifdef DEBUG_SERIAL
extern u16 packet_in_fifo;
extern u16 fifo_trn_depth;

#endif //DEBUG_SERIAL

extern u16 received_packed;




#define ACLK_GO	250	//количество достаточных попыток запуска
//возвращаем не ноль, если часовой кварц работает
int test_run_LFXT1CLK(void){
int c=500;	//общее количество попыток запуска ACLK
int t=0;	//счетчик попыток "удачного бега"
int last_att=9;
int i;
unsigned int to_compare,to_compare1;
 while(c){
	//захватываем значение таймера
  to_compare=TAR;	//TIMER_A_COUNTER;	
	//делаем паузу
  for (i = 100; i>0; i--);           // Delay
	//   \   000C  3C406400          MOV     #100,R12       //2 
	//   \   0010            ?0004:
	//   \   0010  1C93              CMP     #1,R12  	//2
	//   \   0012  0238              JL      (?0003) 	//4
	//   \   0014  3C53              ADD     #-1,R12        //2
	//     60            for (i = 100; i>0; i--);           //
	//   \   0016  FC3F              JMP     (?0004)        //4
	// подсчет числа тактов сделан "эмпирически" 22.05.2003
	//   \   0018            ?0003:                         //=2+(1+2+1+2)*100=602
	// от внутреннего генератора время выполнения:
	// 602/610.000=0,000986885
	// 602/750.000=0,000802667
	// 602/900.000=0,000668889
	// на эмуляторе это задержка показывает 25-26 тактов 32768
	// 25 тактов 32768 = 25/32768=0,000762939453125 сек
	// 26 тактов 32768 = 26/32768=0,000793457 сек
	// минимальное значение таймера, которое, вероятно, можно получить
	// 0,000668889*32768=21,91815475

	//захватываем значение таймера опять
  to_compare1=TAR;	//TIMER_A_COUNTER;
	//рассчитываем разницу
  if (to_compare1>to_compare)
   to_compare=to_compare1-to_compare;
  else
   to_compare=0x2000+to_compare1-to_compare;
	//если таймер бежит, то считаем, что кварц запустился
//  if (to_compare>16) t++; else t=0;
  if (to_compare>2) t++; else t=0;
	//сброс WatchDog
  if (t>ACLK_GO) break;	//считаем, что часовой кварц запустился
  CLEAR_DOG();	
  if ((t>>5)!=last_att){
    last_att=t>>5;
    symbl[3]=0x0A;	//'A'
    symbl[2]=0x0C;	//'C'
    symbl[1]=0x15;	//'L'
    symbl[0]=last_att;	//"псевдономер" попытки выводим на эран
    update_diplay();
    }
  if ((c&0x3F)==0)      show_display();
  c--;
  }
 return c;
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
	// делитель (ACLK/1)
	// DIVA = 0: 1
	// DIVA = 1: 2
	// DIVA = 2: 4
	// DIVA = 3: 8
	// если запрошено отключение XT2 кварца - отключаем
	// XT2Off = 1: the oscillator is off if it is not used for MCLK or SMCLK.
 BCSCTL1 = (BCSCTL1& (~(DIVA0|DIVA1))) | (mode&0x01 ? XT2OFF:0); 

 _BIC_SR(OSCOFF);			// Включаем часовой кварц
	//проверяем запуск часового кварца
 if (!test_run_LFXT1CLK()){	//часовой не запустился
  return 0;
  }
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
	// включаем источник основной частоты - LFXT1CLK
 BCSCTL2=(BCSCTL2&(~(DIVM0|DIVM1|DIVS0|DIVS1)))|SELM0|SELM1|SELS;
 if (mode&0x02) _BIS_SR(SCG0);
 return 1;
}

void set_pin_directions(void){

#ifdef CABLE
  P1DIR = 0xFF;                         // All P1.x outputs
  P1OUT = 0;                            // All P1.x reset
  P2DIR = 0xFF;                         // All P2.x outputs
  P2OUT = 0;                            // All P2.x reset

  P3SEL |= 0xC0;                        // P3.6,7 = USART1 option select
//  P3DIR |= 0x20;                        // P3.6 = output direction
  P3DIR = 0xFF;                         // All P3.x outputs
  P3OUT = 0;                            // All P3.x reset

//  P3SEL |= 0x30;                        // P3.4,5 = USART0 TXD/RXD
//  P3DIR |= 0x10;                        // P3.4 output direction


  P4DIR = 0xFF;                         // All P4.x outputs
  P4OUT = 0;                            // All P4.x reset
  P5DIR = 0xFF;                         // All P5.x outputs
  P5OUT = 0;                            // All P5.x reset
//  P5SEL = 0x0E; SPI1
  P5SEL = 0x70; 			// MCLK, SMCLK,ACLK на вывод
  P6DIR = 0xFF;                         // All P6.x outputs
  P6OUT = 0;                            // All P6.x reset
  P6SEL = 0x0F;                         // Enable A/D channel inputs
#endif //CABLE
#ifdef STEND
  P1DIR = BIT0+BIT1+BIT5+BIT6+BIT7;
  P1OUT = 0;                            // All P1.x reset

  P2DIR = BIT0+BIT1+BIT3+BIT4+BIT5+BIT6+BIT7;
  P2OUT = 0;                            // All P2.x reset

  P3SEL |= BIT4+BIT5;	//USART0 select
  P3DIR = 0xFF;                         // All P3.x outputs
  P3OUT = 0;                            // All P3.x reset

  P4DIR = BIT0+BIT1+BIT2+BIT3+BIT6+BIT7;
  P4OUT = 0;                            // All P4.x reset

  P5DIR = 0xFF;                         // All P5.x outputs
  P5OUT = 0;                            // All P5.x reset
  P5SEL = 0x70; 			// MCLK, SMCLK,ACLK на вывод

  P6DIR = 0x00;                         // All P6.x inputs
  P6OUT = 0;                            // All P6.x reset
  P6SEL = 0xFF;                         // Enable A/D channel inputs
#endif //STEND
analog_on=1;
}
int current_speed;

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





long time_to_change;
int mode_work;

void main(void)
{ 
int i;
	// останавливаем watchdog
 WDTCTL=WDTPW|WDTHOLD;  		// Stop WDT

	//сброс WatchDog
 CLEAR_DOG();	
 

	// конфигурируем ноги ввода вывода
 set_pin_directions();
// run_full_speed=0;
 stop_adc=0;		//АЦП не останавливали
 temperature=0;		//преобразование температуры выключено
 current_speed=0;
 switch_speed_timer=0;
 mode_timer=0;
 mode_display=0;
 GlobalTime=0;
 mode_work=0;
 second_point=0;
 invert=0;
 time_to_show=1;
 change_to_mode=0;
 time_to_change=0;
 stat_rcv_fifo_start=0;
 stat_rcv_fifo_end=0;
 what_doing=0;
 if (IFG1&0x01){	//сработал WATCHDOG
	//то считаем пока это аварийным режимом
	//выключаем ноги от блока USART1
    P3SEL &= ~0xC0;                        // P3.6,7 = USART1 option select
    symbl[3]=0x0D;	//'D'
    symbl[2]=0x0D;	//'D'
    symbl[1]=0x0D;	//'D'
    symbl[0]=0x10;	//' '
    update_diplay();
    while(1){
	//сброс WatchDog
        CLEAR_DOG();	
	//и начинаем этой ногой "мигать"
     for (i = 1000; i>0; i--);           // Delay
     P3OUT^=BIT6;
	//сброс WatchDog
     CLEAR_DOG();	
     for (i = 40; i>0; i--);           // Delay
     P3OUT^=BIT6;
     show_display();
     }
  }
	//запускаем таймер A и часы от него (ACLK)
 init_timer_a();
	//переходим на работу от часового кварца
	//если часовой кварц по каким-то причинам не запустился
 if (!run_LFXT1CLK(0x0)){
	//то считаем пока это аварийным режимом
	//выключаем ноги от блока USART1
    P3SEL &= ~0xC0;                        // P3.6,7 = USART1 option select
    symbl[3]=0x0A;	//'A'
    symbl[2]=0x0C;	//'C'
    symbl[1]=0x15;	//'L'
    symbl[0]=0x10;	//' '
    update_diplay();
    while(1){
	//сброс WatchDog
     CLEAR_DOG();	
	//и начинаем этой ногой "мигать"
     for (i = 1000; i>0; i--);           // Delay
	//сброс WatchDog
     CLEAR_DOG();	
     P3OUT^=BIT6;
     for (i = 1000; i>0; i--);           // Delay
     P3OUT^=BIT6;
     show_display();
     }
  }
// init_wdt();

	/* Watchdog mode -> reset after expired time */
	/* WDT is clocked by fACLK (assumed 32KHz) */
	// #define WDT_ARST_1000       (WDTPW+WDTCNTCL+WDTSSEL)                          
	/* 1000ms  " */
 WDTCTL = WDT_ARST_1000;
 	//сброс WatchDog
 CLEAR_DOG();	

 _BIS_SR(SCG0);
	//переходим на работу от второго кварца
 if (current_speed==0 && power_good() ){
	//если второй кварц по каким-то причинам не запустился
  if (!run_xt2()){
	//то считаем пока это аварийным режимом
	//выключаем ноги от блока USART1
    P3SEL &= ~0xC0;                        // P3.6,7 = USART1 option select
    symbl[3]=0x0C;	//'C'
    symbl[2]=0x15;	//'L'
    symbl[1]=0x02;	//'2'
    symbl[0]=0x10;	//' '
    update_diplay();
    while(1){
	//сброс WatchDog
     CLEAR_DOG();	
	//и начинаем этой ногой "мигать"
     for (i = 1000; i>0; i--);           // Delay
     P3OUT^=BIT6;
     show_display();
     }
   }
  switch_xt2();
  }
 init_adc();
 init_uart();
 CCR2=TAR+0x100;	//workaround прерывания часового

  _EINT();                              // Enable interrupts
  
  while(1)
  {
    #ifdef CABLE
     P1OUT &= ~0x01;                     // Reset P1.0 LED off
    #endif
    SUM_TIME(DISPLAY_JOB,1);
    _BIS_SR(CPUOFF+GIE);
//    SUM_TIME(DISPLAY_JOB,0);
//    _BIS_SR(GIE);

    #ifdef CABLE
     P1OUT |= 0x01;                      // Set P1.0 LED on
    #endif
    //-----------------
    // работа с "дисплеем"
    //-----------------
    if (update_display){
     update_display=0;
     tick_timer();
     work_with_display();
     }

     SUM_TIME(ADC_JOB,0);
     _BIS_SR(GIE);


    //-----------------
    // работа с данными из АЦП
    //-----------------
    #ifdef CABLE
     P1OUT |= 0x01;                      // Set P1.0 LED on
    #endif //
    while((adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1))!=(adc_rcv_fifo_end& (ADC_FIFO_RCV_LEN-1))){ //обязательно написать оценку переполнения
     if (work_with_adc_put()==0) break;
     }
    if (send_correction_temperature>100) put_packet_type1A();
    #ifdef CABLE
     P1OUT &= ~0x01;                     // Reset P1.0 LED off
    #endif
    SUM_TIME(STAT_JOB,0);
    _BIS_SR(GIE);
    //-----------------


    //-----------------
    // работа с данными статистики
    //-----------------
    #ifdef CABLE
     P1OUT |= 0x01;                      // Set P1.0 LED on
    #endif
    while ((stat_rcv_fifo_start& (STAT_FIFO_RCV_LEN-1))!=(stat_rcv_fifo_end& (STAT_FIFO_RCV_LEN-1))){
     if (put_packet_type5()==0) break;
     }
    while ((stat1_rcv_fifo_start& (STAT1_FIFO_RCV_LEN-1))!=(stat1_rcv_fifo_end& (STAT1_FIFO_RCV_LEN-1))){
     if (put_packet_type6()==0) break;
     }
    #ifdef CABLE
     P1OUT &= ~0x01;                     // Reset P1.0 LED off
    #endif
    SUM_TIME(SERIAL_JOB,0);
    _BIS_SR(GIE);
    //-----------------


    //-----------------
    // работа с последовательным портом
    //-----------------
    #ifdef CABLE
     P1OUT |= 0x01;                      // Set P1.0 LED on
    #endif
    while(received_packed){
      work_with_serial_rec();
     }
    while(fifo_trn_depth<(SERIAL_FIFO_TRN_LEN/2)){
     if (packet_in_fifo==0) break;
      work_with_serial();
     }

    #ifdef CABLE
     P1OUT &= ~0x01;                     // Reset P1.0 LED off
    #endif
//    SUM_TIME(0);
//    _BIS_SR(CPUOFF+GIE);
    //-----------------


  }

}




interrupt[NMI_VECTOR] void nmi(void)
{
}

//рассчет потребления
//ADC 
// 200 mkA на два младших разряда во время преобразования
// 0.5 mA (max 0.8) - внутреннее опорное
