// $Id: uart_p.c,v 1.3 2004-03-18 16:51:15 peter Exp $
/* модуль работы с пакетами */

//типы переменных
#include "type_def.h"

// заголовочный файл для работы с пакетами
#include "uart_p.h"

#ifdef STEND
#include "uart_s.h"
#endif // STEND

#ifdef LOADER
#include "uart_l.h"
#endif // LOADER

// "inline" функции для 149 процессора
#include "m149.h"
#include  <msp430x14x.h>


#include "crc.h"


 // служебные данные для пакетов
struct que queue[MAXQUE];

 //данные пакетов
u8 packets[MAXQUE*MAXPACKETLEN];

 //порядковый номер пакета
u8 counts_packet; 


// "захват" пакета для работы с ним 
// должен вызываться с запрещенными прерываниями
u16 hold_packet(void){
int x;
 for (x=0;x<MAXQUE;x++){
  switch(queue[x].busy){
   case FREEPLACE:
    queue[x].busy=PACKBUSY;
    return x;
   }
  }
return MAXQUE;
}

// формирование пакета
//на выходе 0 - если нет свободных пакетов
//          x - необходимые данные занесены, номер пакета
u16 make_packet(void){
u16 num_packet;
        //захватываем свободный пакет
 disable_int_no_interrupt();
 num_packet=hold_packet();
 enable_int_no_interrupt();
 if (num_packet==MAXQUE) {
  #ifdef DEBUG_SERIAL
  packet_fifo_full++;
  #endif //DEBUG_SERIAL
  return 0; //свободных пакетов нет
  }

 #ifdef DEBUG_SERIAL
 packet_in_fifo++;
 if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL

 return num_packet+1;

}

const unsigned int  len_of_packets[]={
        DATA0PACKET,
        DATA1PACKET,
        DATA2PACKET,
        DATA3PACKET,
        DATA4PACKET,
        DATA5PACKET,
        DATA6PACKET,
        DATA7PACKET,
        DATA8PACKET,
        DATA9PACKET,
        DATAxAPACKET,
        DATAxBPACKET,
        DATAxCPACKET,
        DATAxDPACKET,
        DATAxEPACKET,
        DATAxFPACKET,
        DATA10PACKET,
        DATA11PACKET,
        DATA12PACKET,
        DATA13PACKET,
        DATA14PACKET,
        DATA15PACKET,
        DATA16PACKET,
        DATA17PACKET,
        DATA18PACKET,
        DATA19PACKET,
        DATA1APACKET,
        DATA1BPACKET,
        DATA1CPACKET,
        DATA1DPACKET,
        DATA1EPACKET,
        DATA1FPACKET,
        DATA20PACKET,
        DATA21PACKET,
        DATA22PACKET,
        DATA23PACKET,
        DATA24PACKET,
        DATA25PACKET,
        DATA26PACKET};

// заполнить пакет данными, номер пакета передается уже +1
// u8 type_packet - тип пакета
// u16 num_packet - номер пакета
void fill_date_packet(u8 type_packet, u16 num_packet){
 u16 shift_fifo;
 int crc;

 //вычисляем смещение пакета и номер пакета УМЕНЬШАЕМ на единицу
 shift_fifo=(num_packet--)*MAXPACKETLEN;
  //помещаем в пакет его длину (без завершающего EOFPACKET)
 packets[shift_fifo-LENPACKET]=len_of_packets[type_packet];
  //в справочном массиве указываем длину пакета
 queue[num_packet].len=len_of_packets[type_packet];
  //помещаем (и увеличиваем) порядковый номер пакета
 packets[shift_fifo-NUMPACKET]=counts_packet;
 queue[num_packet].numeric=counts_packet++;
  //указываем тип пакета
 packets[shift_fifo-TYPEPACKET]=type_packet;
  //подсчитываем и помещаем CRC пакета
 crc=crc16(&packets[shift_fifo-len_of_packets[type_packet]],len_of_packets[type_packet]-2);
 packets[shift_fifo-CRCPACKET]=crc>>8;
 packets[shift_fifo-CRCPACKET+1]=crc;

 queue[num_packet].busy=NOTSENDED;
}


