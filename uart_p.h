// $Id: uart_p.h,v 1.2 2004-03-17 11:34:16 peter Exp $

struct que{
 u8 busy;    //���� �����. 
                //0 - ᢮�����
                //1 - �����
                //2 �� ��।��� (����室��� ���⢥ত����)
                //3 ��।��� (���� ���⢥ত����)
                //4 �� ��।��� (���⢥ত���� �� ���ॡ����)
 u8 numeric; //���浪��� ����� �����
 u8 len;
};


u16 hold_packet(void);


#define FREEPLACE 0     //᢮����� �����
#define PACKBUSY  1     //������ ��� ��ࠡ���
#define NOTSENDED 2     //����ࠢ����� �����
#define WAIT_ACK  3     //������騩 ���⢥ত����
#define PACKREC   4     //�ਭ��� �����


#define  CRCPACKET      2       //ᬥ饭�� (� ����) ��������� � ����� CRC
#define  LENPACKET      3       //ᬥ饭�� (� ����) ��������� � ����� ����� �����
#define  TYPEPACKET     4       //ᬥ饭�� (� ����) ��������� � ����� ⨯� �����
#define  NUMPACKET      5       //ᬥ饭�� (� ����) ��������� � ����� ����� �����


#define  ESCAPE         0x7D
#define  EOFPACKET      0x7E    //��� �ਧ���� ���� ����



// ����� ����⮢ ࠧ����� ⨯��

#define  DATA0PACKET    5
#define  DATA1PACKET    5
#define  DATA2PACKET    5

#define  DATA3PACKET    24      //ᬥ饭�� (� ����) ��������� � ����� ࠧ��饭�� ������

#define  DATA4PACKET    5

#define  DATA5PACKET     (SIZE_STAT*2+6)
#define  DATA6PACKET     (SIZE_STAT1*2+6)
#define  DATA7PACKET     (32+6)

#define  DATA8PACKET    5

//#define  DATAxAPACKET    (2+4*NUM_MULTICHANNEL+2+6)
//�ࠫ ⥬������� �� �����

#define  DATAxAPACKET    (2+4*NUM_MULTICHANNEL+6)
#define  DATA19PACKET    (2+6)
#define  DATA1APACKET    (6*2+6)

#define ADDRESS_PACK1B 20

#ifdef STEND
 #if DATAxAPACKET>=MAXPACKETLEN
  error size of fifo packets MAXPACKETLEN
 #endif
#endif


//����⨪�
#define STAT_FIFO_RCV_LEN       8           /* size of fifo STAT buffer   */
#define STAT1_FIFO_RCV_LEN      8           /* size of fifo STAT buffer   */
#define SIZE_STAT       9
#define SIZE_STAT1      10
