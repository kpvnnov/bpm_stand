//$Id: uart_l.h,v 1.4 2004-05-12 14:47:15 peter Exp $
// заголовочный файл для работы с последовательным портом loader
 //количество пакетов
#define  MAXQUE 8
 //максимальная длина одного пакета - ДОЛЖНА БЫТЬ четной!
#define  MAXPACKETLEN   76

#define  SERIAL_FIFO_TRN_LEN  128           /* size of transmit fifo serial buffer   */


//количество каналов в мультиплексированном режиме
#define  NUM_MULTICHANNEL 8
