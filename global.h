//$Id: global.h,v 1.11 2003-05-26 16:39:57 peter Exp $


#define DEBUG_SERIAL


#define  STAT_FIFO_RCV_LEN  16           /* size of fifo STAT buffer   */
#define SIZE_STAT	3


typedef unsigned char  u08;
typedef unsigned char  u8;
typedef          char  s08;
typedef unsigned short u16;
typedef          short s16;
typedef unsigned long  u32;
typedef          long  s32;

typedef long time_in ;


#define disable_int_no_interrupt() _DINT()
#define enable_int_no_interrupt() _EINT()


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
void init_adc(void);	//������������� ���
#define  ADC_FIFO_RCV_LEN  16           /* size of fifo ADC buffer   */

//���������
void show_display(int regim);
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
void work_with_adc_put(void);
u8 put_packet_type3(u16 info);
u8 put_packet_type4(void);
u8 put_packet_type5(void);
u8 write_asp_trn_fifo(u8 data_wr);
void init_uart1(void);
void update_diplay(void);



//���������������� ����
#define  SERIAL_FIFO_RCV_LEN  64           /* size of receive fifo serial buffer   */
#define  SERIAL_FIFO_TRN_LEN  64           /* size of transmit fifo serial buffer   */



//������ ������ ������ � ����������
//���� � ���������� ����� �� ������ ������, ��:
//��������� ����� ������, ������������ timer_hold
//����� ��������� ����� ������, ������������ timer_hold
//��� ������ ���������� � ������ ����������
#define HOLD_TIME_IRQ()  if(sleep){timer_sum_idle+=TAR-timer_hold;timer_hold=TAR;}else{timer_sum+=TAR-timer_hold;timer_hold=TAR;}
//��������� ����� ������ �������� ��������� - ���� ������ �������� ����� ���� � ������
#define SUM_TIME()       _DINT();timer_sum+=TAR-timer_hold;timer_hold=TAR;sleep=1;_BIS_SR(CPUOFF+GIE)
//��������� ����� ������ � ���������� � ������ ������ ������ �������� ���������
#define SUM_TIME_IRQ()   timer_sum_int+=TAR-timer_hold;timer_hold=TAR;sleep=0
