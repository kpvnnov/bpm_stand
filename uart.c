// $Id: uart.c,v 1.26 2004-05-13 14:45:46 peter Exp $
#include  <msp430x14x.h>
#include  <string.h>

//типы переменных
#include "type_def.h"

#ifdef STEND
#include "uart_s.h"
#endif // STEND

// модуль для работы с пакетами
#include "uart_p.h"
//данные пакетов
extern u8 packets[MAXQUE*MAXPACKETLEN];
// служебные данные для пакетов
extern struct que queue[MAXQUE];



// "inline" функции для 149 процессора
#include "m149.h"

#include "global.h"


/* модуль работы с последовательным портом msp430f149 */


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

extern u16 stat_buf[STAT_FIFO_RCV_LEN*SIZE_STAT];
extern unsigned int  stat_rcv_fifo_start;      /* stat receive buffer start index      */
extern volatile unsigned int  stat_rcv_fifo_end;        /* stat receive  buffer end index        */

extern u16 stat1_buf[STAT1_FIFO_RCV_LEN*SIZE_STAT1];
extern unsigned int  stat1_rcv_fifo_start;      /* stat receive buffer start index      */
extern volatile unsigned int  stat1_rcv_fifo_end;        /* stat receive  buffer end index        */


#ifdef DEBUG_SERIAL
u16 packet_in_fifo;
u16 packet_in_fifo_max;
u16 fifo_trn_depth;
u16 packet_fifo_full;
u16 fifo_trn_depth_max;
u16 error_uart_depth;
u16 error_send_serial;
u16 length_sended_2_fifo;
u16 length_sended_2_fifo_max;
u16 length_sended_2_fifo_min;
u16 error_packets_crc;
u16 jitter_adc;
u16 jitter_adc_max;
u16 jitter_pusk;
#endif //DEBUG_SERIAL

u16 current_rec_packet;
u16 rec_length;
u16 received_packed;

