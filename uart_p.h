// $Id: uart_p.h,v 1.1 2004-03-07 21:31:50 peter Exp $

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

#define FREEPLACE 0	//᢮����� �����
#define PACKBUSY  1	//������ ��� ��ࠡ���
#define NOTSENDED 2	//����ࠢ����� �����
#define WAIT_ACK  3     //������騩 ���⢥ত����
#define PACKREC   4	//�ਭ��� �����


#define  CRCPACKET	2	//ᬥ饭�� (� ����) ��������� � ����� CRC
#define  LENPACKET	3	//ᬥ饭�� (� ����) ��������� � ����� ����� �����
#define  TYPEPACKET     4	//ᬥ饭�� (� ����) ��������� � ����� ⨯� �����
#define  NUMPACKET	5	//ᬥ饭�� (� ����) ��������� � ����� ����� �����
