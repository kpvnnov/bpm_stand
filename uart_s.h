
 //������⢮ ����⮢
#define  MAXQUE 8		
 //���ᨬ��쭠� ����� ������ ����� - ������ ���� �⭮�!
#define  MAXPACKETLEN	76	


//������⢮ ������� � ���⨯����஢����� ०���
#define  NUM_MULTICHANNEL 8

//����⨪�
#define STAT_FIFO_RCV_LEN	8           /* size of fifo STAT buffer   */
#define STAT1_FIFO_RCV_LEN	8           /* size of fifo STAT buffer   */
#define SIZE_STAT	9
#define SIZE_STAT1	10



// ����� ����⮢ ࠧ����� ⨯��

#define  DATA0PACKET	5
#define  DATA1PACKET	5
#define  DATA2PACKET	5

#define  DATA3PACKET    24	//ᬥ饭�� (� ����) ��������� � ����� ࠧ��饭�� ������

#define  DATA4PACKET	5

#define  DATA5PACKET     (SIZE_STAT*2+6)
#define  DATA6PACKET     (SIZE_STAT1*2+6)
#define  DATA7PACKET     (32+6)

#define  DATA8PACKET	5

//#define  DATAxAPACKET    (2+4*NUM_MULTICHANNEL+2+6)
//�ࠫ ⥬������� �� �����

#define  DATAxAPACKET    (2+4*NUM_MULTICHANNEL+6)
#define  DATA19PACKET    (2+6)
#define  DATA1APACKET	 (6*2+6)
#if DATAxAPACKET>=MAXPACKETLEN
 error size of fifo packets MAXPACKETLEN
#endif
