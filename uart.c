// $Id: uart.c,v 1.5 2003-05-13 15:11:43 peter Exp $
#include  <msp430x14x.h>
#include  <string.h>
#include "global.h"


/*
Количество Значение
  байт
_____

1          порядковый
           номер 
           пакета
1          тип
           пакета
1          длина пакета
2          CRC
1          x7B - маркер пакета


Тип
пакета

0x01      пакет подтверждения.
          подтверждается прием пакета с указанным порядковым номером
0x02      пакет синхронизации времени
          В пакете передается Фаза на момент передачи по последовательному порту
          пакет не требует подтверждения и идет без порядкового номера
0x03      пакет данных ЭКГ
*/

#define  SERIAL_FIFO_RCV_LEN  64           /* size of fifo serial buffer   */
#define  SERIAL_FIFO_TRN_LEN  32           /* size of fifo serial buffer   */
#define  MAXQUE 8		//длина очереди пакетов
#define  MAXPACKETLEN	64	//максимальная длина одного пакета
#define  CRCPACKET	3	//смещение (с конца) положения в пакете CRC
#define  LENPACKET	4	//смещение (с конца) положения в пакете длины пакета
#define  TYPEPACKET     5	//смещение (с конца) положения в пакете типа пакета
#define  NUMPACKET	6	//смещение (с конца) положения в пакете номера пакета
#define  DATA3PACKET    18	//смещение (с конца) положения в пакете размещения данных
#define  SIZEDATA3	12	//количество байт данных в пакете типа 3
volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
u16   asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */

unsigned int  asp_rcv_fifo_start;      /* serial receive buffer start index      */

volatile unsigned int  asp_rcv_fifo_end;        /* serial receive flash buffer end index        */

u8    asp_rcv_fifo_buf[SERIAL_FIFO_RCV_LEN];           /* storage for receive transmit  buffer      */

#define NOTSENDED 2
#define WAIT_ACK  3
#define FREEPLACE 0
#define PACKBUSY  1
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

u8 put_packet_type3(u8 *info){
int n;
int crc;
 while ((n=hold_packet())==MAXQUE) ; //захватываем свободный пакет
 memcpy(&packets[(n+1)*MAXPACKETLEN-DATA3PACKET],info,SIZEDATA3); //копируем туда данные для пакета
 packets[(n+1)*MAXPACKETLEN-LENPACKET]=DATA3PACKET;
 packets[(n+1)*MAXPACKETLEN-NUMPACKET]=counts_packet++;
 packets[(n+1)*MAXPACKETLEN-TYPEPACKET]=0x03;
 packets[(n+1)*MAXPACKETLEN-CRCPACKET]=0xFF;
 packets[(n+1)*MAXPACKETLEN-CRCPACKET+1]=0xFF;
 crc=crc16(&packets[(n+1)*MAXPACKETLEN-DATA3PACKET],DATA3PACKET);
 packets[(n+1)*MAXPACKETLEN-CRCPACKET]=crc;
 packets[(n+1)*MAXPACKETLEN-CRCPACKET+1]=crc<<8;
 queue[n].numeric=packets[(n+1)*MAXPACKETLEN-NUMPACKET];
 queue[n].len=DATA3PACKET;
 queue[n].busy=NOTSENDED;
return 1;
}

void work_with_serial(void){
 work_serial_transmit();
}

void send_full_massiv(u8* data,u16 len){
u16 ostatok;
 while ( (ostatok=send_serial_massiv(data,len))!=0) {
  data+=(len-ostatok);
  len=ostatok;
  }
}

void work_serial_transmit(void){
int x;
for (x=0;x<MAXQUE;x++){
 switch(queue[x].busy){
  case NOTSENDED:
//отладка   queue[x].busy=WAIT_ACK;
   queue[x].busy=FREEPLACE; 	//для отладки
   send_full_massiv(&packets[(x+1)*MAXPACKETLEN-queue[x].len],queue[x].len);
   return;
//   break;
  }
 }
}


void init_uart0(void){
  UCTL0 = CHAR;                         // 8-bit character
  UTCTL0 = SSEL0;                       // UCLK = ACLK
  UBR00 = 0x45;                         // 8MHz 115200
  UBR10 = 0x00;                         // 8MHz 115200
  UMCTL0 = 0x00;                        // 8MHz 115200 modulation
  ME1 |= UTXE0 + URXE0;                 // Enable USART0 TXD/RXD
  IE1 |= URXIE0;                        // Enable USART0 RX interrupt

}
void init_uart1(void){
int x;

  UCTL1 = CHAR;                         // 8-bit character
  UTCTL1 = SSEL1;                       // UCLK = SMCLK
  UBR01 = 0x45;                         // 8Mhz/115200 - 69.44
  UBR11 = 0x00;                         //
  UMCTL1 = 0x2C;                        // modulation
  ME2 |= UTXE1 + URXE1;                 // Enable USART1 TXD/RXD
  IE2 |= URXIE1;			// Enable USART1 RX+TX interrupt
//+ UTXIE1;                
  for (x=0;x<MAXQUE;x++) queue[x].busy=FREEPLACE;

}





