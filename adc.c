
// $Id: adc.c,v 1.10 2003-05-28 16:47:16 peter Exp $
#include  <msp430x14x.h>
#include "global.h"

extern u16 temp_hold;
extern u16 timer_sum_sleep;
extern u16 timer_sum_int;
extern u16 timer_hold;
extern u16 timer_sum;
extern u16 sleep;
extern u16 timer_sum_display;
extern u16 timer_sum_serial;
extern u16 timer_sum_adc;
extern u16 timer_sum_stat;
extern u16 why_job;

unsigned int results0[ADC_FIFO_RCV_LEN];
unsigned int results1[ADC_FIFO_RCV_LEN];
unsigned int results2[ADC_FIFO_RCV_LEN];
unsigned int results3[ADC_FIFO_RCV_LEN];
unsigned int results4[ADC_FIFO_RCV_LEN];
unsigned int results5[ADC_FIFO_RCV_LEN];
unsigned int results6[ADC_FIFO_RCV_LEN];
unsigned int results7[ADC_FIFO_RCV_LEN];
                                        
int error_adc;

int end_adc_conversion;


unsigned int  adc_rcv_fifo_start;      /* ADC receive buffer start index      */

volatile unsigned int  adc_rcv_fifo_end;        /* ADC receive  buffer end index        */


interrupt[ADC_VECTOR] void ADC12ISR (void)
{
//int p=( & (ADC_FIFO_RCV_LEN-1))<<3;
int sh;

int temp_led;
HOLD_TIME_IRQ()
 temp_led=P1OUT;
 P1OUT |= 0x01;                      // Set P1.0 LED on
  sh=adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1);
  results0[sh] = ADC12MEM0;               // Move results, IFG is cleared
  results1[sh] = ADC12MEM1;               // Move results, IFG is cleared
  results2[sh] = ADC12MEM2;               // Move results, IFG is cleared    
  results3[sh] = ADC12MEM3;               // Move results, IFG is cleared  
  results4[sh] = ADC12MEM4;               // Move results, IFG is cleared  
  _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
  end_adc_conversion=1;
  adc_rcv_fifo_end++;
 P1OUT = temp_led;                     // return led state
SUM_TIME_IRQ();
}

//для температурного сенсора sample period более 30 mkS
void init_adc(void){
	//ADC12ON - ADC12 On
        //REFON   - Reference generator
	//MSC	  - Multiple sample and convertion
        //SHT0_xx - Sample-and-hold time. These bits define the number 
	//	    of ADC12CLK cycles in the sampling period for 
	//	    registers ADC12MEM0 to ADC12MEM7
        //SHT1_xx - Sample-and-hold time. These bits define the number 
	//	    of ADC12CLK cycles in the sampling period for 
	//	    registers ADC12MEM8 to ADC12MEM15
  ADC12CTL0 = ADC12ON+REFON+MSC+SHT0_11+SHT1_11;    // Turn on ADC12, set sampling time
//REF2_5V
	//ADC12 SSELx Bits 4-3
	//	ADC12 clock source select
	//	00 ADC12OSC
	//	01 ACLK
	//	10 MCLK
	//	11 SMCLK
	//SHS bits 10-11 Source select for the sample-input signal.
	//0: Control bit ADC12SC is selected.
	//1: Timer_A.OUT1
	//2: Timer_B.OUT0
	//3: Timer_B.OUT1
  ADC12CTL1 = ADC12SSEL_3+ADC12DIV_7+SHP+CONSEQ_1+SHS_1;             // Use sampling timer, single sequence
	//SREFx Bits 6-4 Select reference
	//	000 VR+ = A VCC 	and VR- = A VSS - от 0 до питания
	//	001 VR+ = V REF+ 	and VR- = A VSS - от 0 до внутр. опорн.
	//	010 VR+ = Ve REF+ 	and VR- = A VSS - от 0 до внеш. опорн.
	//	011 VR+ = Ve REF+ 	and VR- = A VSS ???
	//	100 VR+ = A VCC 	and VR- = V REF- / Ve REF-
	//	101 VR+ = V REF+ 	and VR- = V REF- / Ve REF-
	//	110 VR+ = Ve REF+ 	and VR- = V REF- / Ve REF-
	//	111 VR+ = Ve REF+ 	and VR- = V REF- / Ve REF-
  ADC12MCTL0 = INCH_0+SREF_2;           // ref+=Ve REF,  channel = A0
  ADC12MCTL1 = INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL2 = INCH_2+SREF_2;           // ref+=AVcc, channel = A2    
  ADC12MCTL3 = INCH_3+SREF_2;           // ref+=AVcc, channel = A3, end seq.
  ADC12MCTL4 = INCH_10+SREF_1+EOS;      // ref+=AVcc, channel = A10, end seq.
  ADC12IE = 0x10;                       // Enable ADC12IFG.3
  ADC12CTL0 |= ENC;                     // Enable conversions

 error_adc=0;
 end_adc_conversion=1;
 adc_rcv_fifo_end=0;
 adc_rcv_fifo_start=0;

}

int counter;
u16 work_with_adc_put(void){
//int p;
//проверку очереди выносим на верхний круг
// if (adc_rcv_fifo_start==adc_rcv_fifo_end) return ;
 if (counter++>=1000){
  counter=0;
//  put_packet_type4();
//  put_packet_type4();
//  put_packet_type4();
//  put_packet_type4();
//  put_packet_type4();
//  put_packet_type4();
  }
//  p=(adc_rcv_fifo_start & (ADC_FIFO_RCV_LEN-1))<<3;
//отладка
//  results[p]='0';
//  results[p]='1';
//  results[p]='2';
//отладка

  if (put_packet_type3(adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1))){
   adc_rcv_fifo_start++;
   return 1;
   }

  return 0;
}
