//********************************************************
// $Id: msp_main.c,v 1.13 2003-05-21 20:29:46 peter Exp $
//********************************************************

//#include <msp430x11x1.h>
//#include  <msp430x13x.h>
#include  <msp430x14x.h>
#include "global.h"

// ���
extern int end_adc_conversion;
//extern unsigned int results[5];         // Needs to be global in this example
extern int error_adc;

//������
extern int change_to_mode;
	// 0 - ACLK 1 - SMCLK
extern int mode_timer;
	//��� ����� �� ��������
extern time_in GlobalTime;
	//���� �� 0, �� ���������� �� ������ ��������
//extern int run_full_speed;
	//������������� �� ���������� ����� �������
extern int switch_speed_timer;


// �������
extern int mode_display;
extern int second_point;
extern int invert;
	//��� ����� � ������� long ��� ������ �� ���������
extern time_in time_to_show;










//���������� �� ����, ���� ������� ����� ��������
int test_run_LFXT1CLK(void){
int i=10;
unsigned int to_compare,to_compare1;
 while(i){
	//����������� �������� �������
  to_compare=TIMER_A_COUNTER;	
	//������ �����
  for (i = 100; i>0; i--);           // Delay
	//����������� �������� ������� �����
  to_compare1=TIMER_A_COUNTER;
	//������������ �������
  if (to_compare1>to_compare)
   to_compare=to_compare1-to_compare;
  else
   to_compare=0x2000+to_compare1-to_compare;
	//���� ������ �����, �� �������, ��� ����� ����������
  if (to_compare>0x100) break;
  i--;
  }
 return i;
}



//�������� �������� �������� ������� ������
// ���� ������� ������ ����������, ��:
//  - ��������� XT2 (���� mode&0x01)
//  - ��������� DC Generator (SCG0=1 ���� mode&0x02)
//  - ��������� SMCLK (SCG1=1 ���� mode&0x04)
// � ���������� 1 (True) ����� ���������� �������� �� ���������� ����������
// � ���������� 0 (False)
int run_LFXT1CLK( int mode){
	//XTS = 0: The low-frequency oscillator is selected.
 if (mode&0x04) _BIS_SR(SCG1);
 BCSCTL1&=~XTS;

 _BIC_SR(OSCOFF);			// �������� ������� �����
	//��������� ������ �������� ������
 if (!test_run_LFXT1CLK()){	//������� �� ����������
  return 0;
  }
	// �������� (ACLK/1)
	// DIVA = 0: 1
	// DIVA = 1: 2
	// DIVA = 2: 4
	// DIVA = 3: 8
	// ���� ��������� ���������� XT2 ������ - ���������
	// XT2Off = 1: the oscillator is off if it is not used for MCLK or SMCLK.
 BCSCTL1 = (BCSCTL1& (~(DIVA0|DIVA1))) | (mode&0x01 ? XT2OFF:0); 
	// �������� �������� �������� ������� - LFXT1CLK
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
	// ������������� watchdog
 WDTCTL=WDTPW|WDTHOLD;  		// Stop WDT
	// ������������� ���� ����� ������
 set_pin_directions();
// run_full_speed=0;
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
	//��������� ������ A � ���� �� ���� (ACLK)
 init_timer_a();
	//��������� �� ������ �� �������� ������
	//���� ������� ����� �� �����-�� �������� �� ����������
 if (!run_LFXT1CLK(0x02)){
	//�� ������� ���� ��� ��������� �������
	//��������� ���� �� ����� USART1
    P3SEL &= ~0xC0;                        // P3.6,7 = USART1 option select
    while(1){
	//� �������� ���� ����� "������"
     for (i = 3000; i>0; i--);           // Delay
     P3OUT^=PIN�����-��;
     for (i = 500; i>0; i--);           // Delay
     P3OUT^=PIN�����-��;
     }
  }
// init_wdt();

	//��������� �� ������ �� ������� ������
 if (current_speed==0 && power_good() ){
	//���� ������ ����� �� �����-�� �������� �� ����������
  if (!run_xt2()){
	//�� ������� ���� ��� ��������� �������
	//��������� ���� �� ����� USART1
    P3SEL &= ~0xC0;                        // P3.6,7 = USART1 option select
    while(1){
	//� �������� ���� ����� "������"
     for (i = 1000; i>0; i--);           // Delay
     P3OUT^=PIN�����-��;
     }
   }
  switch_xt2();
  }
 init_adc();
 init_uart1();

  _EINT();                              // Enable interrupts
  
  while(1)
  {
//    BCSCTL2|=SELM0|SELM1;
    _BIS_SR(CPUOFF);                 // ������ � ����� ������
    P1OUT &= ~0x01;                     // Reset P1.0 LED off
    tick_timer();
    work_with_display();

    //-----------------
    // ������ � ���������������� ������
    //-----------------
    if (packet_in_fifo&&(fifo_trn_depth<(SERIAL_FIFO_TRN_LEN/2)))
     work_with_serial();
    //-----------------

    //-----------------
    // ������ � ������� �� ���
    //-----------------
    if (adc_rcv_fifo_start!=adc_rcv_fifo_end) 
     work_with_adc_put();
    //-----------------

    P1OUT |= 0x01;                      // Set P1.0 LED on
  }

}




interrupt[NMI_VECTOR] void nmi(void)
{
}