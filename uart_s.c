// $Id: uart_s.c,v 1.1 2003-06-06 13:34:58 peter Exp $
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
extern u16 packet_in_fifo;
extern u16 packet_in_fifo_max;
extern u16 fifo_trn_depth;
extern u16 packet_fifo_full;
extern u16 fifo_trn_depth_max;
extern u16 error_uart_depth;
extern u16 error_send_serial;
extern u16 length_sended_2_fifo;
extern u16 length_sended_2_fifo_max;
extern u16 length_sended_2_fifo_min;
#endif //DEBUG_SERIAL

extern u16 current_rec_packet;
extern u16 rec_length;
extern u16 received_packed;

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
extern volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

extern unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
extern u16   asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */

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

extern struct que queue[MAXQUE];
extern u8 packets[MAXQUE*MAXPACKETLEN];

extern u8 counts_packet; //порядковый номер пакета




u8 put_packet_type3(u16 info){
int n;
int crc;
int shift_fifo;
u16* t_p;
//u8* t_r;
	//захватываем свободный пакет
 disable_int_no_interrupt();
 n=hold_packet();
 enable_int_no_interrupt();
 if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
  packet_fifo_full++;
  #endif //DEBUG_SERIAL
  return 0; //свободных пакетов нет
  }
 #ifdef DEBUG_SERIAL
 packet_in_fifo++;
 if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
 shift_fifo=(n+1)*MAXPACKETLEN;
	//копируем туда данные для пакета
 t_p=(u16*)&packets[shift_fifo-DATA3PACKET];

 *t_p++=results0[info];
 *t_p++=results1[info];
 *t_p++=results2[info];
 *t_p++=results3[info];
// *t_p++=results4[info];
// *t_p++=results5[info];
// *t_p++=results6[info];
// *t_p++=results7[info];
// *t_p++=results8[info];


	//помещаем в пакет его длину (без завершающего EOFPACKET)
 packets[shift_fifo-LENPACKET]=DATA3PACKET;
	//помещаем (и увеличиваем) порядковый номер пакета
 packets[shift_fifo-NUMPACKET]=counts_packet;
 queue[n].numeric=counts_packet++;
	//указываем тип пакета
 packets[shift_fifo-TYPEPACKET]=0x03;
	//подсчитываем и помещаем CRC пакета
 crc=crc16(&packets[shift_fifo-DATA3PACKET],DATA3PACKET-2);
 packets[shift_fifo-CRCPACKET]=crc>>8;
 packets[shift_fifo-CRCPACKET+1]=crc;

	//в справочном массиве указываем длину пакета
 queue[n].len=DATA3PACKET;
 queue[n].busy=NOTSENDED;
return 1;
}
//отправляем пакет со статистикой
u16 put_packet_type5(void){
int n;
//int x;
int crc;
int shift_fifo;
//u16* t_p;
//u16* t_s;
	//захватываем свободный пакет
 disable_int_no_interrupt();
 n=hold_packet();
 enable_int_no_interrupt();
 if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
  packet_fifo_full++;
  #endif //DEBUG_SERIAL
  return 0; //свободных пакетов нет
  }
 #ifdef DEBUG_SERIAL
 packet_in_fifo++;
 if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
 shift_fifo=(n+1)*MAXPACKETLEN;
	//копируем туда данные для пакета
// t_p=(u16*)&packets[shift_fifo-DATA5PACKET];
// t_s=(u16*)&stat_buf[(stat_rcv_fifo_start & (STAT_FIFO_RCV_LEN-1))*SIZE_STAT];

// for(x=0;x<SIZE_STAT;x++)
//  *t_p++=*t_s++;
  memcpy(&packets[shift_fifo-DATA5PACKET],&stat_buf[(stat_rcv_fifo_start & (STAT_FIFO_RCV_LEN-1))*SIZE_STAT],SIZE_STAT*2);


 stat_rcv_fifo_start++;


	//помещаем в пакет его длину (без завершающего EOFPACKET)
 packets[shift_fifo-LENPACKET]=DATA5PACKET;
	//помещаем (и увеличиваем) порядковый номер пакета
 packets[shift_fifo-NUMPACKET]=counts_packet;
 queue[n].numeric=counts_packet++;
	//указываем тип пакета
 packets[shift_fifo-TYPEPACKET]=0x05;
	//подсчитываем и помещаем CRC пакета
 crc=crc16(&packets[shift_fifo-DATA5PACKET],DATA5PACKET-2);
 packets[shift_fifo-CRCPACKET]=crc>>8;
 packets[shift_fifo-CRCPACKET+1]=crc;

	//в справочном массиве указываем длину пакета
 queue[n].len=DATA5PACKET;
 queue[n].busy=NOTSENDED;
