// $Id: timer_a.c,v 1.18 2004-03-07 21:31:50 peter Exp $
#include  <msp430x14x.h>
#include <stdlib.h>

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
extern u16 timer_diff_min;
extern u16 timer_diff_max;
extern u16 why_job;



#ifdef DEBUG_SERIAL
extern u16 packet_in_fifo;
extern u16 packet_in_fifo_max;
extern u16 fifo_trn_depth;
extern u16 packet_fifo_full;
extern u16 fifo_trn_depth_max;
extern u16 error_uart_depth;
extern u16 error_send_serial;
extern u16 length_sended_2_fifo_max;
extern u16 length_sended_2_fifo_min;
#endif //DEBUG_SERIAL



extern u16 chanel;
extern u16 chanel_convert;
u16 dac[NUM_CHANEL];


extern volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

extern unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */



unsigned int  stat_rcv_fifo_start;      /* stat receive buffer start index      */

volatile unsigned int  stat_rcv_fifo_end;        /* stat receive  buffer end index        */

u16 stat_buf[STAT_FIFO_RCV_LEN*SIZE_STAT];

unsigned int  stat1_rcv_fifo_start;      /* stat receive buffer start index      */

volatile unsigned int  stat1_rcv_fifo_end;        /* stat receive  buffer end index        */
u16 stat1_buf[STAT1_FIFO_RCV_LEN*SIZE_STAT1];


//ADC
extern int end_adc_conversion;
extern int error_adc;
extern unsigned int results0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results8[ADC_FIFO_RCV_LEN];

int gradus_to_show;
int volt_to_show;
int capture_timer;

int change_to_mode;
	// 0 - ACLK 1 - SMCLK
int mode_timer;
int counter_timer;
//int refresh_timer;
int sub_counter_timer;
	//это время по гринвичу
time_in GlobalTime;
int second_point;
int invert;
	//если не 0, то запускаемя на полной скорости
int run_full_speed;
	//переключаемся на скоростной режим таймера
int switch_speed_timer;

// дисплей
int mode_display;
int update_display;
	//это время в формате long для показа на индикатор
time_in time_to_show;
int symbl[4];
unsigned int displ[2];

extern int mode_work;
extern long time_to_change;

