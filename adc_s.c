
// $Id: adc_s.c,v 1.11 2004-03-07 21:31:50 peter Exp $
#include  <msp430x14x.h>

//типы переменных
#include "type_def.h"

#ifdef STEND
#include "uart_s.h"
#endif // STEND

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
extern u16 jitter_adc;
extern u16 jitter_adc_max;
extern u16 jitter_pusk;

extern u16 stop_transmit;
extern u16 analog_on;
extern u16 temperature;

//unsigned int results0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//unsigned int results1[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//unsigned int results2[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//unsigned int results3[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//unsigned int results4[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//unsigned int results5[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//unsigned int results6[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];

unsigned int results[ADC_FIFO_RCV_LEN];
unsigned int one_count0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
unsigned int one_count1[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
unsigned int multi_count0[NUM_MULTICHANNEL][ADC_FIFO_RCV_LEN];
unsigned int multi_count1[NUM_MULTICHANNEL][ADC_FIFO_RCV_LEN];
                                        
unsigned int current_level;
unsigned int what_doing;
unsigned int timer1;

extern int error_adc;

extern int end_adc_conversion;

u16 chanel;
u16 chanel_convert;
extern u16 dac[NUM_CHANEL];

extern int rotate_channel;
extern int first_channel;



extern volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

extern unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */


extern unsigned int  adc_rcv_fifo_start;      /* ADC receive buffer start index      */

extern volatile unsigned int  adc_rcv_fifo_end;        /* ADC receive  buffer end index        */


interrupt[ADC_VECTOR] void ADC12ISR (void)
{
//int p=( & (ADC_FIFO_RCV_LEN-1))<<3;
int sh;
unsigned int sum,sum1;
HOLD_TIME_IRQ()
 end_adc_conversion=1;
  jitter_adc=CCR1-TAR;
  if (jitter_adc_max<jitter_adc) jitter_adc_max=jitter_adc;

//    jitter_adc_max++;

//  jitter_adc=TAR;
//  if (jitter_pusk>jitter_adc) jitter_adc=jitter_pusk-jitter_adc; 
//   else jitter_adc=jitter_adc-jitter_pusk;
//  if (jitter_adc_max<jitter_adc) jitter_adc_max=jitter_adc;

 if (temperature==1){
    sum=ADC12MEM0;
    sum+=ADC12MEM1;
    sum+=ADC12MEM2;
    sum+=ADC12MEM3;
    sum+=ADC12MEM4;
    sum+=ADC12MEM5;
    sum+=ADC12MEM6;
    sum+=ADC12MEM7;
    sum+=ADC12MEM8;
    sum+=ADC12MEM9;
    sum+=ADC12MEM10;
    sum+=ADC12MEM11;
    sum+=ADC12MEM12;
    sum+=ADC12MEM13;
    sum+=ADC12MEM14;
    sum+=ADC12MEM15;
   sh=(adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1));
   multi_count0[0][sh]=sum;
   results[sh]=0x4000;
   if (stop_transmit==0)
    adc_rcv_fifo_end++;
   ADC12CTL0 |= ADC12SC;                 // Start conversion
   _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
   SUM_TIME_IRQ();
  }
 else
 if (chanel_convert&0x40){
  sh=(adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1));

    sum=ADC12MEM0;
    if (rotate_channel==0) 
     current_level=sum;
    sum+=ADC12MEM2;
    sum+=ADC12MEM4;
    sum+=ADC12MEM6;
    sum+=ADC12MEM8;
    sum+=ADC12MEM10;
    sum+=ADC12MEM12;
    sum+=ADC12MEM14;


    sum1=ADC12MEM1;
    sum1+=ADC12MEM3;
    sum1+=ADC12MEM5;
    sum1+=ADC12MEM7;
    sum1+=ADC12MEM9;
    sum1+=ADC12MEM11;
    sum1+=ADC12MEM13;
    sum1+=ADC12MEM15;

    if (rotate_channel<=NUM_MULTICHANNEL){
     multi_count0[rotate_channel][sh]=sum;
     multi_count1[rotate_channel][sh]=sum1;
     }
    else rotate_channel=0; ///ошиБОЧНАЯ СИТУАЦИЯ

    rotate_channel++;


//  if (rotate_channel==(NUM_MULTICHANNEL)){ //для следующего цикла устанавливаем температуру
//   set_adc_temperature(); //температура будет суммой sum и sum1
//   SUM_TIME_IRQ_NOSLEEP();
//   }
//  else
  if (rotate_channel==(NUM_MULTICHANNEL)){ //для следующего цикла устанавливаем цикл с начала
    // и выставляем данные для занесения в очередь передачи
   results[sh]=0x8000|(first_channel&0x1F);
   if (stop_transmit==0)
    adc_rcv_fifo_end++;
   rotate_channel=0;
   set_adc((first_channel&0xC0)+((first_channel+rotate_channel)&0x3F));
   ADC12CTL0 |= ADC12SC;                 // Start conversion
   _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
   SUM_TIME_IRQ();
   }
  else { //обычный цикл для следующего канала
   set_adc((first_channel&0xC0)+((first_channel+rotate_channel)&0x3F));
   ADC12CTL0 |= ADC12SC;                 // Start conversion
   SUM_TIME_IRQ_NOSLEEP();
   }
  
  }
 else{ //"обыкновенный" парный режим
  sh=(adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1))*SIZE_OF_ADC_DUMP;

  one_count0[sh]   = ADC12MEM0;               
  current_level=one_count0[sh];
  one_count1[sh++] = ADC12MEM1;               
  one_count0[sh]   = ADC12MEM2;
  one_count1[sh++] = ADC12MEM3;               
  one_count0[sh]   = ADC12MEM4;               
  one_count1[sh++] = ADC12MEM5;               
  one_count0[sh]   = ADC12MEM6;               
  one_count1[sh++] = ADC12MEM7;               
  one_count0[sh]   = ADC12MEM8;               
  one_count1[sh++] = ADC12MEM9;               
  one_count0[sh]   = ADC12MEM10;               
  one_count1[sh++] = ADC12MEM11;               
  one_count0[sh]   = ADC12MEM12;               
  one_count1[sh++] = ADC12MEM13;               
  one_count0[sh]   = ADC12MEM14;               
  one_count1[sh++] = ADC12MEM15;               

  results[adc_rcv_fifo_end & (ADC_FIFO_RCV_LEN-1)]=chanel_convert&0x1F;
  if (stop_transmit==0)
   adc_rcv_fifo_end++;

  ADC12CTL0 |= ADC12SC;                 // Start conversion
  _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
  SUM_TIME_IRQ();
 }
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
  ADC12CTL0 = ADC12ON+REFON+MSC+SHT0_6+SHT1_6;    // Turn on ADC12, set sampling time
