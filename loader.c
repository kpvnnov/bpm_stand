// $Id: loader.c,v 1.3 2004-03-18 16:51:14 peter Exp $
#include  <msp430x14x.h>
//типы переменных
#include "type_def.h"


//дефайнеры загрузчика
#include "uart_l.h"


// модуль для работы с пакетами
#include "uart_p.h"

//контрольная сумма
#include "crc.h"

#include <string.h>
//#include "m149.h"

//данные пакетов
extern u8 packets[MAXQUE*MAXPACKETLEN];
// служебные данные для пакетов
extern struct que queue[MAXQUE];

//по какому порту работает загрузчик (1 или 2)
#define COM_PORT        2

#pragma codeseg(LOADER_CODE)

u8 buffer_data[512]; // буфер промежуточный - равен одной странице программирования

volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index   отсюда берем данные   */

unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
u16   asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */


u16 rec_length;
u16 current_rec_packet;
u16 last_sended_packet;


//сатистика
u16 received_packed;
u16 error_packets_crc;


u16 current_speed;

u16 check_loader(void);

void load(void);
void init_uart(void);
int run_xt2(void);
void packet_processing(void);
void send_serial(void);
u16 rec_serial(u8*);
u16 receive_symbol(u8 received_sym);
u16 send_serial_massiv(u8* data,u16 len);
void work_with_serial(void);



void main(void){
 int x;

  // останавливаем watchdog
 WDTCTL=WDTPW|WDTHOLD;                  // Stop WDT


 for (x=0;x<MAXQUE;x++) queue[x].busy=FREEPLACE;
 rec_length=0;
 current_rec_packet=0;
 current_speed=0;
 queue[0].busy=PACKBUSY;
 last_sended_packet=MAXQUE;
 asp_trn_fifo_start=asp_trn_fifo_end=0;
 run_xt2();


 init_uart();

 if (check_loader())
  load();
 }

void send_serial(void){
 if (asp_trn_fifo_start!=asp_trn_fifo_end) {


#if COM_PORT==1
  if ((IFG1 & UTXIFG0))        // USART1 TX buffer ready?
   {
    TXBUF0 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
   }

#elif COM_PORT==2
  if ((IFG2 & UTXIFG1))        // USART1 TX buffer ready?
   {
    TXBUF1 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
   }
#else
 неправильно определен COM порт
#endif
  }
}

u8 write_asp_trn_fifo(u8 data_wr){

 if ( ( (asp_trn_fifo_end+1)&(SERIAL_FIFO_TRN_LEN-1))== (asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)) )
  return 0;
 asp_trn_fifo_buf[asp_trn_fifo_end++ & (SERIAL_FIFO_TRN_LEN-1)]=data_wr;
 #ifdef DEBUG_SERIAL
  fifo_trn_depth++;
 #endif
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
   while (write_asp_trn_fifo(EOFPACKET)==0) send_serial();
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


//осуществляется прием/передача по опросу
u16 rec_serial(u8* rec_data){
#if COM_PORT==1
 if (IFG1&URXIFG0){     //есть данные приема
  if (rec_data) *rec_data=RXBUF0;
  return 1;
  }
 else
  return 0;
#elif COM_PORT==2
 if (IFG2&URXIFG1){     //есть данные приема
  if (rec_data) *rec_data=RXBUF1;
  return 1;
  }
 else
  return 0;
#else
 неправильно определен COM порт
#endif
}
void load(void){

u8 received_data;
 do{
  //отправляем данные из фифошки
  send_serial();
  work_with_serial();
  //принимаем данные
  if (rec_serial(&received_data)){ //если данные есть
   if (receive_symbol(received_data)) // если принятый символ сформировал пакет
    packet_processing();
   }
 }while(1);
}