return 1;
}
u16 put_packet_type6(void){
int n;
//int x;
int crc;
int shift_fifo;
	//захватываем свободный пакет
 disable_int_no_interrupt();
 n=hold_packet();
 enable_int_no_interrupt();
 if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
  packet_fifo_full++;
  #endif //DEBUG_SERIAL
  return 0; //свободных пакетов нет
  }
 #ifdef DEBUG_SERIAL
 packet_in_fifo++;
 if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
 shift_fifo=(n+1)*MAXPACKETLEN;
	//копируем туда данные для пакета
 memcpy(&packets[shift_fifo-DATA6PACKET],&stat1_buf[(stat1_rcv_fifo_start & (STAT1_FIFO_RCV_LEN-1))*SIZE_STAT1],SIZE_STAT1*2);


 stat1_rcv_fifo_start++;


	//помещаем в пакет его длину (без завершающего EOFPACKET)
 packets[shift_fifo-LENPACKET]=DATA6PACKET;
	//помещаем (и увеличиваем) порядковый номер пакета
 packets[shift_fifo-NUMPACKET]=counts_packet;
 queue[n].numeric=counts_packet++;
	//указываем тип пакета
 packets[shift_fifo-TYPEPACKET]=0x06;
	//подсчитываем и помещаем CRC пакета
 crc=crc16(&packets[shift_fifo-DATA6PACKET],DATA6PACKET-2);
 packets[shift_fifo-CRCPACKET]=crc>>8;
 packets[shift_fifo-CRCPACKET+1]=crc;

	//в справочном массиве указываем длину пакета
 queue[n].len=DATA6PACKET;
 queue[n].busy=NOTSENDED;
return 1;
}
u8 put_packet_type4(void){
int n;
int crc;
int shift_fifo;
	//захватываем свободный пакет
 disable_int_no_interrupt();
 n=hold_packet();
 enable_int_no_interrupt();
 if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
  packet_fifo_full++;
  #endif //DEBUG_SERIAL
  return 0; //свободных пакетов нет
  }
 #ifdef DEBUG_SERIAL
 packet_in_fifo++;
 if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL

 shift_fifo=(n+1)*MAXPACKETLEN;
 packets[shift_fifo-11]='0';
 packets[shift_fifo-10]='9';
 packets[shift_fifo-9]='8';
 packets[shift_fifo-8]='7';
 packets[shift_fifo-7]='6';
 packets[shift_fifo-6]='5';
 packets[shift_fifo-5]='4';
 packets[shift_fifo-4]='3';
 packets[shift_fifo-3]='2';
 packets[shift_fifo-TYPEPACKET]=0x04;
 crc=crc16(&packets[shift_fifo-13],11);
 packets[shift_fifo-CRCPACKET]=crc>>8;
 packets[shift_fifo-CRCPACKET+1]=crc;
 queue[n].len=13;
 queue[n].busy=NOTSENDED;
return 1;
}

u8 put_packet_type7(u16 info){
int n;
int crc;
int shift_fifo;
u16* t_p;
//u8* t_r;
	//захватываем свободный пакет
 disable_int_no_interrupt();
 n=hold_packet();
 enable_int_no_interrupt();
 if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
  packet_fifo_full++;
  #endif //DEBUG_SERIAL
  return 0; //свободных пакетов нет
  }
 #ifdef DEBUG_SERIAL
 packet_in_fifo++;
 if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
 shift_fifo=(n+1)*MAXPACKETLEN;
	//копируем туда данные для пакета
 t_p=(u16*)&packets[shift_fifo-DATA7PACKET];

 memcpy(t_p,&results0[info*SIZE_OF_ADC_DUMP],SIZE_OF_ADC_DUMP*2);
 memcpy(t_p+SIZE_OF_ADC_DUMP,&results1[info*SIZE_OF_ADC_DUMP],SIZE_OF_ADC_DUMP*2);
 packets[shift_fifo-DATA7PACKET+2*SIZE_OF_ADC_DUMP]=results[info];

	//помещаем в пакет его длину (без завершающего EOFPACKET)
 packets[shift_fifo-LENPACKET]=DATA7PACKET;
	//помещаем (и увеличиваем) порядковый номер пакета
 packets[shift_fifo-NUMPACKET]=counts_packet;
 queue[n].numeric=counts_packet++;
	//указываем тип пакета
 packets[shift_fifo-TYPEPACKET]=0x07;
	//подсчитываем и помещаем CRC пакета
 crc=crc16(&packets[shift_fifo-DATA7PACKET],DATA7PACKET-2);
 packets[shift_fifo-CRCPACKET]=crc>>8;
 packets[shift_fifo-CRCPACKET+1]=crc;

	//в справочном массиве указываем длину пакета
 queue[n].len=DATA7PACKET;
 queue[n].busy=NOTSENDED;
return 1;
}