interrupt[UART1TX_VECTOR] void usart1_tx (void)
{
 TXBUF1 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
 if (asp_trn_fifo_start==asp_trn_fifo_end) // если данных больше нет
  IE2 &= ~UTXIE1;                          // то запрещаем прерыв. передачи
}
interrupt[UART1RX_VECTOR] void usart0_rx (void)
{

//  while ((IFG1 & UTXIFG0) == 0);        // USART0 TX buffer ready?
//  TXBUF0 = ;                      // RXBUF0 to TXBUF0


//так как нет inline, то раскрываем  write_asp_rcv_fifo(RXBUF0);
//u8 write_asp_rcv_fifo(u8 data_wr){
// if ( ( (asp_rcv_fifo_end+1)&(SERIAL_FIFO_RCV_LEN-1))== (asp_rcv_fifo_start&(SERIAL_FIFO_RCV_LEN-1)) )
//  return 0;
// asp_rcv_fifo_buf[asp_rcv_fifo_end++ & (SERIAL_FIFO_RCV_LEN-1)]=data_wr;
// return 1;
//}


// - добавить в отладочном режиме эту проверку if ( ( (asp_rcv_fifo_end+1)&(SERIAL_FIFO_RCV_LEN-1))!= (asp_rcv_fifo_start&(SERIAL_FIFO_RCV_LEN-1)) )
  asp_rcv_fifo_buf[asp_rcv_fifo_end++ & (SERIAL_FIFO_RCV_LEN-1)]=RXBUF1;



// if (temp_iosr_serial&( (1<<bit_OE)| //Overrun occurs - переполнение приема
//                 (1<<bit_FE)| //framing error
//                 (1<<bit_BI)   //break
//               )
//    ) { 
//  portADTR;
//  portADTR;	//два раза читаем порт, чтобы освободиться от данных
//  rDIOSR&=0x00FF;
//  rDIOSR|=0x2600;	//сброс флагов BI(break interrupt)
//                        //FE(framing error)
//                        //переполнения(OE-overrun error) в IOSR
//  portIOSR=rDIOSR;

}
u16 read_asp_rcv_fifo(void){ 
 if (asp_rcv_fifo_start==asp_rcv_fifo_end) return 0;
 return (asp_rcv_fifo_buf[asp_rcv_fifo_start++ & (SERIAL_FIFO_RCV_LEN-1)]|0x0100);
}


u16 send_serial_massiv(u8* data,u16 len){
u16 counter;
u16 counter1;
u16 t_end;
u16 t_start;
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
   asp_trn_fifo_buf[t_end++]=*data++;
   counter--;
   }
//  memcpy(&asp_trn_fifo_buf[t_end],data,counter);
  len-=counter1;
//  len-=counter;
  asp_trn_fifo_end+=counter1;
//  asp_trn_fifo_end+=counter;
  disable_int_no_interrupt();
  //если фифошка не пустая и прерывания запрещены, то разрешаем их
  if ( ((asp_trn_fifo_end&(SERIAL_FIFO_TRN_LEN-1))!=(asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)))
//&&
  )
   IE2 |= UTXIE1;		// данные в фифошке есть - разрешаем прерывания передачи

//        ((IFG2 & UTXIFG1) != 0)  ){	//прерывания передачи запрещены?

//   TXBUF1 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
//   }
  enable_int_no_interrupt();
// }while(len);
return len;
}
u8 write_asp_trn_fifo(u8 data_wr){

 if ( ( (asp_trn_fifo_end+1)&(SERIAL_FIFO_TRN_LEN-1))== (asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)) )
  return 0;
 disable_int_no_interrupt();
 asp_trn_fifo_buf[asp_trn_fifo_end++ & (SERIAL_FIFO_TRN_LEN-1)]=data_wr;
 IE2 |= UTXIE1;		// данные в фифошке есть - разрешаем прерывания передачи
// if ((IFG2 & UTXIFG1) != 0)        // USART0 TX buffer ready?
//  TXBUF1 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
 enable_int_no_interrupt();
 return 1;
}

unsigned int crc16(void* massiv,int len){
return 0;
}
