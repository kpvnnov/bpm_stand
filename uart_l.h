//$Id: uart_l.h,v 1.3 2004-03-18 16:51:15 peter Exp $
// заголовочный файл для работы с последовательным портом loader
 //количество пакетов
#define  MAXQUE 4
 //максимальная длина одного пакета - ДОЛЖНА БЫТЬ четной!
#define  MAXPACKETLEN   76

#define  SERIAL_FIFO_TRN_LEN  128           /* size of transmit fifo serial buffer   */


//количество каналов в мультиплексированном режиме
#define  NUM_MULTICHANNEL 8
