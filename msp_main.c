//******************************************************************************
// $Id: msp_main.c,v 1.8 2003-04-29 11:07:46 peter Exp $
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

// АЦП
extern int end_adc_conversion;
extern unsigned int results[5];         // Needs to be global in this example
extern int error_adc;

//таймер
extern int change_to_mode;
	// 0 - ACLK 1 - SMCLK
extern int mode_timer;
	//это время по гринвичу
extern time_in GlobalTime;
	//если не 0, то запускаемя на полной скорости
extern int run_full_speed;
	//переключаемся на скоростной режим таймера
extern int switch_speed_timer;


// дисплей
extern int mode_display;
extern int second_point;
extern int invert;
	//это время в формате long для показа на индикатор
extern time_in time_to_show;










//возвращаем 1, если часовой кварц работает
int test_run_LFXT1CLK(void){
int i;
 for (i = 100; i>0; i--);           // Delay
 return 1;
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
//int i;
	// останавливаем watchdog
 WDTCTL=WDTPW|WDTHOLD;  		// Stop WDT
	// конфигурируем ноги ввода вывода
 set_pin_directions();
 run_full_speed=0;
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
	//переходим на работу от часового кварца
 run_LFXT1CLK(0x02);
// init_wdt();

//запускаем таймер A и часы от него
 init_timer_a();
 init_adc();
  _EINT();                              // Enable interrupts
  
  while(1)
  {
//    BCSCTL2|=SELM0|SELM1;
    P1OUT &= ~0x01;                     // Reset P1.0 LED off
    _BIS_SR(CPUOFF);                 // входим в режим спячки
    P1OUT |= 0x01;                      // Set P1.0 LED on
    tick_timer();
    if (current_speed==0 && run_full_speed  && run_xt2()){
     switch_xt2();
     }
    switch(mode_work){
     case 0: //вывод на экран времени (минуты секунды)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode++;
       mode_work++;
       time_to_change=GlobalTime;
       }
      break;
     case 1: //вывод на экран времени (часы минуты)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode++;
       mode_work++;
       time_to_change=GlobalTime;
       }
      break;
     case 2: //вывод на экран напряжения (0-4095)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode++;
       mode_work++;
       time_to_change=GlobalTime;
       }
      break;
     case 3: //вывод на экран температуры (градусы цельсия)
      if ((time_to_change+5)<GlobalTime){
       change_to_mode=0;
       mode_work=0;
       time_to_change=GlobalTime;
       }
      break;
     }
//    P1OUT &= ~0x01;                     // Reset P1.0 LED off
  }

}




interrupt[NMI_VECTOR] void nmi(void)
{
}