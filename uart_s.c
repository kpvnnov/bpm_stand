// $Id: uart_s.c,v 1.18 2004-06-07 12:32:14 peter Exp $
#include  <msp430x14x.h>
#include  <string.h>

//⨯� ��६�����
#include "type_def.h"


#include "global.h"
#include "uart_s.h"

// "inline" �㭪樨 ��� 149 ������
#include "m149.h"

// ����� ��� ࠡ��� � ����⠬�
#include "uart_p.h"


#define STEND_N 2

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

u16 stop_transmit;
u16 analog_on;
u16 temperature;
u16 stop_adc;
u16 send_correction_temperature;

extern u16 chanel_convert;


extern unsigned int current_level;
extern unsigned int what_doing;
unsigned int to_level;
extern unsigned int valve_hold;

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
extern u16 error_packets_crc;
#endif //DEBUG_SERIAL

extern u16 current_rec_packet;
extern u16 rec_length;
extern u16 received_packed;

//extern unsigned int results0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results1[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results2[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results3[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results4[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results5[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
//extern unsigned int results6[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];

extern unsigned int one_count0[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int one_count1[ADC_FIFO_RCV_LEN*SIZE_OF_ADC_DUMP];
extern unsigned int multi_count0[NUM_MULTICHANNEL][ADC_FIFO_RCV_LEN];
extern unsigned int multi_count1[NUM_MULTICHANNEL][ADC_FIFO_RCV_LEN];



extern unsigned int results[ADC_FIFO_RCV_LEN];
extern u16 dac[NUM_CHANEL];
extern u16 chanel;


/*
����� �� ᪮��� 115200

������⢮ ���祭��
  ����
_____

X      ����� �����
 �㦥��� ����� �����:
1      ���浪��� ����� �����
       ( �᫨ ⨯ ����� 0x01 - � �� ����� ���⢥ত������ �����)
1      ⨯ �����
1      ����� ����� (��� ��થ� ���� ����� = X+5)
2      CRC16 (���訩 ���� ���� ����, ��⮬ ����訩)
       CRC16 - ��⠥��� ���ᨢ ������ � ��砫� ����� ������
       �� ���� ���騬 ��। ����� CRC16 (����� �����) �����⥫쭮
       ��᫥ �⮣� ����祭��� CRC �������� � ᫥���饬 ���浪�:
       ���訩 ����, ����訩 ����
1      x7E - ��થ� �����


 � 楫�� ������� (���७��) ����� ����� ������ ��� ⨯�
�ணࠬ���� ॠ������ ࠧ��� ᮤ�ন���� ����� (������ �����)
����⥫쭮 ������ �� "������" �����, 
� �㦥���� ���������� - �� 墮�� ����� (��થ� ����)

 �� ��।�� �� ������ ���� ����� (����� �����+�㦥��� �
㦥 �����⠭�묨 ���ﬨ � CRC, �� �᪫�祭��� ��થ� ���� 
����� 0x7E) ������������ �� ����稥 ���� � ������
 0x7D (ESCAPE)
 0x7E (EOF)
⠪�� ����� ���������� �� ��᫥����⥫쭮��� �� ���� ����
0x7D,��室�� ����� XOR 0x40
� ����
 �뫮          �⠫�
 0x7D          0x7D 0x3D
 0x7E          0x7D 0x3E

�� �ਥ���� ��஭�:
 ��⮪ ������������ �� ����㯫���� ����
0x7D (ESCAPE).  �� ����祭�� ������� ���祭�� ᠬ ��� �⮣� 
���� � ������� �� �砢����, � ��� ���祭�� ᫥���饣� ����
����室��� �஢��� ������ XOR 0x40. ����祭�� �����
��᫥ ������ ����樨 �� ����� �㦥��묨 ������ (EOF ���
ESCAPE), � ����� ��몭������ ᮤ�ন�� �����.
 �� ����祭�� ���� 0x7E (EOF) ����室��� �������� CRC
��� �ਭ���� ������ ��। �⨬ ����� (�᪫��� ᠬ ᨬ��� EOF)
�� १������饬 ���祭�� CRC = 0 ����஫쭠� �㬬� ��⠥��� 
ᮢ���襩. �������⥫�� ����஫�� 楫��⭮�� �����
����� �㦨�� ᮢ������� ������⢠ ॠ�쭮 �ਭ���� ������
� ���祭��� 㪠����� � �㦥���� ���� ����� "�����".
���
�����

0x01      ����� ���⢥ত����.
          ���⢥ত����� �ਥ� ����� � 㪠����� ���浪��� ����஬
_________________________________________________________________
0x02      ����� ᨭ�஭���樨 �६���
          � ����� ��।����� ���� �� ������ ��।�� �� 
          ��᫥����⥫쭮�� �����
          ����� �� �ॡ�� ���⢥ত���� � ���� ��� ���浪����� �����
_________________________________________________________________
0x03      ����� ������ ���
_________________________________________________________________
0x05      ����� ������ ����⨪�
_________________________________________________________________
0x06      ����� ������ ����⨪� 1
_________________________________________________________________
0x07      ����� ������ � ���� �������
          ᮤ�ঠ��� �����:
          (0) ᫮�� - ���祭�� ���୮�� ������ 
          (1) ᫮�� - ���祭�� ���୮�� ������ (᫥���饥 �� �६���)
          .....
          (7) ᫮�� - ���祭�� ���୮�� ������ (᫥���饥 �� �६���)
          (8) ᫮�� - ���祭�� ��᫥�㥬��� ������
          (9) ᫮�� - ���祭�� ��᫥�㥬��� ������ (᫥���饥 �� �६���)
          .....
          (15) ᫮�� - ���祭�� ��᫥�㥬��� ������ (᫥���饥 �� �६���)
          1 ���� - ��१�ࢨ஢���
          ��饥 ������⢮ ������ 16*2+1=33 ����
_________________________________________________________________
0x08      ����� ������ ��⠭���� ������ ���
        ᮤ�ঠ��� �����:
         ��� ���� - ���祭�� ��� ���
         ����      - ����� ������ ���
        ��饥 ������⢮ ������ 2+1=3 ����
_________________________________________________________________
0x09      ����� ������ ��⠭���� ������ ���
       ᮤ�ঠ��� �����:
    ����      - ����� ������ ��� �८�ࠧ������
                � ���஬ - SXMM MAAM
                S - 0 - �᭮����, 1 - १�ࢭ�
                X -  0 - ०�� �뢮�� ��� ����⮢ ���୮�� � 
                         ��᫥�㥬��� ������ (����� 0x07)
                     1 - ०�� �뢮�� �㬬���� ���祭��
                A - ���祭�� ������ ��� (PRESS_1 - PRESS_4)
                M - ���祭�� ��� MUX (A0-A3)
_________________________________________________________________
0x0A      ����� ������ � ������� (���㬬�஢����)
       ᮤ�ঠ��� �����:
    ���� - ������⢮ �뢮����� ������� (N)
    ���� - ����� ��ࢮ�� ������
    ᫮�� - ���祭�� ���୮�� ������ � ᬥ饭��� 0
    ᫮�� - ���祭�� ��᫥�㥬��� ������ � ᬥ饭��� 0
    ᫮�� - ���祭�� ���୮�� ������ � ᬥ饭��� 1
    ᫮�� - ���祭�� ��᫥�㥬��� ������ � ᬥ饭��� 1
    .....
    ᫮�� - ���祭�� ���୮�� ������ � ᬥ饭��� (N-1)
    ᫮�� - ���祭�� ��᫥�㥬��� ������ � ᬥ饭��� (N-1)
    ᫮�� - ���祭�� ⥬������� (�㬬�஢���� 16 ����⮢)
    1 ���� - ��१�ࢨ஢���
    ��饥 ������⢮ ������ 2+N*2+1 ����
_________________________________________________________________
0x0B      ������� ������
     ᮤ�ঠ��� �����:
      ���
_________________________________________________________________
0x0�      ������ ������
     ᮤ�ঠ��� �����:
      ���
_________________________________________________________________
0x0D      ������ �������� �� �ॡ㥬�� ����稭�
     ᮤ�ঠ��� �����:
          � ����� ᫮�� (+1 ���� reserved) - ��᮫�⭮� 
          ���祭�� �� ���ண�  ����室��� ��������� � ������� ������
          12 ��� ⥪�饣� ���୮�� (�᭮����� ��� १�ࢭ���)
          ������ 
    ��饥 ������⢮ ������ 3 ����
_________________________________________________________________
0x0E      ������� �������� �� 㪠������ ����稭�
     ᮤ�ঠ��� �����:
          � ����� ᫮�� (+1 ���� reserved) - ��᮫�⭮� 
          ���祭�� �� ���ண� ����室��� ������� ��������
          �।���⥫쭮 ����室��� ���� ��������
          ������� ������
    ��饥 ������⢮ ������ 3 ����
_________________________________________________________________
0x0F      �몫���� ��������. ������ ������� �४�頥�
          ����⢨� ����� 0x0E
     ᮤ�ঠ��� �����:
      ���
_________________________________________________________________
0x10      ��⠭���� ���ࢠ�� ⠩���� (ᥪ) � �祭��
          ���ண� ������� ������� �������� �� �ॡ㥬���
     ᮤ�ঠ��� �����:
          � ����� 1 ����, �� 㬮�砭�� 30.
          �� ���祭�� �⮣� �६��� ���뫠���� �㦥��� �����
          0x11
    ��饥 ������⢮ ������ 1 ����
_________________________________________________________________
0x11      �㦥��� ����� ᨣ���������騩 � ���祭��
          ⠩���� ����窨
     ᮤ�ঠ��� �����:
      ���
_________________________________________________________________
0x12      ��⠭���� ���ࢠ�� ⠩���� (ᥪ) � �祭��
          ���ண� ������� ������� �������� �� �ॡ㥬���
          (�� 㬮�砭�� 5)
     ᮤ�ঠ��� �����:
          � ����� 1 ����
    ��饥 ������⢮ ������ 1 ����
          �� ���祭�� �⮣� �६��� ���뫠���� �㦥��� �����
          0x14
_________________________________________________________________
0x14      �㦥��� ����� ᨣ���������騩 � ���祭��
          ⠩���� ��᪠
     ᮤ�ঠ��� �����:
      ���
_________________________________________________________________
0x15      ��⠭���� �������� �� ���⨦���� ���ண�
          ��⮬���᪨ �믮������ 0x0F 0x0�
          � ���뫠���� �㦥��� ����� 0x16
     ᮤ�ঠ��� �����:
          � ����� ᫮�� (+1 ���� reserved) - ��᮫�⭮� 
          �।��쭮� ���祭�� ��������
    ��饥 ������⢮ ������ 3 ����
_________________________________________________________________
0x16      �㦥��� ����� ᨣ���������騩 � �ॢ�襭��
          ��������� �஢�� �������� (���਩�� �஢���)
     ᮤ�ঠ��� �����:
      ���
_________________________________________________________________
0x18      ��⠭����� ���� �ࠢ����� ०����
     ᮤ�ঠ��� �����:
          � ����� 1 ���� 
            0 ��� - ����祭�� ����殮��� �� ���稪��
            1 ��� - ����祭�� ����殮��� �� ���������� ���
            2 ��� - �몫�祭�� �८�ࠧ������ ���
            3 ��� - ����祭�� �८�ࠧ������ ���
            4 ��� - �몫�祭�� UART
            5 ��� - ����祭�� UART
            6 ��� - �몫�祭�� ⥬�������
            7 ��� - ����祭�� ⥬�������

    ��饥 ������⢮ ������ 1 ����
_________________________________________________________________
0x19      ����� ⥬�������
     ᮤ�ঠ��� �����:
      2 ���� - ����७��� ���祭��
_________________________________________________________________
0x1A      ����� ���४樨 ⥬������� (ࠧ � ᥪ㭤�)
     ᮤ�ঠ��� �����:
      2 ���� - ������
      2 ���� - ᬥ饭��
      2 ���� - ���祭�� ��� (��ࢠ� �窠)
      2 ���� - ���祭�� ⥬������� 㬭������� �� 100 (��ࢠ� �窠)
      2 ���� - ���祭�� ��� (���� �窠)
      2 ���� - ���祭�� ⥬������� 㬭������� �� 100 (���� �窠)

*/


#define  CRCPACKET      2       //ᬥ饭�� (� ����) ��������� � ����� CRC
#define  LENPACKET      3       //ᬥ饭�� (� ����) ��������� � ����� ����� �����
#define  TYPEPACKET     4       //ᬥ饭�� (� ����) ��������� � ����� ⨯� �����
#define  NUMPACKET      5       //ᬥ饭�� (� ����) ��������� � ����� ����� �����
#define  CHANNEL_TO_SET 6       //���祭�� ����� ������ ��� � 9-�� �����
#define  NUM_OF_CHANNEL_DAC 6   //���祭�� ����� ������ ��� � 8-�� �����
#define  VALVE_OF_CHANNEL_DAC 8 //���祭�� ��� ��� � 8-�� �����
#define  LEVEL_SET      8       //���祭�� ������ � 3-� ���⮢�� �����
#define  REGIM_JOB      6       //��⠭���� ०���� ࠡ���

extern volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

extern unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
extern u16   asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */

//unsigned int  asp_rcv_fifo_start;      /* serial receive buffer start index      */

//volatile unsigned int  asp_rcv_fifo_end;        /* serial receive flash buffer end index        */

//u8    asp_rcv_fifo_buf[SERIAL_FIFO_RCV_LEN];           /* storage for receive transmit  buffer      */

#define FREEPLACE 0     //᢮����� �����
#define PACKBUSY  1     //������ ��� ��ࠡ���
#define NOTSENDED 2     //����ࠢ����� �����
#define WAIT_ACK  3     //������騩 ���⢥ত����
#define PACKREC   4     //�ਭ��� �����


extern struct que queue[MAXQUE];
extern u8 packets[MAXQUE*MAXPACKETLEN];

extern u8 counts_packet; //���浪��� ����� �����




//��ࠢ�塞 ����� � ����⨪��
u16 put_packet_type5(void){
    int n;
    //int x;
    int crc;
    int shift_fifo;
    //u16* t_p;
    //u16* t_s;
    //��墠�뢠�� ᢮����� �����
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //᢮������ ����⮢ ���
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //�����㥬 �㤠 ����� ��� �����
    // t_p=(u16*)&packets[shift_fifo-DATA5PACKET];
    // t_s=(u16*)&stat_buf[(stat_rcv_fifo_start & (STAT_FIFO_RCV_LEN-1))*SIZE_STAT];

    // for(x=0;x<SIZE_STAT;x++)
    //  *t_p++=*t_s++;
    memcpy(&packets[shift_fifo-DATA5PACKET],&stat_buf[(stat_rcv_fifo_start & (STAT_FIFO_RCV_LEN-1))*SIZE_STAT],SIZE_STAT*2);


    stat_rcv_fifo_start++;


    //����頥� � ����� ��� ����� (��� �������饣� EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA5PACKET;
    //����頥� (� 㢥��稢���) ���浪��� ����� �����
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //㪠�뢠�� ⨯ �����
    packets[shift_fifo-TYPEPACKET]=0x05;
    //������뢠�� � ����頥� CRC �����
    crc=crc16(&packets[shift_fifo-DATA5PACKET],DATA5PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
    queue[n].len=DATA5PACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}
u16 put_packet_type6(void){
    int n;
    //int x;
    int crc;
    int shift_fifo;
    //��墠�뢠�� ᢮����� �����
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //᢮������ ����⮢ ���
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //�����㥬 �㤠 ����� ��� �����
    memcpy(&packets[shift_fifo-DATA6PACKET],&stat1_buf[(stat1_rcv_fifo_start & (STAT1_FIFO_RCV_LEN-1))*SIZE_STAT1],SIZE_STAT1*2);


    stat1_rcv_fifo_start++;


    //����頥� � ����� ��� ����� (��� �������饣� EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA6PACKET;
    //����頥� (� 㢥��稢���) ���浪��� ����� �����
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //㪠�뢠�� ⨯ �����
    packets[shift_fifo-TYPEPACKET]=0x06;
    //������뢠�� � ����頥� CRC �����
    crc=crc16(&packets[shift_fifo-DATA6PACKET],DATA6PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
    queue[n].len=DATA6PACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}
u8 put_packet_type4(void){
    int n;
    int crc;
    int shift_fifo;
    //��墠�뢠�� ᢮����� �����
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //᢮������ ����⮢ ���
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
    //��墠�뢠�� ᢮����� �����
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //᢮������ ����⮢ ���
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //�����㥬 �㤠 ����� ��� �����
    t_p=(u16*)&packets[shift_fifo-DATA7PACKET];

    memcpy(t_p,&one_count0[info*SIZE_OF_ADC_DUMP],SIZE_OF_ADC_DUMP*2);
    memcpy(t_p+SIZE_OF_ADC_DUMP,&one_count1[info*SIZE_OF_ADC_DUMP],SIZE_OF_ADC_DUMP*2);
    packets[shift_fifo-DATA7PACKET+4*SIZE_OF_ADC_DUMP]=results[info];

    //����頥� � ����� ��� ����� (��� �������饣� EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA7PACKET;
    //����頥� (� 㢥��稢���) ���浪��� ����� �����
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //㪠�뢠�� ⨯ �����
    packets[shift_fifo-TYPEPACKET]=0x07;
    //������뢠�� � ����頥� CRC �����
    crc=crc16(&packets[shift_fifo-DATA7PACKET],DATA7PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
    queue[n].len=DATA7PACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}
u8 put_packet_typeA(u16 info){
    int z;
    int n;
    int crc;
    int shift_fifo;
    u16* t_p;
    //��墠�뢠�� ᢮����� �����
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //᢮������ ����⮢ ���
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //�����㥬 �㤠 ����� ��� �����
    t_p=(u16*)&packets[shift_fifo-DATAxAPACKET];

    *t_p++=((results[info]&0xFF)<<8)|NUM_MULTICHANNEL;//������⢮ �뢮����� �������
    for (z=0;z<NUM_MULTICHANNEL;z++){
        *t_p++=multi_count0[z][info];
        *t_p++=multi_count1[z][info];
    }
    // *t_p++=multi_count0[NUM_MULTICHANNEL][info]+multi_count1[NUM_MULTICHANNEL][info]; //⥬������

    //����頥� � ����� ��� ����� (��� �������饣� EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATAxAPACKET;
    //����頥� (� 㢥��稢���) ���浪��� ����� �����
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //㪠�뢠�� ⨯ �����
    packets[shift_fifo-TYPEPACKET]=0x0A;
    //������뢠�� � ����頥� CRC �����
    crc=crc16(&packets[shift_fifo-DATAxAPACKET],DATAxAPACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
    queue[n].len=DATAxAPACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}
u8 put_packet_type19(u16 info){
    //int z;
    int n;
    int crc;
    int shift_fifo;
    u16* t_p;
    //��墠�뢠�� ᢮����� �����
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //᢮������ ����⮢ ���
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //�����㥬 �㤠 ����� ��� �����
    t_p=(u16*)&packets[shift_fifo-DATA19PACKET];

    *t_p++=multi_count0[0][info];//⥬������

    //����頥� � ����� ��� ����� (��� �������饣� EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA19PACKET;
    //����頥� (� 㢥��稢���) ���浪��� ����� �����
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //㪠�뢠�� ⨯ �����
    packets[shift_fifo-TYPEPACKET]=0x19;
    //������뢠�� � ����頥� CRC �����
    crc=crc16(&packets[shift_fifo-DATA19PACKET],DATA19PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
    queue[n].len=DATA19PACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}

u8 put_packet_type1A(void){
    u16* t_p;
    int n;
    int crc;
    int shift_fifo;
    //��墠�뢠�� ᢮����� �����
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //᢮������ ����⮢ ���
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL

    shift_fifo=(n+1)*MAXPACKETLEN;
    //�����㥬 �㤠 ����� ��� �����
    t_p=(u16*)&packets[shift_fifo-DATA1APACKET];

#if   STEND_N == 1
    *t_p++=41294;  //������
    *t_p++=42148;  //ᬥ饭��
    *t_p++=43344;  //���祭�� ��� (��ࢠ� �窠)
    *t_p++=2266;   //���祭�� ⥬������� 㬭������� �� 100 (��ࢠ� �窠)
    *t_p++=43680;  //���祭�� ��� (���� �窠)
    *t_p++=4460;   //���祭�� ⥬������� 㬭������� �� 100 (���� �窠)
    *t_p++=0x04A4; //����� �⥭�� A4040003
    *t_p++=0x0100; // A - �⥭�, 04 - ���, 0003 - �����
    warning!!!

#elif STEND_N == 2
    *t_p++=41294;  //������
    *t_p++=42148;  //ᬥ饭��
    *t_p++=43344;  //���祭�� ��� (��ࢠ� �窠)
    *t_p++=2266;   //���祭�� ⥬������� 㬭������� �� 100 (��ࢠ� �窠)
    *t_p++=43680;  //���祭�� ��� (���� �窠)
    *t_p++=4460;   //���祭�� ⥬������� 㬭������� �� 100 (���� �窠)
    *t_p++=0x04A4; //����� �⥭�� A4040003
    *t_p++=0x0200; // A - �⥭�, 04 - ���, 0003 - �����
#elif STEND_N == 3
    *t_p++=0;  //������
    *t_p++=0;  //ᬥ饭��
    *t_p++=0;  //���祭�� ��� (��ࢠ� �窠)
    *t_p++=0;   //���祭�� ⥬������� 㬭������� �� 100 (��ࢠ� �窠)
    *t_p++=0;  //���祭�� ��� (���� �窠)
    *t_p++=0;   //���祭�� ⥬������� 㬭������� �� 100 (���� �窠)
    *t_p++=0x04A4; //����� �⥭�� A4040003
    *t_p++=0x0300; // A - �⥭�, 04 - ���, 0003 - �����
#else
    warning!!!

#endif 

    //����頥� � ����� ��� ����� (��� �������饣� EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA1APACKET;
    //����頥� (� 㢥��稢���) ���浪��� ����� �����
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //㪠�뢠�� ⨯ �����
    packets[shift_fifo-TYPEPACKET]=0x1A;
    //������뢠�� � ����頥� CRC �����
    crc=crc16(&packets[shift_fifo-DATA1APACKET],DATA1APACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
    queue[n].len=DATA1APACKET;
    queue[n].busy=NOTSENDED;
    send_correction_temperature=0;
    return 1;
}

u8 buffer[SIZE_N_LOADER];

void work_with_serial_rec(void){
    int x;
    int crc;
    u16 shift_fifo;
    u16  address;

    for (x=0;x<MAXQUE;x++){
        if (queue[x].busy==PACKREC){
            received_packed--;
            //�������� CRC
            shift_fifo=x*MAXPACKETLEN+queue[x].len;
            if (crc16(&packets[x*MAXPACKETLEN],queue[x].len)==0){ //�rc ᮢ����?
                switch(packets[shift_fifo-TYPEPACKET]){
                case 0x08:
                    dac[packets[shift_fifo-NUM_OF_CHANNEL_DAC]&((NUM_CHANEL>>1)-1)]=packets[shift_fifo-VALVE_OF_CHANNEL_DAC]+(packets[shift_fifo-VALVE_OF_CHANNEL_DAC+1]<<8);
                    break;
                case 0x09:
                    chanel=packets[shift_fifo-CHANNEL_TO_SET];
                    break;
                case 0x0B:         //������� ������
                    close_valve();
                    break;
                case 0x0C:         //������ ������
                    open_valve();
                    break;
                case 0x0D:         //������ �������� �� �ॡ㥬�� ����稭�
                    //     ᮤ�ঠ��� �����:
                    //  � ����� ᫮�� (+1 ���� reserved) - ��᮫�⭮�
                    //  ���祭�� �� ���ண�  ����室��� ��������� � ������� ������
                    //  12 ��� ⥪�饣� ���୮�� (�᭮����� ��� १�ࢭ���)
                    //  ������
                    // ��饥 ������⢮ ������ 3 ����
                    what_doing=NO_JOB;
                    to_level=packets[shift_fifo-LEVEL_SET]+(packets[shift_fifo-LEVEL_SET+1]<<8);
                    to_level&=0xFFF;
                    open_valve();
                    off_pump();
                    what_doing=LEVEL_DOWN;
                    break;
                case 0x0E:         //������� �������� �� 㪠������ ����稭�
                    //     ᮤ�ঠ��� �����:
                    //  � ����� ᫮�� (+1 ���� reserved) - ��᮫�⭮�
                    //  ���祭�� �� ���ண� ����室��� ������� ��������
                    //  �।���⥫쭮 ����室��� ���� ��������
                    //  ������� ������
                    // ��饥 ������⢮ ������ 3 ����
                    what_doing=NO_JOB;
                    to_level=packets[shift_fifo-LEVEL_SET]+(packets[shift_fifo-LEVEL_SET+1]<<8);
                    to_level&=0xFFF;
                    close_valve();
                    on_pump();
                    what_doing=LEVEL_UP;
                    break;
                case 0x0F:         //�몫���� ��������. ������ ������� �४�頥�
                    break;
                case 0x15:         //��⠭���� �������� �� ���⨦���� ���ண�
                    break;
                case 0x18:         //��⠭����� ०��� ࠡ���
                    if (packets[shift_fifo-REGIM_JOB]&0x01) //����殮��� �� ���稪��
                        P4OUT&=~BIT6;
                    else
                        P4OUT|=BIT6;

                    if (packets[shift_fifo-REGIM_JOB]&0x02) //����殮��� �� ���������� ���
                    {
                        P1OUT&=~BIT0;

                        analog_on=1;
                        P2OUT|=BIT0+BIT1+BIT3;   //����砥� ���

                        P5SEL |= 0x70;           // MCLK, SMCLK,ACLK �� �뢮�


                    }

                    else{
                        P5SEL&= ~0x70;           // MCLK, SMCLK,ACLK �몫�砥� �� �뢮�

                        P2OUT&=~(BIT0+BIT1+BIT3);

                        P3OUT&=~(BIT0+BIT1+BIT2+BIT3);
                        analog_on=0;

                        P1OUT|=BIT0;
                    }

                    if ((stop_adc==0) && packets[shift_fifo-REGIM_JOB]&0x04){ //�몫���� ���
                        disable_int_no_interrupt();
                        chanel_convert&=~0x40; //�몫�砥� ������ ०��
                        off_adc();
                        stop_adc=1;
                        enable_int_no_interrupt();
                    }

                    if ((stop_adc==1) && packets[shift_fifo-REGIM_JOB]&0x08){ //������� ���
                        disable_int_no_interrupt();
                        init_adc();
                        stop_adc=0;
                        enable_int_no_interrupt();
                    }
                    if (packets[shift_fifo-REGIM_JOB]&0x10){ //�몫���� UART
                        disable_int_no_interrupt();
                        P3SEL &= ~BIT4;                        // �몫�砥� � ���� ��।��� USART0
                        stop_transmit=1;
                        //  P3DIR = 0xFF;                         // All P3.x outputs
                        //  P3OUT = 0;                            // All P3.x reset
                        enable_int_no_interrupt();
                    }
                    if (packets[shift_fifo-REGIM_JOB]&0x20){ //������� UART
                        disable_int_no_interrupt();
                        P3SEL |= BIT4;                        // ����砥� � ���� ��।��� USART0
                        stop_transmit=0;
                        //  P3DIR = 0xFF;                         // All P3.x outputs
                        //  P3OUT = 0;                            // All P3.x reset
                        enable_int_no_interrupt();
                    }
                    if ( (temperature==1) && packets[shift_fifo-REGIM_JOB]&0x40){ //�몫���� ⥬�������
                        disable_int_no_interrupt();
                        temperature=0;
                        if (stop_adc==0) { //�᫨ ��� �� ��⠭�����
                            set_adc(chanel_convert,1);
                            ADC12CTL0 |= ADC12SC;                 // Start conversion
                        }
                        enable_int_no_interrupt();
                    }
                    if ((stop_adc==0) && (temperature==0) && packets[shift_fifo-REGIM_JOB]&0x80){ //������� ⥬�������
                        disable_int_no_interrupt();
                        temperature=1;
                        send_correction_temperature=10000;
                        set_adc_temperature();
                        ADC12CTL0 |= ADC12SC;                 // Start conversion
                        enable_int_no_interrupt();
                    }


                    break;
                case 0x1B:
                    address=packets[shift_fifo-ADDRESS_PACK1B]+(packets[shift_fifo-ADDRESS_PACK1B+1]<<8);
                    memcpy(&buffer[0],&packets[shift_fifo-ADDRESS_PACK1B_DATA],SIZE_N_LOADER);

                    break;
                }
                //��ࠢ��� ���⢥ত����, �᫨ �� �� 0x01 ⨯ �����
                if (packets[shift_fifo-TYPEPACKET]!=0x01){
                    shift_fifo=(x+1)*MAXPACKETLEN;
                    //����頥� ���浪��� ����� ����� ��� ���⢥ত����
                    packets[shift_fifo-NUMPACKET]=packets[x*MAXPACKETLEN+queue[x].len-NUMPACKET];
                    packets[shift_fifo-LENPACKET]=NUMPACKET;   //����� ���⢥ত��饣� �����
                    //㪠�뢠�� ⨯ �����
                    packets[shift_fifo-TYPEPACKET]=0x01;               //���⢥ত��騩 �����
                    //������뢠�� � ����頥� CRC �����
                    crc=crc16(&packets[shift_fifo-NUMPACKET],NUMPACKET-2);
                    packets[shift_fifo-CRCPACKET]=crc>>8;
                    packets[shift_fifo-CRCPACKET+1]=crc;

                    //� �ࠢ�筮� ���ᨢ� 㪠�뢠�� ����� �����
                    queue[x].len=NUMPACKET;
     #ifdef DEBUG_SERIAL
                    packet_in_fifo++;
                    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
     #endif //DEBUG_SERIAL
                    queue[x].busy=NOTSENDED;
                }
                else{ // �� ����� 0x01 ⨯�, ���⢥ত��騩
                    queue[x].busy=FREEPLACE;
                }
            }
            else{ // crc �� ᮢ����
                queue[x].busy=FREEPLACE;
                error_packets_crc++;
            }
        }
    }
}


//u8 test[4]={'0','1','2','3'};
extern u16 last_sended_packet;


//���樠������ serial port 0
void init_uart0(void){

}
//���樠������ serial port 1
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
    IE2 |= URXIE1;                        // Enable USART1 RX+TX interrupt
    //+ UTXIE1;
    for (x=0;x<MAXQUE;x++) queue[x].busy=FREEPLACE;
    last_sended_packet=MAXQUE;
 #ifdef DEBUG_SERIAL
    fifo_trn_depth_max=0;
    fifo_trn_depth=0;
    packet_in_fifo=0;
    packet_fifo_full=0;
    error_packets_crc=0;
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
    IE1 |= URXIE0;                        // Enable USART1 RX+TX interrupt
    //+ UTXIE0;
    for (x=0;x<MAXQUE;x++) queue[x].busy=FREEPLACE;
    last_sended_packet=MAXQUE;
    current_rec_packet=0;
    stop_transmit=0;
    rec_length=0;
    queue[0].busy=PACKBUSY;
 #ifdef DEBUG_SERIAL
    fifo_trn_depth_max=0;
    fifo_trn_depth=0;
    packet_in_fifo=0;
    packet_fifo_full=0;
    error_packets_crc=0;
 #endif
    received_packed=0;
#endif //STEND

    dac[0]=0x7FF;
}



