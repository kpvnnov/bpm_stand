//$Id: global.h,v 1.2 2003-04-03 07:27:31 peter Exp $
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


//таймер
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





// АЦП
void init_adc(void);	//инициализация АЦП

//индикатор
void show_display(int regim);
void redraw_display_second(int force);
void redraw_display_minutes(int force);
void redraw_display_voltage(int force);
void redraw_display_celciy(int force);
void cs_on_display(void);
void cs_off_display(void);
