
// $Id: adc.c,v 1.1 2003-04-03 07:27:31 peter Exp $
#include  <msp430x14x.h>
#include "global.h"


unsigned int results[5];         // Needs to be global in this example
                                        // Otherwise, the compiler removes it
                                        // because it is not used for anything.
int error_adc;

int end_adc_conversion;

interrupt[ADC_VECTOR] void ADC12ISR (void)
{
  results[0] = ADC12MEM0;               // Move results, IFG is cleared
  results[1] = ADC12MEM1;               // Move results, IFG is cleared
  results[2] = ADC12MEM2;               // Move results, IFG is cleared    
  results[3] = ADC12MEM3;               // Move results, IFG is cleared  
  results[4] = ADC12MEM4;               // Move results, IFG is cleared  
  _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
  end_adc_conversion=1;
}


void init_adc(void){
  ADC12CTL0 = ADC12ON+REFON+MSC+SHT0_6;    // Turn on ADC12, set sampling time
//REF2_5V
  ADC12CTL1 = SHP+CONSEQ_1;             // Use sampling timer, single sequence
  ADC12MCTL0 = INCH_0;                  // ref+=AVcc, channel = A0
  ADC12MCTL1 = INCH_1;                  // ref+=AVcc, channel = A1
  ADC12MCTL2 = INCH_2;                  // ref+=AVcc, channel = A2    
  ADC12MCTL3 = INCH_3;                  // ref+=AVcc, channel = A3, end seq.
  ADC12MCTL4 = INCH_10+SREF_1+EOS;      // ref+=AVcc, channel = A3, end seq.
  ADC12IE = 0x10;                       // Enable ADC12IFG.3
  ADC12CTL0 |= ENC;                     // Enable conversions

 error_adc=0;
 end_adc_conversion=1;

}
