// $Id: uart_p.h,v 1.3 2004-03-18 16:51:15 peter Exp $

struct que{
 u8 busy;    //����� ������. 
                //0 - ��������
                //1 - ������
                //2 �� �������� (���������� �������������)
                //3 �������� (���� �������������)
                //4 �� �������� (������������� �� �����������)
 u8 numeric; //���������� ����� ������
 u8 len;
};


u16 hold_packet(void);
u16 make_packet(void);
void fill_date_packet(u8 type_packet, u16 num_packet);


#define FREEPLACE 0     //��������� �����
#define PACKBUSY  1     //������� ��� ���������
#define NOTSENDED 2     //�������������� �����
#define WAIT_ACK  3     //��������� �������������
#define PACKREC   4     //�������� �����


#define  CRCPACKET      2       //�������� (� �����) ��������� � ������ CRC
#define  LENPACKET      3       //�������� (� �����) ��������� � ������ ����� ������
#define  TYPEPACKET     4       //�������� (� �����) ��������� � ������ ���� ������
#define  NUMPACKET      5       //�������� (� �����) ��������� � ������ ������ ������


#define  ESCAPE         0x7D
#define  EOFPACKET      0x7E    //��� �������� ����� �����



// ����� ������� ��������� �����

#define  SIZE_N_LOADER  64

#define  DATA0PACKET    5
#define  DATA1PACKET    5
#define  DATA2PACKET    5

#define  DATA3PACKET    24      //�������� (� �����) ��������� � ������ ���������� ������

#define  DATA4PACKET    5

#define  DATA5PACKET     (SIZE_STAT*2+6)
#define  DATA6PACKET     (SIZE_STAT1*2+6)
#define  DATA7PACKET     (32+6)

#define  DATA8PACKET    5
#define  DATA9PACKET    5





#define  DATAxAPACKET    (2+4*NUM_MULTICHANNEL+6)

#define  DATAxBPACKET   5
#define  DATAxCPACKET   5
#define  DATAxDPACKET   5
#define  DATAxEPACKET   5
#define  DATAxFPACKET   5
#define  DATA10PACKET   5
#define  DATA11PACKET   5
#define  DATA12PACKET   5
#define  DATA13PACKET   5
#define  DATA14PACKET   5
#define  DATA15PACKET   5
#define  DATA16PACKET   5
#define  DATA17PACKET   5
#define  DATA18PACKET   5

#define  DATA19PACKET    (2+6)
#define  DATA1APACKET    (6*2+6)

#define  DATA1BPACKET    (2+SIZE_N_LOADER+6)
#define  ADDRESS_PACK1B  (2+SIZE_N_LOADER+6)
#define  ADDRESS_PACK1B_DATA (SIZE_N_LOADER+6)


#define  DATA1CPACKET   (2+6)
#define  ADDRESS_PACK1C (2+6)


#define  DATA1DPACKET   5
#define  DATA1EPACKET   5
#define  DATA1FPACKET   5
#define  DATA20PACKET   5
#define  DATA21PACKET   5
#define  DATA22PACKET   5
#define  DATA23PACKET   5
#define  DATA24PACKET   5

#define  DATA25PACKET   (2+SIZE_N_LOADER+6)
#define  ADDRESS_PACK25      (2+SIZE_N_LOADER+6)
#define  ADDRESS_PACK25_DATA (SIZE_N_LOADER+6)

#define  DATA26PACKET   5

#ifdef STEND
 #if DATAxAPACKET>=MAXPACKETLEN
  error size of fifo packets MAXPACKETLEN
 #endif
#endif


//����������
#define STAT_FIFO_RCV_LEN       8           /* size of fifo STAT buffer   */
#define STAT1_FIFO_RCV_LEN      8           /* size of fifo STAT buffer   */
#define SIZE_STAT       9
#define SIZE_STAT1      10