void packet_processing(void){
int x;
int crc;
u16 shift_fifo;
u16 address;
u16 packet_hold;
 for (x=0;x<MAXQUE;x++){
  if (queue[x].busy==PACKREC){
   received_packed--;
   if (queue[x].len==0) {
    queue[x].busy=FREEPLACE;
    continue;
    }
   //подсчитать CRC
   shift_fifo=x*MAXPACKETLEN+queue[x].len;
   if (crc16(&packets[x*MAXPACKETLEN],queue[x].len)==0){ //сrc совпала?
    switch(packets[shift_fifo-TYPEPACKET]){
     case 0x1B: //данные для программирования
      address=packets[shift_fifo-ADDRESS_PACK1B];
      if (address<(sizeof(buffer_data)-SIZE_N_LOADER)){
       memcpy(&buffer_data[address],&packets[shift_fifo-ADDRESS_PACK1B_DATA],SIZE_N_LOADER);
       }
      break;
     case 0x1C: //запрос данных из промежуточного буфера
      address=packets[shift_fifo-ADDRESS_PACK1C];
      if ((packet_hold=make_packet())!=0){
       shift_fifo=packet_hold*MAXPACKETLEN;
       memcpy(&packets[shift_fifo-ADDRESS_PACK25_DATA],&buffer_data[address],SIZE_N_LOADER);
       memcpy(&packets[shift_fifo-ADDRESS_PACK25],&address,2);
       fill_date_packet(0x25,packet_hold);
       queue[packet_hold-1].busy=NOTSENDED;
       }
      break;
     case 0x20: //запрос чтения флеш
      break;
     case 0x22: //очистка сектора
      break;
     case 0x23: //запрос версии загрузчика
      break;
     case 0x26: //запись в сектор данных из промежуточного буфера
      break;
     default: // пакет не обрабатывается
      break; //необходимо отослать информацию о том, что данный пакет не обрабатывается
     }
    //отправить подтверждение, если это не 0x01 тип пакета
    if (packets[shift_fifo-TYPEPACKET]!=0x01){
     shift_fifo=(x+1)*MAXPACKETLEN;
        //помещаем порядковый номер пакета для подтверждения
     packets[shift_fifo-NUMPACKET]=packets[x*MAXPACKETLEN+queue[x].len-NUMPACKET];
     packets[shift_fifo-LENPACKET]=NUMPACKET;   //длина подтверждающего пакета
        //указываем тип пакета
     packets[shift_fifo-TYPEPACKET]=0x01;               //подтверждающий пакет
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
    else{ // это пакет 0x01 типа, подтверждающий
     queue[x].busy=FREEPLACE;
     }
    }
   else{ // crc не совпала
    queue[x].busy=FREEPLACE;
    error_packets_crc++;
    }
   }
  }

}

u16 check_loader(void){
return 1;
}

//осуществляется прием посимвольно
//при встрече символа конца пакета возвращает 1
// если недостаточно места возвращает 2
u16 receive_symbol(u8 received_sym){
 u16 shift_fifo;
 static u8 esc_rec;
 u16 ret_code=0;
 if (current_rec_packet>=MAXQUE){
  current_rec_packet=hold_packet();
  if (current_rec_packet>=MAXQUE) return 2; // свободный пакет не дали
  rec_length=0;
  }
 shift_fifo=current_rec_packet*MAXPACKETLEN+rec_length;
 switch(received_sym){
  case ESCAPE:
   esc_rec=1;
   break;
  case EOFPACKET:
   queue[current_rec_packet].busy=PACKREC;
   queue[current_rec_packet].len=rec_length;
   current_rec_packet=MAXQUE;
   received_packed++;
   ret_code=1;
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
   break;
  }
return ret_code;
}


void init_uart(void){
#if COM_PORT==1
   UCTL0 = CHAR;                         // 8-bit character
   UTCTL0 = SSEL1;                       // UCLK = SMCLK

   UBR00 = 0x40;                         //7.372.800/115200 = 64 (0x40)
   UBR10 = 0x00;                         //
   UMCTL0 = 0x00;                        // no modulation

   ME1 |= UTXE0 + URXE0;                 // Enable USART1 TXD/RXD

  P3SEL |= 0x30;                        // P3.4,5 = USART0 TXD/RXD
  P3DIR |= 0x10;                        // P3.4 output direction

#elif COM_PORT==2
   UCTL1 = CHAR;                         // 8-bit character
   UTCTL1 = SSEL1;                       // UCLK = SMCLK

//   UBR01 = 0x80;                         //7.372.800/19200 = 384 (0x180)
//   UBR11 = 0x01;                         //
   UBR01 = 0x40;                         //7.372.800/115200 = 64 (0x40)
   UBR11 = 0x00;                         //
   UMCTL1 = 0x00;                        // no modulation

   ME2 |= UTXE1 + URXE1;                 // Enable USART1 TXD/RXD

  P3SEL |= 0xC0;                        // P3.6,7 = USART1 option select
  P3DIR |= 0x20;                        // P3.6 = output direction

#else
 неправильно определена переменная COM_PORT
#endif
}

int run_xt2(void){
int i;
int count=10;
  BCSCTL1 &= ~XT2OFF;                   // XT2 = HF XTAL
  do 
  {
  IFG1 &= ~OFIFG;                       // Clear OSCFault flag
  for (i = 0xFF; i > 0; i--);           // Time for flag to set
  count--;
  if (count==0) break;
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?                

  if ((IFG1 & OFIFG)==0){
   BCSCTL2 = (BCSCTL2&(~SELM0))|SELM1|SELS;   // MCLK = XT2 (safe)
   current_speed=1;
   }
  return count;
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
   asp_trn_fifo_end+=counter1;
  #ifdef DEBUG_SERIAL
  // _DINT();
  #endif
  #ifdef DEBUG_SERIAL
   fifo_trn_depth+=counter1;
  // _EINT();                              // Enable interrupts
  #endif
// }while(len);
 return len;
}

