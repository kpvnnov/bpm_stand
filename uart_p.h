// $Id: uart_p.h,v 1.2 2004-03-17 11:34:16 peter Exp $

struct que{
 u8 busy;    //место занято. 
                //0 - свободно
                //1 - занято
                //2 не передано (необходимо подтверждение)
                //3 передано (ждем подтверждения)
                //4 не передано (подтверждения не потребуется)
 u8 numeric; //порядковый номер пакета
 u8 len;
};


u16 hold_packet(void);


#define FREEPLACE 0     //свободный пакет
#define PACKBUSY  1     //занятый под обработку
#define NOTSENDED 2     //неотправленный пакет
#define WAIT_ACK  3     //ожидающий подтверждения
#define PACKREC   4     //принятый пакет


#define  CRCPACKET      2       //смещение (с конца) положения в пакете CRC
#define  LENPACKET      3       //смещение (с конца) положения в пакете длины пакета
#define  TYPEPACKET     4       //смещение (с конца) положения в пакете типа пакета
#define  NUMPACKET      5       //смещение (с конца) положения в пакете номера пакета


#define  ESCAPE         0x7D
#define  EOFPACKET      0x7E    //код признака конца кадра



// длина пакетов различных типов

#define  DATA0PACKET    5
#define  DATA1PACKET    5
#define  DATA2PACKET    5

#define  DATA3PACKET    24      //смещение (с конца) положения в пакете размещения данных

#define  DATA4PACKET    5

#define  DATA5PACKET     (SIZE_STAT*2+6)
#define  DATA6PACKET     (SIZE_STAT1*2+6)
#define  DATA7PACKET     (32+6)

#define  DATA8PACKET    5

//#define  DATAxAPACKET    (2+4*NUM_MULTICHANNEL+2+6)
//убрал температуру из пакета

#define  DATAxAPACKET    (2+4*NUM_MULTICHANNEL+6)
#define  DATA19PACKET    (2+6)
#define  DATA1APACKET    (6*2+6)

#define ADDRESS_PACK1B 20

#ifdef STEND
 #if DATAxAPACKET>=MAXPACKETLEN
  error size of fifo packets MAXPACKETLEN
 #endif
#endif


//статистика
#define STAT_FIFO_RCV_LEN       8           /* size of fifo STAT buffer   */
#define STAT1_FIFO_RCV_LEN      8           /* size of fifo STAT buffer   */
#define SIZE_STAT       9
#define SIZE_STAT1      10
