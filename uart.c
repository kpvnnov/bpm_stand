// $Id: uart.c,v 1.19 2003-06-06 13:34:58 peter Exp $
#include  <msp430x14x.h>
#include  <string.h>
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


Тип
пакета

0x01      пакет подтверждения.
          подтверждается прием пакета с указанным порядковым номером
0x02      пакет синхронизации времени
          В пакете передается Фаза на момент передачи по последовательному порту
          пакет не требует подтверждения и идет без порядкового номера
0x03      пакет данных ЭКГ
0x05      пакет данных статистики
0x06      пакет данных статистики
0x07      пакет данных с двух каналов
0x08      пакет данных установки канала и АЦП
    два байта - значение для ЦАП
    байт      - номер канала ЦАП
    байт      - номер канала для преобразования
                в котором - SxMA AMMM
                S - 0 - основной, 1 - резервный
                x - зарезервировано
                A - значение канала АЦП (PRESS_1 - PRESS_4)
                M - значение для MUX (A0-A3)
*/


#define  MAXQUE 8		//длина очереди пакетов
#define  CRCPACKET	2	//смещение (с конца) положения в пакете CRC
#define  LENPACKET	3	//смещение (с конца) положения в пакете длины пакета
#define  TYPEPACKET     4	//смещение (с конца) положения в пакете типа пакета
#define  NUMPACKET	5	//смещение (с конца) положения в пакете номера пакета
#define  CHANNEL_TO_SET 6       //значение номера канала АЦП в 8-ом пакете
#define  NUM_OF_CHANNEL_DAC 7   //значение номера канала ЦАП в 8-ом пакете
#define  VALVE_OF_CHANNEL_DAC 9 //значение для ЦАП в 8-ом пакете


#define  DATA3PACKET    24	//смещение (с конца) положения в пакете размещения данных
#define  DATA5PACKET     (SIZE_STAT*2+6)
#define  DATA6PACKET     (SIZE_STAT1*2+6)
#define  DATA7PACKET     (32+6)



#define  ESCAPE		0x7D
#define  EOFPACKET	0x7E	//код признака конца кадра
volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
u16   asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */

//unsigned int  asp_rcv_fifo_start;      /* serial receive buffer start index      */

//volatile unsigned int  asp_rcv_fifo_end;        /* serial receive flash buffer end index        */

//u8    asp_rcv_fifo_buf[SERIAL_FIFO_RCV_LEN];           /* storage for receive transmit  buffer      */

#define FREEPLACE 0	//свободный пакет
#define PACKBUSY  1	//занятый под обработку
#define NOTSENDED 2	//неотправленный пакет
#define WAIT_ACK  3     //ожидающий подтверждения
#define PACKREC   4	//принятый пакет

struct que{
 u8 busy;    //место занято. 
		//0 - свободно
		//1 - занято
		//2 не передано (необходимо подтверждение)
		//3 передано (ждем подтверждения)
		//4 не передано (подтверждения не потребуется)
 u8 numeric; //порядковый номер пакета
 u8 len;
};

struct que queue[MAXQUE];
u8 packets[MAXQUE*MAXPACKETLEN];

u8 counts_packet; //порядковый номер пакета

int hold_packet(void){
int x;
 for (x=0;x<MAXQUE;x++){
  switch(queue[x].busy){
   case FREEPLACE:
    queue[x].busy=PACKBUSY;
    return x;
//    break;
   }
  }
return MAXQUE;
}
const unsigned int  CrcTable16[256]={
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0};

/*
unsigned int  update_crc_16(unsigned char octet,unsigned int crc)
{
 return  CrcTable16[((crc>>8) ^ octet) & 0xFF] ^ ((crc<<8) & 0xFF00);

}
*/

unsigned int crc16(void* massiv, unsigned int len){
unsigned int crc=0x0000;
unsigned int c;
unsigned char*m=(unsigned char*)massiv;
for (c=0;c<len;c++)
 crc=CrcTable16[((crc>>8) ^ m[c]) & 0xFF] ^ ((crc<<8) & 0xFF00); 
return crc;
}





//u8 test[4]={'0','1','2','3'};
u16 last_sended_packet;




#ifdef STEND
//прерывание serial port 0
interrupt[UART0TX_VECTOR] void usart0_tx (void)
{
#endif //STEND

#ifdef CABLE
//прерывание serial port 1
interrupt[UART1TX_VECTOR] void usart1_tx (void)
{
#endif //CABLE

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
#endif //stend
#ifdef CABLE
//прерывание serial port 1
interrupt[UART1RX_VECTOR] void usart1_rx (void)
{
#endif //cable

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

u16 escape_sym;
u16 send_serial_massiv(u8* data,u16 len){
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
   IE2 |= UTXIE1;		// данные в фифошке есть - разрешаем прерывания передачи
#endif //CABLE
#ifdef STEND
   IE1 |= UTXIE0;		// данные в фифошке есть - разрешаем прерывания передачи
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
 IE2 |= UTXIE1;		// данные в фифошке есть - разрешаем прерывания передачи
#endif //CABLE
#ifdef STEND
 IE1 |= UTXIE0;		// данные в фифошке есть - разрешаем прерывания передачи
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