//ADC12CLK=3 686 400
//SHT0_8 t_sample=4*ADC12CLK*64=14400 Гц    0.00006944

//REF2_5V
	//ADC12 SSELx Bits 4-3
	//	ADC12 clock source select
	//	00 ADC12OSC
	//	01 ACLK
	//	10 MCLK
	//	11 SMCLK -ADC12SSEL_3
	//SHS bits 10-11 Source select for the sample-input signal.
	//0: Control bit ADC12SC is selected.
	//1: Timer_A.OUT1
	//2: Timer_B.OUT0
	//3: Timer_B.OUT1
  ADC12CTL1 = ADC12SSEL_3+ADC12DIV_1+SHP+CONSEQ_1+SHS_1;             // Use sampling timer, single sequence
// ADC12DIV_1 - деление на 2
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

void off_adc(void){
int z;
ADC12CTL0=0;
ADC12CTL0=0;
z=ADC12MEM15;

}


int counter;
u16 work_with_adc_put(void){
 if (counter++>=1000){
  counter=0;
//  put_packet_type4();
  }

  if (results[adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1)]&0x8000){
   if (put_packet_typeA(adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1))){
    adc_rcv_fifo_start++;
    return 1;
    }
   }
  else
  if (results[adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1)]&0x4000){
   if (put_packet_type19(adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1))){
    adc_rcv_fifo_start++;
    return 1;
    }
   }
  else
  if (put_packet_type7(adc_rcv_fifo_start& (ADC_FIFO_RCV_LEN-1))){
   adc_rcv_fifo_start++;
   return 1;
   }

  return 0;
}


void set_adc_temperature(void){
 ADC12CTL0 &= ~ENC;                     // Enable conversions

 ADC12CTL0 = ADC12ON+REFON+MSC+SHT0_15+SHT1_15;    // Turn on ADC12, set sampling time

 ADC12MCTL0 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL1 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL2 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL3 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL4 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL5 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL6 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL7 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL8 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL9 = INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL10= INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL11= INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL12= INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL13= INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL14= INCH_10+SREF_1;      // ref+=AVcc, channel = A10, end seq.
 ADC12MCTL15= INCH_10+SREF_1+EOS;      // ref+=AVcc, channel = A10, end seq.

 ADC12IE = 1<<15;                       // Enable ADC12IFG.3
 ADC12CTL0 |= ENC;                     // Enable conversions
}

