// $Id: loader.c,v 1.2 2004-03-17 11:34:16 peter Exp $
#include  <msp430x14x.h>
//типы переменных
#include "type_def.h"


//дефайнеры загрузчика
#include "uart_l.h"


// модуль для работы с пакетами
#include "uart_p.h"

//контрольная сумма
#include "crc.h"


//данные пакетов
extern u8 packets[MAXQUE*MAXPACKETLEN];
// служебные данные для пакетов
extern struct que queue[MAXQUE];

#pragma codeseg(LOADER_CODE)

u16 buffer_data[512]; // буфер промежуточный - равен одной странице программирования

u16 rec_length;
u16 current_rec_packet;


//сатистика
u16 received_packed;
u16 error_packets_crc;

u16 check_loader(void);

void load(void);

void main(void){
 rec_length=0;
 current_rec_packet=0;

 if (check_loader())
  load();
 }
void load(void){

int x;
int crc;
u16 shift_fifo;
u16 address;
 for (x=0;x<MAXQUE;x++){
  if (queue[x].busy==PACKREC){
   received_packed--;
   //подсчитать CRC
   shift_fifo=x*MAXPACKETLEN+queue[x].len;
   if (crc16(&packets[x*MAXPACKETLEN],queue[x].len)==0){ //сrc совпала?
    switch(packets[shift_fifo-TYPEPACKET]){
     case 0x1B: //данные для программирования
      address=packets[shift_fifo-ADDRESS_PACK1B];
      break;
     case 0x1C: //запрос данных из промежуточного буфера
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

void receive_symbol(u8 received_sym){
 u16 shift_fifo;
 static u8 esc_rec;
 if (current_rec_packet>=MAXQUE){
  current_rec_packet=hold_packet();
  if (current_rec_packet>=MAXQUE) return; // свободный пакет не дали
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

}