extern unsigned int results0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int results1[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int results2[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int results3[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results4[ADC_FIFO_RCV_LEN];
//extern unsigned int results5[ADC_FIFO_RCV_LEN];
//extern unsigned int results6[ADC_FIFO_RCV_LEN];
//extern unsigned int results7[ADC_FIFO_RCV_LEN];
//extern unsigned int results8[ADC_FIFO_RCV_LEN];
extern unsigned int results[ADC_FIFO_RCV_LEN];
extern u16 dac[NUM_CHANEL];
extern u16 chanel;


/*
Количество Значение
  байт
_____

1          порядковый
           номер 
           пакета
1          тип
           пакета
1          длина пакета (без маркера конца пакета)
2          CRC
1          x7E - маркер пакета


*/


#define  CHANNEL_TO_SET 6       //значение номера канала АЦП в 8-ом пакете
#define  NUM_OF_CHANNEL_DAC 7   //значение номера канала ЦАП в 8-ом пакете
#define  VALVE_OF_CHANNEL_DAC 9 //значение для ЦАП в 8-ом пакете


#define  DATA3PACKET    24      //смещение (с конца) положения в пакете размещения данных
#define  DATA5PACKET     (SIZE_STAT*2+6)
#define  DATA6PACKET     (SIZE_STAT1*2+6)
#define  DATA7PACKET     (32+6)



#define  ESCAPE         0x7D
#define  EOFPACKET      0x7E    //код признака конца кадра
volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
u16   asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */





//u8 test[4]={'0','1','2','3'};
u16 last_sended_packet;




#ifdef STEND
//прерывание serial port 0
interrupt[UART0TX_VECTOR] void usart0_tx (void)
{
#else

 #ifdef CABLE
//прерывание serial port 1
interrupt[UART1TX_VECTOR] void usart1_tx (void)
{
 #else  //CABLE
    error directives
 #endif 
#endif

#ifdef CABLE
    int temp_led;
#endif
    HOLD_TIME_IRQ()
 #ifdef CABLE
    temp_led=P1OUT;
 #endif

#ifdef CABLE
    TXBUF1 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
#endif //CABLE

#ifdef STEND
    TXBUF0 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
#endif //STEND


 #ifdef DEBUG_SERIAL
    if (fifo_trn_depth_max<fifo_trn_depth) fifo_trn_depth_max=fifo_trn_depth;
    fifo_trn_depth--;
 #endif
    if (asp_trn_fifo_start==asp_trn_fifo_end){ // если данных больше нет
  #ifdef CABLE
        IE2 &= ~UTXIE1;                          // то запрещаем прерыв. передачи
  #endif //CABLE

  #ifdef STEND
        IE1 &= ~UTXIE0;                          // то запрещаем прерыв. передачи
  #endif //STEND

  #ifdef DEBUG_SERIAL
        if (fifo_trn_depth) {error_uart_depth++;fifo_trn_depth=0;}
  #endif
        //фифошка обнулилась - хватит спать, надо передавать данные
        _BIC_SR_IRQ(CPUOFF);               // Clear LPM0, SET BREAKPOINT HERE
    }
 #ifdef CABLE
    P1OUT = temp_led;                     // return led state
 #endif
    //для отладки
    //обязательно УБРАТЬ ЭТОТ АНТИСЛИП!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    //_BIC_SR_IRQ(CPUOFF);             // Clear CPUOFF bits from 0(SR)
    //!!!!!!!!!!!!!!!!!!!!!!!

    SUM_TIME_IRQ_NOSLEEP();
}
u16 esc_rec;
#ifdef STEND
//прерывание serial port 0
interrupt[UART0RX_VECTOR] void usart0_rx (void)
{
#else
 #ifdef CABLE
//прерывание serial port 1
interrupt[UART1RX_VECTOR] void usart1_rx (void)
{
 #else
    error defines stend or cable
 #endif //cable
#endif //stend

    u16 received_sym;
    u16 shift_fifo;
    HOLD_TIME_IRQ()
 #ifdef CABLE
    received_sym=RXBUF1;
 #endif //cable
 #ifdef STEND
    received_sym=RXBUF0;
 #endif //stend
    if (current_rec_packet>=MAXQUE){
        current_rec_packet=hold_packet();
        if (current_rec_packet>=MAXQUE) return; // свободный пакет не дали
        rec_length=0;
    }
    shift_fifo=current_rec_packet*MAXPACKETLEN+rec_length;
    switch(received_sym){
    case ESCAPE:
        esc_rec=1;
        SUM_TIME_IRQ_NOSLEEP();
        break;
    case EOFPACKET:
        queue[current_rec_packet].busy=PACKREC;
        queue[current_rec_packet].len=rec_length;
        current_rec_packet=MAXQUE;
        received_packed++;
        _BIC_SR_IRQ(CPUOFF);             // Clear CPUOFF bits from 0(SR)
        SUM_TIME_IRQ();
        break;
    default:
        if (esc_rec){
            esc_rec=0;
            packets[shift_fifo++]=received_sym^0x40;
            rec_length++;
            if (rec_length>=MAXPACKETLEN){ //переполнение приема (нет EOF)
                rec_length=0;
            }
        }
        else{
            packets[shift_fifo++]=received_sym;
            rec_length++;
            if (rec_length>=MAXPACKETLEN){ //переполнение приема (нет EOF)
                rec_length=0;
            }
        }
        SUM_TIME_IRQ_NOSLEEP();
        break;
    }




}

/* в fifo на передачу пересылает данные пакета, с перекодированием
ESC последовательности
Возвращает количество байт, которые еще не удалось передать */

u16 send_serial_massiv(u8* data,u16 len){
    static u16 escape_sym;
    u16 counter;
    u16 counter1;
    u16 t_end;
    u16 t_start;

#ifdef DEBUG_SERIAL
    if (len==0){
        error_send_serial++;
        return 0;
    }
#endif
    // do {
    t_end=  asp_trn_fifo_end&(SERIAL_FIFO_TRN_LEN-1);
    t_start=asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1);
    if (t_end>=t_start) //очередь еще не перешла через край и хвост впереди
    {
        counter=SERIAL_FIFO_TRN_LEN-t_end;
        if (t_start==0) counter--;
    }
    else
        counter=t_start-t_end-1;
    //  if (counter==0) break;
    if (counter==0) return len;
    if (counter>len) counter=len;
    counter1=counter;
    while(counter){
        if (escape_sym){
            escape_sym=0;
            asp_trn_fifo_buf[t_end++]=(*data++)^0x40;
        }
        else
            if (*data==EOFPACKET||*data==ESCAPE){
                escape_sym=1;
                asp_trn_fifo_buf[t_end++]=ESCAPE;
                len++;
            }
            else{
                asp_trn_fifo_buf[t_end++]=*data++;
            }
        counter--;
    }
    len-=counter1;
  #ifdef DEBUG_SERIAL
    _DINT();
  #endif
    asp_trn_fifo_end+=counter1;
  #ifdef DEBUG_SERIAL
    fifo_trn_depth+=counter1;
    _EINT();                              // Enable interrupts
  #endif
    disable_int_no_interrupt();
    //если фифошка не пустая и прерывания запрещены, то разрешаем их
    if ( ((asp_trn_fifo_end&(SERIAL_FIFO_TRN_LEN-1))!=(asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)))
            //&&
       )
#ifdef CABLE
        IE2 |= UTXIE1;          // данные в фифошке есть - разрешаем прерывания передачи
#endif //CABLE
#ifdef STEND
    IE1 |= UTXIE0;              // данные в фифошке есть - разрешаем прерывания передачи
#endif //STEND

    enable_int_no_interrupt();
    // }while(len);
    return len;
}
u8 write_asp_trn_fifo(u8 data_wr){

    if ( ( (asp_trn_fifo_end+1)&(SERIAL_FIFO_TRN_LEN-1))== (asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)) )
        return 0;
    disable_int_no_interrupt();
    asp_trn_fifo_buf[asp_trn_fifo_end++ & (SERIAL_FIFO_TRN_LEN-1)]=data_wr;
 #ifdef DEBUG_SERIAL
    fifo_trn_depth++;
 #endif
#ifdef CABLE
    IE2 |= UTXIE1;              // данные в фифошке есть - разрешаем прерывания передачи
#endif //CABLE
#ifdef STEND
    IE1 |= UTXIE0;              // данные в фифошке есть - разрешаем прерывания передачи
#endif //STEND

    enable_int_no_interrupt();
    return 1;
}


void work_with_serial(void){
    int x;
    for (x=0;x<MAXQUE;x++){
        if (last_sended_packet<MAXQUE) break;
        switch(queue[x].busy){
        case NOTSENDED:
            last_sended_packet=x;
            break;
        }//hctiws
    }//rof
    if (last_sended_packet<MAXQUE){
        x=last_sended_packet;
        //посылаем (сколько в фифошку поместится) пакет
  #ifdef DEBUG_SERIAL
        length_sended_2_fifo=queue[x].len;
  #endif //DEBUG_SERIAL
        queue[x].len=send_serial_massiv(&packets[(x+1)*MAXPACKETLEN-queue[x].len],queue[x].len);
  #ifdef DEBUG_SERIAL
        length_sended_2_fifo-=queue[x].len;
        if (length_sended_2_fifo_max<length_sended_2_fifo)
            length_sended_2_fifo_max=length_sended_2_fifo;
        if (length_sended_2_fifo_min>length_sended_2_fifo)
            length_sended_2_fifo_min=length_sended_2_fifo;
        //здесь необходимо поместить эти данные в fifo статистики
  #endif //DEBUG_SERIAL

        //если все отправили
        if (queue[x].len==0){
            //высылаем маркер конца пакета
            while (write_asp_trn_fifo(EOFPACKET)==0) ;
            //помечаем пакет отмеченным или ожидающем подтверждения
            //отладка   queue[x].busy=WAIT_ACK;

            //для отладки
            queue[x].busy=FREEPLACE;
   #ifdef DEBUG_SERIAL
            packet_in_fifo--;
   #endif //DEBUG_SERIAL
            //для отладки

            //ищем следующий пакет в очереди на отправку
            while((++last_sended_packet)<MAXQUE){
                switch(queue[last_sended_packet].busy){
                    //найден неотправленный пакет
                case NOTSENDED:
                    return;
                }
            }//elihw дошли до конца очереди - пакетов на отправку нет
        }//fi - текущий пакет отправлен еще не до конца
    }//fi - пока нет пакетов для отправки
}