void set_adc(int ch){

 ADC12CTL0 &= ~ENC;                     // Enable conversions
 ADC12CTL0 = ADC12ON+REFON+MSC+SHT0_6+SHT1_6;    // Turn on ADC12, set sampling time
 set_dac(dac[ch&((NUM_CHANEL>>1)-1)]); //
 
 if (analog_on){
  P3OUT&=~(BIT0|BIT1|BIT2|BIT3);

  if (ch&0x01) P3OUT|=BIT0;
  if (ch&0x08) P3OUT|=BIT2;
  if (ch&0x10) P3OUT|=BIT1;
  if ((ch&0x20)==0) P3OUT|=BIT3;
  }

 switch((ch>>1)&0x03){
  case 0:
   ADC12MCTL1 = INCH_6+SREF_2;
   ADC12MCTL3 = INCH_6+SREF_2;
   ADC12MCTL5 = INCH_6+SREF_2;
   ADC12MCTL7 = INCH_6+SREF_2;
   ADC12MCTL9 = INCH_6+SREF_2;
   ADC12MCTL11= INCH_6+SREF_2;
   ADC12MCTL13= INCH_6+SREF_2;
   ADC12MCTL15= INCH_6+SREF_2+EOS;
   break;
  case 1:
   ADC12MCTL1 = INCH_5+SREF_2;
   ADC12MCTL3 = INCH_5+SREF_2;
   ADC12MCTL5 = INCH_5+SREF_2;
   ADC12MCTL7 = INCH_5+SREF_2;
   ADC12MCTL9 = INCH_5+SREF_2;
   ADC12MCTL11= INCH_5+SREF_2;
   ADC12MCTL13= INCH_5+SREF_2;
   ADC12MCTL15= INCH_5+SREF_2+EOS;
   break;
  case 2:
   ADC12MCTL1 = INCH_4+SREF_2;
   ADC12MCTL3 = INCH_4+SREF_2;
   ADC12MCTL5 = INCH_4+SREF_2;
   ADC12MCTL7 = INCH_4+SREF_2;
   ADC12MCTL9 = INCH_4+SREF_2;
   ADC12MCTL11= INCH_4+SREF_2;
   ADC12MCTL13= INCH_4+SREF_2;
   ADC12MCTL15= INCH_4+SREF_2+EOS;
   break;
  case 3:
   ADC12MCTL1 = INCH_3+SREF_2;
   ADC12MCTL3 = INCH_3+SREF_2;
   ADC12MCTL5 = INCH_3+SREF_2;
   ADC12MCTL7 = INCH_3+SREF_2;
   ADC12MCTL9 = INCH_3+SREF_2;
   ADC12MCTL11= INCH_3+SREF_2;
   ADC12MCTL13= INCH_3+SREF_2;
   ADC12MCTL15= INCH_3+SREF_2+EOS;
   break;
  }//свитч
 if (ch&0x80){ //резервный канал
  ADC12MCTL0 = INCH_1+SREF_2;           // ref+=Ve REF,  channel = A1
  ADC12MCTL2 = INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL4 = INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL6 = INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL8 = INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL10= INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL12= INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  ADC12MCTL14= INCH_1+SREF_2;           // ref+=Ve REF+, channel = A1
  }
 else{// основной канал
  ADC12MCTL0 = INCH_7+SREF_2;           // ref+=Ve REF,  channel = A7
  ADC12MCTL2 = INCH_7+SREF_2;           // ref+=Ve REF+, channel = A7
  ADC12MCTL4 = INCH_7+SREF_2;           // ref+=Ve REF+, channel = A7
  ADC12MCTL6 = INCH_7+SREF_2;           // ref+=Ve REF+, channel = A7
  ADC12MCTL8 = INCH_7+SREF_2;           // ref+=Ve REF+, channel = A7
  ADC12MCTL10= INCH_7+SREF_2;           // ref+=Ve REF+, channel = A7
  ADC12MCTL12= INCH_7+SREF_2;           // ref+=Ve REF+, channel = A7
  ADC12MCTL14= INCH_7+SREF_2;           // ref+=Ve REF+, channel = A7
  }
 ADC12IE = 1<<15;                       // Enable ADC12IFG.3
// ADC12IE = 1<<7;                       // Enable ADC12IFG.3
 ADC12CTL0 |= ENC;                     // Enable conversions
}
