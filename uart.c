// $Id: uart.c,v 1.8 2003-05-16 19:29:08 peter Exp $
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
1          x7E - маркер пакета


Тип
пакета

0x01      пакет подтверждения.
          подтверждается прием пакета с указанным порядковым номером
0x02      пакет синхронизации времени
          В пакете передается Фаза на момент передачи по последовательному порту
          пакет не требует подтверждения и идет без порядкового номера
0x03      пакет данных ЭКГ
*/

#define  SERIAL_FIFO_RCV_LEN  64           /* size of receive fifo serial buffer   */
#define  SERIAL_FIFO_TRN_LEN  64           /* size of transmit fifo serial buffer   */
#define  MAXQUE 8		//длина очереди пакетов
#define  MAXPACKETLEN	64	//максимальная длина одного пакета
#define  CRCPACKET	2	//смещение (с конца) положения в пакете CRC
#define  LENPACKET	3	//смещение (с конца) положения в пакете длины пакета
#define  TYPEPACKET     4	//смещение (с конца) положения в пакете типа пакета
#define  NUMPACKET	5	//смещение (с конца) положения в пакете номера пакета
#define  DATA3PACKET    17	//смещение (с конца) положения в пакете размещения данных
#define  SIZEDATA3	12	//количество байт данных в пакете типа 3
#define  EOFPACKET	0x7E	//код признака конца кадра
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

u8 put_packet_type3(u8 *info){
int n;
int crc;
	//захватываем свободный пакет
 while ((n=hold_packet())==MAXQUE) ; 
	//копируем туда данные для пакета
 memcpy(&packets[(n+1)*MAXPACKETLEN-DATA3PACKET],info,SIZEDATA3); 
	//помещаем в пакет его длину (без завершающего EOFPACKET)
 packets[(n+1)*MAXPACKETLEN-LENPACKET]=DATA3PACKET;
	//помещаем (и увеличиваем) порядковый номер пакета
 packets[(n+1)*MAXPACKETLEN-NUMPACKET]=counts_packet;
 queue[n].numeric=counts_packet++;
	//указываем тип пакета
 packets[(n+1)*MAXPACKETLEN-TYPEPACKET]=0x03;
	//подсчитываем и помещаем CRC пакета
 crc=crc16(&packets[(n+1)*MAXPACKETLEN-DATA3PACKET],DATA3PACKET-3);
 packets[(n+1)*MAXPACKETLEN-CRCPACKET]=crc>>8;
 packets[(n+1)*MAXPACKETLEN-CRCPACKET+1]=crc;

	//в справочном массиве указываем длину пакета
 queue[n].len=DATA3PACKET;
 queue[n].busy=NOTSENDED;
return 1;
}
u8 put_packet_type4(void){
int n;
int crc;
 while ((n=hold_packet())==MAXQUE) ; //захватываем свободный пакет
 packets[(n+1)*MAXPACKETLEN-11]='0';
 packets[(n+1)*MAXPACKETLEN-10]='9';
 packets[(n+1)*MAXPACKETLEN-9]='8';
 packets[(n+1)*MAXPACKETLEN-8]='7';
 packets[(n+1)*MAXPACKETLEN-7]='6';
 packets[(n+1)*MAXPACKETLEN-6]='5';
 packets[(n+1)*MAXPACKETLEN-5]='4';
 packets[(n+1)*MAXPACKETLEN-4]='3';
 packets[(n+1)*MAXPACKETLEN-3]='2';
 crc=crc16(&packets[(n+1)*MAXPACKETLEN-13],10);
 packets[(n+1)*MAXPACKETLEN-CRCPACKET]=crc>>8;
 packets[(n+1)*MAXPACKETLEN-CRCPACKET+1]=crc;
 queue[n].len=12;
 queue[n].busy=NOTSENDED;
return 1;
}



//u8 test[4]={'0','1','2','3'};
void work_with_serial(void){
u16 ostatok;
int x;
u8* data;
u16 len;
for (x=0;x<MAXQUE;x++){
 switch(queue[x].busy){
  case NOTSENDED:
//отладка   queue[x].busy=WAIT_ACK;
   queue[x].busy=FREEPLACE; 	//для отладки
   data=&packets[(x+1)*MAXPACKETLEN-queue[x].len];
   len=queue[x].len;
	//посылаем (с ожиданием) весь пакет
   while ( (ostatok=send_serial_massiv(data,len))!=0) {
    data+=(len-ostatok);
    len=ostatok;
    }
	//высылаем маркер конца пакета
   while (write_asp_trn_fifo(EOFPACKET)==0) ;
   return;
//   break;
  }
 }
}


//инициализация serial port 0
void init_uart0(void){
  UCTL0 = CHAR;                         // 8-bit character
  UTCTL0 = SSEL0;                       // UCLK = ACLK
  UBR00 = 0x45;                         // 8MHz 115200
  UBR10 = 0x00;                         // 8MHz 115200
  UMCTL0 = 0x00;                        // 8MHz 115200 modulation
  ME1 |= UTXE0 + URXE0;                 // Enable USART0 TXD/RXD
  IE1 |= URXIE0;                        // Enable USART0 RX interrupt

}
//инициализация serial port 1
void init_uart1(void){
int x;

  UCTL1 = CHAR;                         // 8-bit character
  UTCTL1 = SSEL1;                       // UCLK = SMCLK
//  UBR01 = 0x45;                         // 8Mhz/115200 - 69.44
//  UBR11 = 0x00;                         //
//  UMCTL1 = 0x2C;                        // modulation

  UBR01 = 0x80;                         //7.372.800/19200 = 384 (0x180)
  UBR11 = 0x01;                         //
  UMCTL1 = 0x00;                        // no modulation

  ME2 |= UTXE1 + URXE1;                 // Enable USART1 TXD/RXD
  IE2 |= URXIE1;			// Enable USART1 RX+TX interrupt
//+ UTXIE1;                
  for (x=0;x<MAXQUE;x++) queue[x].busy=FREEPLACE;

}





//прерывание serial port 1
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
//чтение буфера приема
u16 read_asp_rcv_fifo(void){ 
 if (asp_rcv_fifo_start==asp_rcv_fifo_end) return 0;
 return (asp_rcv_fifo_buf[asp_rcv_fifo_start++ & (SERIAL_FIFO_RCV_LEN-1)]|0x0100);
}

u16 escape_sym;
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
    else
     asp_trn_fifo_buf[t_end++]=*data++;
   counter--;
   }
  len-=counter1;
  asp_trn_fifo_end+=counter1;
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
 enable_int_no_interrupt();
 return 1;
}

