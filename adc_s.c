
// $Id: adc_s.c,v 1.1 2003-06-06 13:34:58 peter Exp $
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

unsigned int results0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
unsigned int results1[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
unsigned int results2[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
unsigned int results3[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
unsigned int results[ADC_FIFO_RCV_LEN];
                                        
extern int error_adc;

extern int end_adc_conversion;

u16 chanel;
u16 chanel_convert;




extern volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

extern unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */


extern unsigned int  adc_rcv_fifo_start;      /* ADC receive buffer start index      */

extern volatile unsigned int  adc_rcv_fifo_end;        /* ADC receive  buffer end index        */


interrupt[ADC_VECTOR] void ADC12ISR (void)
{
//int p=( & (ADC_FIFO_RCV_LEN-1))<<3;
int sh;

HOLD_TIME_IRQ()
  sh=(adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1))*SIZE_OF_ADC_DUMP;


  results0[sh]   = ADC12MEM0;               
  results1[sh++] = ADC12MEM1;               
  results0[sh]   = ADC12MEM2;
  results1[sh++] = ADC12MEM3;               
  results0[sh]   = ADC12MEM4;               
  results1[sh++] = ADC12MEM5;               
  results0[sh]   = ADC12MEM6;               
  results1[sh++] = ADC12MEM7;               
  results0[sh]   = ADC12MEM8;               
  results1[sh++] = ADC12MEM9;               
  results0[sh]   = ADC12MEM10;               
  results1[sh++] = ADC12MEM11;               
  results0[sh]   = ADC12MEM12;               
  results1[sh++] = ADC12MEM13;               
  results0[sh]   = ADC12MEM14;               
  results1[sh++] = ADC12MEM15;               

  results[adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1)]=chanel_convert;



  end_adc_conversion=1;
  adc_rcv_fifo_end++;

_BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
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
  ADC12CTL0 = ADC12ON+REFON+MSC+SHT0_15+SHT1_15;    // Turn on ADC12, set sampling time
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
  ADC12CTL1 = ADC12SSEL_3+ADC12DIV_1+SHP+CONSEQ_1+SHS_1;             // Use sampling timer, single sequence
	//SREFx Bits 6-4 Select reference
	//	000 VR+ = A VCC 	and VR- = A VSS - от 0 до питания
	//	001 VR+ = V REF+ 	and VR- = A VSS - от 0 до внутр. опорн.
	//	010 VR+ = Ve REF+ 	and VR- = A VSS - от 0 до внеш. опорн.
	//	011 VR+ = Ve REF+ 	and VR- = A VSS ???
	//	100 VR+ = A VCC 	and VR- = V REF- / Ve REF-
	//	101 VR+ = V REF+ 	and VR- = V REF- / Ve REF-
	//	110 VR+ = Ve REF+ 	and VR- = V REF- / Ve REF-
	//	111 VR+ = Ve REF+ 	and VR- = V REF- / Ve REF-
  ADC12MCTL0 = INCH_1+SREF_2;           // ref+=Ve REF,  channel = A0
  ADC12MCTL1 = INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL2 = INCH_1+SREF_2;           // ref+=AVcc, channel = A2    
  ADC12MCTL3 = INCH_1+SREF_2;           // ref+=AVcc, channel = A3, 
  ADC12MCTL4 = INCH_1+SREF_2;           // ref+=AVcc, channel = A3, 
  ADC12MCTL5 = INCH_1+SREF_2;           // ref+=AVcc, channel = A3, 
  ADC12MCTL6 = INCH_1+SREF_2;           // ref+=AVcc, channel = A3, 
  ADC12MCTL7 = INCH_1+SREF_2;           // ref+=AVcc, channel = A3, 
  ADC12MCTL8 = INCH_10+SREF_1+EOS;      // ref+=AVcc, channel = A10, end seq.


  ADC12IE = 1<<8;                       // Enable ADC12IFG.3




  ADC12CTL0 |= ENC;                     // Enable conversions


 error_adc=0;
 end_adc_conversion=0;
 adc_rcv_fifo_end=0;
 adc_rcv_fifo_start=0;
 chanel=0x0;
 chanel_convert=0xFF;
// dac[0]=0x800;

}

int counter;
u16 work_with_adc_put(void){
 if (counter++>=1000){
  counter=0;
//  put_packet_type4();
  }

  if (put_packet_type7(adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1))){
   adc_rcv_fifo_start++;
   return 1;
   }

  return 0;
}
