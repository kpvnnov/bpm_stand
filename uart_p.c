// $Id: uart_p.c,v 1.1 2004-03-07 21:31:50 peter Exp $
/* ����� ࠡ��� � ����⠬� */

//⨯� ��६�����
#include "type_def.h"

// ���������� 䠩� ��� ࠡ��� � ����⠬�
#include "uart_p.h"

#ifdef STEND
#include "uart_s.h"
#endif // STEND

// "inline" �㭪樨 ��� 149 ������
#include "m149.h"
#include  <msp430x14x.h>


#include "crc.h"


 // �㦥��� ����� ��� ����⮢
struct que queue[MAXQUE];

 //����� ����⮢
u8 packets[MAXQUE*MAXPACKETLEN];

 //���浪��� ����� �����
u8 counts_packet; 


// "��墠�" ����� ��� ࠡ��� � ��� 
// ������ ��뢠���� � ����饭�묨 ���뢠��ﬨ
int hold_packet(void){
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

// �ନ஢���� �����
//�� ��室� 0 - �᫨ ��� ᢮������ ����⮢
//          x - ����室��� ����� ����ᥭ�, ����� �����
u16 put_packet(void){
u16 num_packet;
	//��墠�뢠�� ᢮����� �����
 disable_int_no_interrupt();
 num_packet=hold_packet();
 enable_int_no_interrupt();
 if (num_packet==MAXQUE) {
  #ifdef DEBUG_SERIAL
  packet_fifo_full++;
  #endif //DEBUG_SERIAL
  return 0; //᢮������ ����⮢ ���
  }

 #ifdef DEBUG_SERIAL
 packet_in_fifo++;
 if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL

 return num_packet;

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
	DATA8PACKET};

// ��������� ����� ����묨, ����� ����� ��।����� 㦥 +1
void fill_date_packet(u8 type_packet, u16 num_packet){
 u16 shift_fifo;
 int crc;

 //����塞 ᬥ饭�� ����� � ����� ����� ��������� �� �������
 shift_fifo=(num_packet--)*MAXPACKETLEN;
  //����頥� � ����� ��� ����� (��� �������饣� EOFPACKET)
 packets[shift_fifo-LENPACKET]=len_of_packets[type_packet];
  //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
 queue[num_packet].len=len_of_packets[type_packet];
  //����頥� (� 㢥��稢���) ���浪��� ����� �����
 packets[shift_fifo-NUMPACKET]=counts_packet;
 queue[num_packet].numeric=counts_packet++;
  //㪠�뢠�� ⨯ �����
 packets[shift_fifo-TYPEPACKET]=type_packet;
  //������뢠�� � ����頥� CRC �����
 crc=crc16(&packets[shift_fifo-len_of_packets[type_packet]],len_of_packets[type_packet]-2);
 packets[shift_fifo-CRCPACKET]=crc>>8;
 packets[shift_fifo-CRCPACKET+1]=crc;

 queue[num_packet].busy=NOTSENDED;
}


