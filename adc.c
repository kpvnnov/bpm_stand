
// $Id: adc.c,v 1.14 2004-03-07 21:31:50 peter Exp $
#include  <msp430x14x.h>

//типы переменных
#include "type_def.h"

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

extern unsigned int results0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int results1[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int results2[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int results3[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int results[ADC_FIFO_RCV_LEN];
                                        
int error_adc;

int end_adc_conversion;

extern u16 chanel;
extern u16 chanel_convert;




extern volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

extern unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */


unsigned int  adc_rcv_fifo_start;      /* ADC receive buffer start index      */

volatile unsigned int  adc_rcv_fifo_end;        /* ADC receive  buffer end index        */




