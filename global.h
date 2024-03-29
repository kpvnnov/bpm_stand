//$Id: global.h,v 1.28 2004-05-13 14:45:46 peter Exp $


#define DEBUG_SERIAL


//#define CLEAR_DOG()   WDTCTL=(WDTCTL&0x00FF)+WDTPW+WDTCNTCL
#define CLEAR_DOG()     WDTCTL=WDTPW|WDTHOLD

//#define DISPLAY






//������
struct tm_in {
        int sec;
        int min;
        int hour;
        int day;
};

#define SECS_IN_MIN (time_in)60
#define MINS_IN_HR  (time_in)60
#define HRS_IN_DAY  (time_in)24
#define SECS_IN_HR  (SECS_IN_MIN * MINS_IN_HR)
#define SECS_IN_DAY (SECS_IN_HR * HRS_IN_DAY)
void local_time(time_in t1,struct tm_in *t2);
void init_timer_a(void);
void tick_timer(void);





// ���
void init_adc(void);    //������������� ���
void off_adc(void);     //���������� ���
#define  ADC_FIFO_RCV_LEN  8           /* size of fifo ADC buffer   */
#define  SIZE_OF_ADC_DUMP  8
#define  NUM_CHANEL 32
void set_dac(u16 d);

//���������
void show_display(void);
void redraw_display_second(int force);
void redraw_display_minutes(int force);
void redraw_display_voltage(int force);
void redraw_display_celciy(int force);
//void cs_on_display(void);
//void cs_off_display(void);
//int power_good(void);
void work_with_display(void);
void work_with_serial(void);
unsigned int crc16(void*,unsigned int);
u16 send_serial_massiv(u8* data,u16 len);
u16 work_with_adc_put(void);
u8 put_packet_type3(u16 info);
u8 put_packet_type4(void);
u16 put_packet_type5(void);
u16 put_packet_type6(void);
u8 put_packet_type7(u16 info);
u8 put_packet_typeA(u16 info);
u8 put_packet_type19(u16 info);
u8 put_packet_type1A(void);

u8 write_asp_trn_fifo(u8 data_wr);
void init_uart(void);
void update_diplay(void);
void work_with_serial_rec(void);
void set_adc(int ch, char b);
void set_adc_temperature(void);



//���������������� ����
//#define  SERIAL_FIFO_RCV_LEN  8           /* size of receive fifo serial buffer   */
#define  SERIAL_FIFO_TRN_LEN  64           /* size of transmit fifo serial buffer   */

// ????????? #if NUM_MULTICHANNEL



#define  ALL_JOB        0
#define  DISPLAY_JOB    1
#define  SERIAL_JOB     2
#define  ADC_JOB        3
#define  STAT_JOB       4
//������ ������ ������ � ����������
//���� � ���������� ����� �� ������ ������, ��:
//��������� ����� ������, ������������ timer_hold
//����� ��������� ����� ������, ������������ timer_hold
//��� ������ ���������� � ������ ����������
#define HOLD_TIME_IRQ()  temp_hold=TAR-2; \
                         if (sleep) {timer_sum_sleep+=temp_hold-timer_hold;} \
                         else { \
                          switch(why_job){ \
                           default: \
                            timer_sum+=temp_hold-timer_hold;break; \
                           case DISPLAY_JOB: \
                            timer_sum_display+=temp_hold-timer_hold;break; \
                           case SERIAL_JOB: \
                            timer_sum_serial+=temp_hold-timer_hold;break; \
                           case ADC_JOB: \
                            timer_sum_adc+=temp_hold-timer_hold;break; \
                           case STAT_JOB: \
                            timer_sum_stat+=temp_hold-timer_hold;break; \
                           } \
                          }
//��������� ����� ������ �������� ��������� - ���� ������ �������� ����� ���� � ������
#define SUM_TIME(what_job,to_sleep)       _DINT(); \
                          temp_hold=TAR; \
                          switch(why_job){ \
                           default: \
                            timer_sum+=temp_hold-timer_hold;break; \
                           case DISPLAY_JOB: \
                            timer_sum_display+=temp_hold-timer_hold;break; \
                           case SERIAL_JOB: \
                            timer_sum_serial+=temp_hold-timer_hold;break; \
                           case ADC_JOB: \
                            timer_sum_adc+=temp_hold-timer_hold;break; \
                           case STAT_JOB: \
                            timer_sum_stat+=temp_hold-timer_hold;break; \
                           } \
                         sleep=to_sleep;why_job=what_job; \
                         timer_hold=TAR; \
                         if (sleep) \
                          timer_sum_sleep+=(timer_hold-temp_hold); \
                         else timer_sum+=(timer_hold-temp_hold)

//��������� ����� ������ � ���������� � ������ ������ ������ �������� ���������
#define SUM_TIME_IRQ()   timer_hold=TAR+2;timer_sum_int+=timer_hold-temp_hold;sleep=0
#define SUM_TIME_IRQ_NOSLEEP()   timer_hold=TAR+3;timer_sum_int+=timer_hold-temp_hold



enum chanel_job{
        NO_JOB,
        LEVEL_UP,       //���������� ��������
        LEVEL_DOWN
       };

#define open_valve()    {valve_hold=0;P2OUT&=~(BIT4+BIT5);}
#define set_hold()      P2OUT&=~BIT5
#define close_valve()   {valve_hold=4;P2OUT|=(BIT4+BIT5);}
#define on_pump()       P4OUT|=BIT0
#define off_pump()      P4OUT&=~BIT0