void work_with_serial_rec(void){
int x;
int crc;
u16 shift_fifo;
 for (x=0;x<MAXQUE;x++){
  if (queue[x].busy==PACKREC){
   received_packed--;
   //подсчитать CRC
   shift_fifo=x*MAXPACKETLEN+queue[x].len;
   if (crc16(&packets[x*MAXPACKETLEN],queue[x].len)==0){ //сrc совпала?
    switch(packets[shift_fifo-TYPEPACKET]){
     case 0x08:
      dac[packets[shift_fifo-NUM_OF_CHANNEL_DAC]&(NUM_CHANEL-1)]=packets[shift_fifo-VALVE_OF_CHANNEL_DAC]+(packets[shift_fifo-VALVE_OF_CHANNEL_DAC+1]<<8);
      chanel=packets[shift_fifo-CHANNEL_TO_SET];
      break;
     }
    //отправить подтверждение
    shift_fifo=(x+1)*MAXPACKETLEN;
   	//помещаем порядковый номер пакета для подтверждения
    packets[shift_fifo-NUMPACKET]=packets[x*MAXPACKETLEN+queue[x].len-NUMPACKET];
    packets[shift_fifo-LENPACKET]=NUMPACKET;	//длина подтверждающего пакета
   	//указываем тип пакета
    packets[shift_fifo-TYPEPACKET]=0x01;		//подтверждающий пакет
   	//подсчитываем и помещаем CRC пакета
    crc=crc16(&packets[shift_fifo-NUMPACKET],NUMPACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;
  
   	//в справочном массиве указываем длину пакета
    queue[x].len=NUMPACKET;
    #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
    #endif //DEBUG_SERIAL
    queue[x].busy=NOTSENDED;
    }
   else{
    queue[x].busy=FREEPLACE;
    }
   }
  }
}


//u8 test[4]={'0','1','2','3'};
extern u16 last_sended_packet;


//инициализация serial port 0
void init_uart0(void){

}
//инициализация serial port 1
void init_uart(void){
int x;

#ifdef CABLE
  UCTL1 = CHAR;                         // 8-bit character
  UTCTL1 = SSEL1;                       // UCLK = SMCLK
//  UBR01 = 0x45;                         // 8Mhz/115200 - 69.44
//  UBR11 = 0x00;                         //
//  UMCTL1 = 0x2C;                        // modulation

  UBR01 = 0x80;                         //7.372.800/19200 = 384 (0x180)
  UBR11 = 0x01;                         //
  UMCTL1 = 0x00;                        // no modulation

  UBR01 = 0x40;                         //7.372.800/115200 = 64 (0x40)
  UBR11 = 0x00;                         //
  UMCTL1 = 0x00;                        // no modulation

//  UBR01 = 0xC0;                         //7.372.800/38400 = 192 (0xC0)
//  UBR11 = 0x00;                         //
//  UMCTL1 = 0x00;                        // no modulation

  ME2 |= UTXE1 + URXE1;                 // Enable USART1 TXD/RXD
  IE2 |= URXIE1;			// Enable USART1 RX+TX interrupt
//+ UTXIE1;                
  for (x=0;x<MAXQUE;x++) queue[x].busy=FREEPLACE;
  last_sended_packet=MAXQUE;
 #ifdef DEBUG_SERIAL
  fifo_trn_depth_max=0;
  fifo_trn_depth=0;
  packet_in_fifo=0;
  packet_fifo_full=0;
 #endif
#endif //CABLE
#ifdef STEND

  UCTL0 = CHAR;                         // 8-bit character
  UTCTL0 = SSEL1;                       // UCLK = SMCLK
//  UBR00 = 0x45;                         // 8Mhz/115200 - 69.44
//  UBR10 = 0x00;                         //
//  UMCTL0 = 0x2C;                        // modulation

//  UBR00 = 0x80;                         //7.372.800/19200 = 384 (0x180)
//  UBR10 = 0x01;                         //
//  UMCTL0 = 0x00;                        // no modulation

  UBR00 = 0x40;                         //7.372.800/115200 = 64 (0x40)
  UBR10 = 0x00;                         //
  UMCTL0 = 0x00;                        // no modulation

  ME1 |= UTXE0 + URXE0;                 // Enable USART1 TXD/RXD
  IE1 |= URXIE0;			// Enable USART1 RX+TX interrupt
//+ UTXIE0;                
  for (x=0;x<MAXQUE;x++) queue[x].busy=FREEPLACE;
  last_sended_packet=MAXQUE;
  current_rec_packet=0;
  rec_length=0;
  queue[0].busy=PACKBUSY;
 #ifdef DEBUG_SERIAL
  fifo_trn_depth_max=0;
  fifo_trn_depth=0;
  packet_in_fifo=0;
  packet_fifo_full=0;
 #endif
  received_packed=0;
#endif //STEND

dac[0]=0x7FF;
}



