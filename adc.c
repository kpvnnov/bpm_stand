
// $Id: adc.c,v 1.7 2003-05-21 20:29:45 peter Exp $
#include  <msp430x14x.h>
#include "global.h"


#define  ADC_FIFO_RCV_LEN  16           /* size of fifo ADC buffer   */

unsigned int results[ADC_FIFO_RCV_LEN*8];         // Needs to be global in this example
                                        // Otherwise, the compiler removes it
                                        // because it is not used for anything.
int error_adc;

int end_adc_conversion;


unsigned int  adc_rcv_fifo_start;      /* ADC receive buffer start index      */

volatile unsigned int  adc_rcv_fifo_end;        /* ADC receive  buffer end index        */


interrupt[ADC_VECTOR] void ADC12ISR (void)
{
int p=(adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1))<<3;

  results[p++] = ADC12MEM0;               // Move results, IFG is cleared
  results[p++] = ADC12MEM1;               // Move results, IFG is cleared
  results[p++] = ADC12MEM2;               // Move results, IFG is cleared    
  results[p++] = ADC12MEM3;               // Move results, IFG is cleared  
  results[p++] = ADC12MEM4;               // Move results, IFG is cleared  
  _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
  end_adc_conversion=1;
  adc_rcv_fifo_end++;
}


void init_adc(void){
  ADC12CTL0 = ADC12ON+REFON+MSC+SHT0_11+SHT1_11;    // Turn on ADC12, set sampling time
//REF2_5V
  ADC12CTL1 = ADC12SSEL_3+ADC12DIV_7+SHP+CONSEQ_1;             // Use sampling timer, single sequence
  ADC12MCTL0 = INCH_0;                  // ref+=AVcc, channel = A0
  ADC12MCTL1 = INCH_1;                  // ref+=AVcc, channel = A1
  ADC12MCTL2 = INCH_2;                  // ref+=AVcc, channel = A2    
  ADC12MCTL3 = INCH_3;                  // ref+=AVcc, channel = A3, end seq.
  ADC12MCTL4 = INCH_10+SREF_1+EOS;      // ref+=AVcc, channel = A10, end seq.
  ADC12IE = 0x10;                       // Enable ADC12IFG.3
  ADC12CTL0 |= ENC;                     // Enable conversions

 error_adc=0;
 end_adc_conversion=1;
 adc_rcv_fifo_end=0;
 adc_rcv_fifo_start=0;

}

int counter;
void work_with_adc_put(void){
int p;
//проверку очереди выносим на верхний круг
// if (adc_rcv_fifo_start==adc_rcv_fifo_end) return ;
 if (counter++>=20){
  counter=0;
  put_packet_type4();
  put_packet_type4();
  put_packet_type4();
  put_packet_type4();
  }
  p=(adc_rcv_fifo_start & (ADC_FIFO_RCV_LEN-1))<<3;
//отладка
//  results[p]='0';
//  results[p]='1';
//  results[p]='2';
//отладка
  if (put_packet_type3((u8*)&results[p]))
   adc_rcv_fifo_start++;

}
