// $Id: loader.c,v 1.2 2004-03-17 11:34:16 peter Exp $
#include  <msp430x14x.h>
//���� ����������
#include "type_def.h"


//��������� ����������
#include "uart_l.h"


// ������ ��� ������ � ��������
#include "uart_p.h"

//����������� �����
#include "crc.h"


//������ �������
extern u8 packets[MAXQUE*MAXPACKETLEN];
// ��������� ������ ��� �������
extern struct que queue[MAXQUE];

#pragma codeseg(LOADER_CODE)

u16 buffer_data[512]; // ����� ������������� - ����� ����� �������� ����������������

u16 rec_length;
u16 current_rec_packet;


//���������
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
   //���������� CRC
   shift_fifo=x*MAXPACKETLEN+queue[x].len;
   if (crc16(&packets[x*MAXPACKETLEN],queue[x].len)==0){ //�rc �������?
    switch(packets[shift_fifo-TYPEPACKET]){
     case 0x1B: //������ ��� ����������������
      address=packets[shift_fifo-ADDRESS_PACK1B];
      break;
     case 0x1C: //������ ������ �� �������������� ������
      break;
     case 0x20: //������ ������ ����
      break;
     case 0x22: //������� �������
      break;
     case 0x23: //������ ������ ����������
      break;
     case 0x26: //������ � ������ ������ �� �������������� ������
      break;
     default: // ����� �� ��������������
      break; //���������� �������� ���������� � ���, ��� ������ ����� �� ��������������
     }
    //��������� �������������, ���� ��� �� 0x01 ��� ������
    if (packets[shift_fifo-TYPEPACKET]!=0x01){
     shift_fifo=(x+1)*MAXPACKETLEN;
        //�������� ���������� ����� ������ ��� �������������
     packets[shift_fifo-NUMPACKET]=packets[x*MAXPACKETLEN+queue[x].len-NUMPACKET];
     packets[shift_fifo-LENPACKET]=NUMPACKET;   //����� ��������������� ������
        //��������� ��� ������
     packets[shift_fifo-TYPEPACKET]=0x01;               //�������������� �����
        //������������ � �������� CRC ������
     crc=crc16(&packets[shift_fifo-NUMPACKET],NUMPACKET-2);
     packets[shift_fifo-CRCPACKET]=crc>>8;
     packets[shift_fifo-CRCPACKET+1]=crc;
  
        //� ���������� ������� ��������� ����� ������
     queue[x].len=NUMPACKET;
     #ifdef DEBUG_SERIAL
     packet_in_fifo++;
     if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
     #endif //DEBUG_SERIAL
     queue[x].busy=NOTSENDED;
     }
    else{ // ��� ����� 0x01 ����, ��������������
     queue[x].busy=FREEPLACE;
     }
    }
   else{ // crc �� �������
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
  if (current_rec_packet>=MAXQUE) return; // ��������� ����� �� ����
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
    if (rec_length>=MAXPACKETLEN){ //������������ ������ (��� EOF)
     rec_length=0;
     }
    }
   else{
    packets[shift_fifo++]=received_sym;
    rec_length++;
    if (rec_length>=MAXPACKETLEN){ //������������ ������ (��� EOF)
     rec_length=0;
     }
    }
   break;
  }

}


