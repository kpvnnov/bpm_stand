// $Id: uart_s.c,v 1.18 2004-06-07 12:32:14 peter Exp $
#include  <msp430x14x.h>
#include  <string.h>

//типы переменных
#include "type_def.h"


#include "global.h"
#include "uart_s.h"

// "inline" функции для 149 процессора
#include "m149.h"

// модуль для работы с пакетами
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
обмен на скорости 115200

Количество Значение
  байт
_____

X      данные пакета
 служебные данные пакета:
1      порядковый номер пакета
       ( если тип пакета 0x01 - то это номер подтверждаемого пакета)
1      тип пакета
1      длина пакета (без маркера конца пакета = X+5)
2      CRC16 (старший байт идет первым, потом младший)
       CRC16 - считается массив данных с начала пакета данных
       до поля идущим перед полем CRC16 (длина пакета) включительно
       после этого полученная CRC заносится в следующем порядке:
       старший байт, младший байт
1      x7E - маркер пакета


 В целях гибкости (расширения) длины пакета данных любого типа
программную реализацию разбора содержимого пакета (данных пакета)
желательно делать от "головы" пакета, 
а служебных заголовков - от хвоста пакета (маркера конца)

 при передаче по каналу весь пакет (данные пакета+служебные с
уже подсчитанными полями и CRC, за исключением маркера конца 
пакета 0x7E) анализируется на наличие байт с кодами
 0x7D (ESCAPE)
 0x7E (EOF)
такие байты заменяются на последовательность из двух байт
0x7D,исходные данные XOR 0x40
то есть
 было          стало
 0x7D          0x7D 0x3D
 0x7E          0x7D 0x3E

на приемной стороне:
 поток анализируется на поступление байта
0x7D (ESCAPE).  При получении данного значения сам код этого 
байта в анализе не учавствует, а над значение следующего байта
необходимо провести операцию XOR 0x40. Полученные данные
после данной операции не являются служебными кодами (EOF или
ESCAPE), а являются обыкновенным содержимым пакета.
 при получении кода 0x7E (EOF) необходимо подсчитать CRC
всех принятых данных перед этим кодом (исключая сам символ EOF)
при результирующем значении CRC = 0 контрольная сумма считается 
совпавшей. Дополнительным контролем целостности пакета
может служить совпадение количества реально принятых данных
со значением указанным в служебном поле пакета "длина".
Тип
пакета

0x01      пакет подтверждения.
          подтверждается прием пакета с указанным порядковым номером
_________________________________________________________________
0x02      пакет синхронизации времени
          В пакете передается Фаза на момент передачи по 
          последовательному порту
          пакет не требует подтверждения и идет без порядкового номера
_________________________________________________________________
0x03      пакет данных ЭКГ
_________________________________________________________________
0x05      пакет данных статистики
_________________________________________________________________
0x06      пакет данных статистики 1
_________________________________________________________________
0x07      пакет данных с двух каналов
          содержание пакета:
          (0) слово - значение опорного канала 
          (1) слово - значение опорного канала (следующее по времени)
          .....
          (7) слово - значение опорного канала (следующее по времени)
          (8) слово - значение исследуемого канала
          (9) слово - значение исследуемого канала (следующее по времени)
          .....
          (15) слово - значение исследуемого канала (следующее по времени)
          1 байт - зарезервировано
          общее количество данных 16*2+1=33 байта
_________________________________________________________________
0x08      пакет данных установки канала ЦАП
        содержание пакета:
         два байта - значение для ЦАП
         байт      - номер канала ЦАП
        общее количество данных 2+1=3 байта
_________________________________________________________________
0x09      пакет данных установки канала АЦП
       содержание пакета:
    байт      - номер канала для преобразования
                в котором - SXMM MAAM
                S - 0 - основной, 1 - резервный
                X -  0 - режим вывода всех отсчетов опорного и 
                         исследуемого канала (пакет 0x07)
                     1 - режим вывода суммарных значений
                A - значение канала АЦП (PRESS_1 - PRESS_4)
                M - значение для MUX (A0-A3)
_________________________________________________________________
0x0A      пакет данных с каналов (просуммированные)
       содержание пакета:
    байт - количество выводимых каналов (N)
    байт - номер первого канала
    слово - значение опорного канала со смещением 0
    слово - значение исследуемого канала со смещением 0
    слово - значение опорного канала со смещением 1
    слово - значение исследуемого канала со смещением 1
    .....
    слово - значение опорного канала со смещением (N-1)
    слово - значение исследуемого канала со смещением (N-1)
    слово - значение температуры (суммированные 16 отсчетов)
    1 байт - зарезервировано
    общее количество данных 2+N*2+1 байта
_________________________________________________________________
0x0B      закрыть клапан
     содержание пакета:
      нет
_________________________________________________________________
0x0С      открыть клапан
     содержание пакета:
      нет
_________________________________________________________________
0x0D      спустить давление до требуемой величины
     содержание пакета:
          в пакете слово (+1 байт reserved) - абсолютное 
          значение до которого  необходимо опуститься и закрыть клапан
          12 бит текущего опорного (основного или резервного)
          канала 
    общее количество данных 3 байта
_________________________________________________________________
0x0E      накачать давление до указанной величины
     содержание пакета:
          в пакете слово (+1 байт reserved) - абсолютное 
          значение до которого необходимо накачать давление
          предварительно необходимо дать комманду
          закрыть клапан
    общее количество данных 3 байта
_________________________________________________________________
0x0F      выключить компрессор. Данная команда прекращает
          действие пакета 0x0E
     содержание пакета:
      нет
_________________________________________________________________
0x10      установка интервала таймаута (сек) в течении
          которого пытаться накачать давление до требуемого
     содержание пакета:
          в пакете 1 байт, по умолчанию 30.
          По истечении этого времени высылается служебный пакет
          0x11
    общее количество данных 1 байт
_________________________________________________________________
0x11      служебный пакет сигнализирующий о истечении
          таймаута накачки
     содержание пакета:
      нет
_________________________________________________________________
0x12      установка интервала таймаута (сек) в течении
          которого пытаться опустить давление до требуемого
          (по умолчанию 5)
     содержание пакета:
          в пакете 1 байт
    общее количество данных 1 байт
          По истечении этого времени высылается служебный пакет
          0x14
_________________________________________________________________
0x14      служебный пакет сигнализирующий о истечении
          таймаута спуска
     содержание пакета:
      нет
_________________________________________________________________
0x15      установка давления при достижении которого
          автоматически выполняется 0x0F 0x0С
          и высылается служебный пакет 0x16
     содержание пакета:
          в пакете слово (+1 байт reserved) - абсолютное 
          предельное значение давления
    общее количество данных 3 байта
_________________________________________________________________
0x16      служебный пакет сигнализирующий о превышении
          заданного уровня давления (аварийный уровень)
     содержание пакета:
      нет
_________________________________________________________________
0x18      Установить биты управления режимом
     содержание пакета:
          в пакете 1 байт 
            0 бит - включение напряжения на датчиках
            1 бит - включение напряжения на аналоговой части
            2 бит - выключение преобразования АЦП
            3 бит - включение преобразования АЦП
            4 бит - выключение UART
            5 бит - включение UART
            6 бит - выключение температуры
            7 бит - включение температуры

    общее количество данных 1 байт
_________________________________________________________________
0x19      Данные температуры
     содержание пакета:
      2 байта - измеренное значение
_________________________________________________________________
0x1A      Данные коррекции температуры (раз в секунду)
     содержание пакета:
      2 байта - наклон
      2 байта - смещение
      2 байта - значение АЦП (первая точка)
      2 байта - значение температуры умноженное на 100 (первая точка)
      2 байта - значение АЦП (вторая точка)
      2 байта - значение температуры умноженное на 100 (вторая точка)

*/


#define  CRCPACKET      2       //смещение (с конца) положения в пакете CRC
#define  LENPACKET      3       //смещение (с конца) положения в пакете длины пакета
#define  TYPEPACKET     4       //смещение (с конца) положения в пакете типа пакета
#define  NUMPACKET      5       //смещение (с конца) положения в пакете номера пакета
#define  CHANNEL_TO_SET 6       //значение номера канала АЦП в 9-ом пакете
#define  NUM_OF_CHANNEL_DAC 6   //значение номера канала ЦАП в 8-ом пакете
#define  VALVE_OF_CHANNEL_DAC 8 //значение для ЦАП в 8-ом пакете
#define  LEVEL_SET      8       //значение данных в 3-х байтовом пакете
#define  REGIM_JOB      6       //установка режимов работы

extern volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

extern unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
extern u16   asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */

//unsigned int  asp_rcv_fifo_start;      /* serial receive buffer start index      */

//volatile unsigned int  asp_rcv_fifo_end;        /* serial receive flash buffer end index        */

//u8    asp_rcv_fifo_buf[SERIAL_FIFO_RCV_LEN];           /* storage for receive transmit  buffer      */

#define FREEPLACE 0     //свободный пакет
#define PACKBUSY  1     //занятый под обработку
#define NOTSENDED 2     //неотправленный пакет
#define WAIT_ACK  3     //ожидающий подтверждения
#define PACKREC   4     //принятый пакет


extern struct que queue[MAXQUE];
extern u8 packets[MAXQUE*MAXPACKETLEN];

extern u8 counts_packet; //порядковый номер пакета




//отправляем пакет со статистикой
u16 put_packet_type5(void){
    int n;
    //int x;
    int crc;
    int shift_fifo;
    //u16* t_p;
    //u16* t_s;
    //захватываем свободный пакет
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //свободных пакетов нет
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //копируем туда данные для пакета
    // t_p=(u16*)&packets[shift_fifo-DATA5PACKET];
    // t_s=(u16*)&stat_buf[(stat_rcv_fifo_start & (STAT_FIFO_RCV_LEN-1))*SIZE_STAT];

    // for(x=0;x<SIZE_STAT;x++)
    //  *t_p++=*t_s++;
    memcpy(&packets[shift_fifo-DATA5PACKET],&stat_buf[(stat_rcv_fifo_start & (STAT_FIFO_RCV_LEN-1))*SIZE_STAT],SIZE_STAT*2);


    stat_rcv_fifo_start++;


    //помещаем в пакет его длину (без завершающего EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA5PACKET;
    //помещаем (и увеличиваем) порядковый номер пакета
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //указываем тип пакета
    packets[shift_fifo-TYPEPACKET]=0x05;
    //подсчитываем и помещаем CRC пакета
    crc=crc16(&packets[shift_fifo-DATA5PACKET],DATA5PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //в справочном массиве указываем длину пакета
    queue[n].len=DATA5PACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}
u16 put_packet_type6(void){
    int n;
    //int x;
    int crc;
    int shift_fifo;
    //захватываем свободный пакет
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //свободных пакетов нет
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //копируем туда данные для пакета
    memcpy(&packets[shift_fifo-DATA6PACKET],&stat1_buf[(stat1_rcv_fifo_start & (STAT1_FIFO_RCV_LEN-1))*SIZE_STAT1],SIZE_STAT1*2);


    stat1_rcv_fifo_start++;


    //помещаем в пакет его длину (без завершающего EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA6PACKET;
    //помещаем (и увеличиваем) порядковый номер пакета
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //указываем тип пакета
    packets[shift_fifo-TYPEPACKET]=0x06;
    //подсчитываем и помещаем CRC пакета
    crc=crc16(&packets[shift_fifo-DATA6PACKET],DATA6PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //в справочном массиве указываем длину пакета
    queue[n].len=DATA6PACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}
u8 put_packet_type4(void){
    int n;
    int crc;
    int shift_fifo;
    //захватываем свободный пакет
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //свободных пакетов нет
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
    //захватываем свободный пакет
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //свободных пакетов нет
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //копируем туда данные для пакета
    t_p=(u16*)&packets[shift_fifo-DATA7PACKET];

    memcpy(t_p,&one_count0[info*SIZE_OF_ADC_DUMP],SIZE_OF_ADC_DUMP*2);
    memcpy(t_p+SIZE_OF_ADC_DUMP,&one_count1[info*SIZE_OF_ADC_DUMP],SIZE_OF_ADC_DUMP*2);
    packets[shift_fifo-DATA7PACKET+4*SIZE_OF_ADC_DUMP]=results[info];

    //помещаем в пакет его длину (без завершающего EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA7PACKET;
    //помещаем (и увеличиваем) порядковый номер пакета
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //указываем тип пакета
    packets[shift_fifo-TYPEPACKET]=0x07;
    //подсчитываем и помещаем CRC пакета
    crc=crc16(&packets[shift_fifo-DATA7PACKET],DATA7PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //в справочном массиве указываем длину пакета
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
    //захватываем свободный пакет
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //свободных пакетов нет
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //копируем туда данные для пакета
    t_p=(u16*)&packets[shift_fifo-DATAxAPACKET];

    *t_p++=((results[info]&0xFF)<<8)|NUM_MULTICHANNEL;//количество выводимых каналов
    for (z=0;z<NUM_MULTICHANNEL;z++){
        *t_p++=multi_count0[z][info];
        *t_p++=multi_count1[z][info];
    }
    // *t_p++=multi_count0[NUM_MULTICHANNEL][info]+multi_count1[NUM_MULTICHANNEL][info]; //температура

    //помещаем в пакет его длину (без завершающего EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATAxAPACKET;
    //помещаем (и увеличиваем) порядковый номер пакета
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //указываем тип пакета
    packets[shift_fifo-TYPEPACKET]=0x0A;
    //подсчитываем и помещаем CRC пакета
    crc=crc16(&packets[shift_fifo-DATAxAPACKET],DATAxAPACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //в справочном массиве указываем длину пакета
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
    //захватываем свободный пакет
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //свободных пакетов нет
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL
    shift_fifo=(n+1)*MAXPACKETLEN;
    //копируем туда данные для пакета
    t_p=(u16*)&packets[shift_fifo-DATA19PACKET];

    *t_p++=multi_count0[0][info];//температура

    //помещаем в пакет его длину (без завершающего EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA19PACKET;
    //помещаем (и увеличиваем) порядковый номер пакета
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //указываем тип пакета
    packets[shift_fifo-TYPEPACKET]=0x19;
    //подсчитываем и помещаем CRC пакета
    crc=crc16(&packets[shift_fifo-DATA19PACKET],DATA19PACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //в справочном массиве указываем длину пакета
    queue[n].len=DATA19PACKET;
    queue[n].busy=NOTSENDED;
    return 1;
}

u8 put_packet_type1A(void){
    u16* t_p;
    int n;
    int crc;
    int shift_fifo;
    //захватываем свободный пакет
    disable_int_no_interrupt();
    n=hold_packet();
    enable_int_no_interrupt();
    if (n==MAXQUE) {
  #ifdef DEBUG_SERIAL
        packet_fifo_full++;
  #endif //DEBUG_SERIAL
        return 0; //свободных пакетов нет
    }
 #ifdef DEBUG_SERIAL
    packet_in_fifo++;
    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
 #endif //DEBUG_SERIAL

    shift_fifo=(n+1)*MAXPACKETLEN;
    //копируем туда данные для пакета
    t_p=(u16*)&packets[shift_fifo-DATA1APACKET];

#if   STEND_N == 1
    *t_p++=41294;  //наклон
    *t_p++=42148;  //смещение
    *t_p++=43344;  //значение АЦП (первая точка)
    *t_p++=2266;   //значение температуры умноженное на 100 (первая точка)
    *t_p++=43680;  //значение АЦП (вторая точка)
    *t_p++=4460;   //значение температуры умноженное на 100 (вторая точка)
    *t_p++=0x04A4; //номер стенда A4040003
    *t_p++=0x0100; // A - стенд, 04 - год, 0003 - номер
    warning!!!

#elif STEND_N == 2
    *t_p++=41294;  //наклон
    *t_p++=42148;  //смещение
    *t_p++=43344;  //значение АЦП (первая точка)
    *t_p++=2266;   //значение температуры умноженное на 100 (первая точка)
    *t_p++=43680;  //значение АЦП (вторая точка)
    *t_p++=4460;   //значение температуры умноженное на 100 (вторая точка)
    *t_p++=0x04A4; //номер стенда A4040003
    *t_p++=0x0200; // A - стенд, 04 - год, 0003 - номер
#elif STEND_N == 3
    *t_p++=0;  //наклон
    *t_p++=0;  //смещение
    *t_p++=0;  //значение АЦП (первая точка)
    *t_p++=0;   //значение температуры умноженное на 100 (первая точка)
    *t_p++=0;  //значение АЦП (вторая точка)
    *t_p++=0;   //значение температуры умноженное на 100 (вторая точка)
    *t_p++=0x04A4; //номер стенда A4040003
    *t_p++=0x0300; // A - стенд, 04 - год, 0003 - номер
#else
    warning!!!

#endif 

    //помещаем в пакет его длину (без завершающего EOFPACKET)
    packets[shift_fifo-LENPACKET]=DATA1APACKET;
    //помещаем (и увеличиваем) порядковый номер пакета
    packets[shift_fifo-NUMPACKET]=counts_packet;
    queue[n].numeric=counts_packet++;
    //указываем тип пакета
    packets[shift_fifo-TYPEPACKET]=0x1A;
    //подсчитываем и помещаем CRC пакета
    crc=crc16(&packets[shift_fifo-DATA1APACKET],DATA1APACKET-2);
    packets[shift_fifo-CRCPACKET]=crc>>8;
    packets[shift_fifo-CRCPACKET+1]=crc;

    //в справочном массиве указываем длину пакета
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
            //подсчитать CRC
            shift_fifo=x*MAXPACKETLEN+queue[x].len;
            if (crc16(&packets[x*MAXPACKETLEN],queue[x].len)==0){ //сrc совпала?
                switch(packets[shift_fifo-TYPEPACKET]){
                case 0x08:
                    dac[packets[shift_fifo-NUM_OF_CHANNEL_DAC]&((NUM_CHANEL>>1)-1)]=packets[shift_fifo-VALVE_OF_CHANNEL_DAC]+(packets[shift_fifo-VALVE_OF_CHANNEL_DAC+1]<<8);
                    break;
                case 0x09:
                    chanel=packets[shift_fifo-CHANNEL_TO_SET];
                    break;
                case 0x0B:         //закрыть клапан
                    close_valve();
                    break;
                case 0x0C:         //открыть клапан
                    open_valve();
                    break;
                case 0x0D:         //спустить давление до требуемой величины
                    //     содержание пакета:
                    //  в пакете слово (+1 байт reserved) - абсолютное
                    //  значение до которого  необходимо опуститься и закрыть клапан
                    //  12 бит текущего опорного (основного или резервного)
                    //  канала
                    // общее количество данных 3 байта
                    what_doing=NO_JOB;
                    to_level=packets[shift_fifo-LEVEL_SET]+(packets[shift_fifo-LEVEL_SET+1]<<8);
                    to_level&=0xFFF;
                    open_valve();
                    off_pump();
                    what_doing=LEVEL_DOWN;
                    break;
                case 0x0E:         //накачать давление до указанной величины
                    //     содержание пакета:
                    //  в пакете слово (+1 байт reserved) - абсолютное
                    //  значение до которого необходимо накачать давление
                    //  предварительно необходимо дать комманду
                    //  закрыть клапан
                    // общее количество данных 3 байта
                    what_doing=NO_JOB;
                    to_level=packets[shift_fifo-LEVEL_SET]+(packets[shift_fifo-LEVEL_SET+1]<<8);
                    to_level&=0xFFF;
                    close_valve();
                    on_pump();
                    what_doing=LEVEL_UP;
                    break;
                case 0x0F:         //выключить компрессор. Данная команда прекращает
                    break;
                case 0x15:         //установка давления при достижении которого
                    break;
                case 0x18:         //установить режимы работы
                    if (packets[shift_fifo-REGIM_JOB]&0x01) //напряжения на датчиках
                        P4OUT&=~BIT6;
                    else
                        P4OUT|=BIT6;

                    if (packets[shift_fifo-REGIM_JOB]&0x02) //напряжения на аналоговой части
                    {
                        P1OUT&=~BIT0;

                        analog_on=1;
                        P2OUT|=BIT0+BIT1+BIT3;   //включаем ЦАП

                        P5SEL |= 0x70;           // MCLK, SMCLK,ACLK на вывод


                    }

                    else{
                        P5SEL&= ~0x70;           // MCLK, SMCLK,ACLK выключаем на вывод

                        P2OUT&=~(BIT0+BIT1+BIT3);

                        P3OUT&=~(BIT0+BIT1+BIT2+BIT3);
                        analog_on=0;

                        P1OUT|=BIT0;
                    }

                    if ((stop_adc==0) && packets[shift_fifo-REGIM_JOB]&0x04){ //выключить АЦП
                        disable_int_no_interrupt();
                        chanel_convert&=~0x40; //выключаем пакетный режим
                        off_adc();
                        stop_adc=1;
                        enable_int_no_interrupt();
                    }

                    if ((stop_adc==1) && packets[shift_fifo-REGIM_JOB]&0x08){ //включить АЦП
                        disable_int_no_interrupt();
                        init_adc();
                        stop_adc=0;
                        enable_int_no_interrupt();
                    }
                    if (packets[shift_fifo-REGIM_JOB]&0x10){ //выключить UART
                        disable_int_no_interrupt();
                        P3SEL &= ~BIT4;                        // выключаем в ноль передачу USART0
                        stop_transmit=1;
                        //  P3DIR = 0xFF;                         // All P3.x outputs
                        //  P3OUT = 0;                            // All P3.x reset
                        enable_int_no_interrupt();
                    }
                    if (packets[shift_fifo-REGIM_JOB]&0x20){ //включить UART
                        disable_int_no_interrupt();
                        P3SEL |= BIT4;                        // включаем в ноль передачу USART0
                        stop_transmit=0;
                        //  P3DIR = 0xFF;                         // All P3.x outputs
                        //  P3OUT = 0;                            // All P3.x reset
                        enable_int_no_interrupt();
                    }
                    if ( (temperature==1) && packets[shift_fifo-REGIM_JOB]&0x40){ //выключить температуру
                        disable_int_no_interrupt();
                        temperature=0;
                        if (stop_adc==0) { //если АЦП не остановлен
                            set_adc(chanel_convert,1);
                            ADC12CTL0 |= ADC12SC;                 // Start conversion
                        }
                        enable_int_no_interrupt();
                    }
                    if ((stop_adc==0) && (temperature==0) && packets[shift_fifo-REGIM_JOB]&0x80){ //включить температуру
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
                //отправить подтверждение, если это не 0x01 тип пакета
                if (packets[shift_fifo-TYPEPACKET]!=0x01){
                    shift_fifo=(x+1)*MAXPACKETLEN;
                    //помещаем порядковый номер пакета для подтверждения
                    packets[shift_fifo-NUMPACKET]=packets[x*MAXPACKETLEN+queue[x].len-NUMPACKET];
                    packets[shift_fifo-LENPACKET]=NUMPACKET;   //длина подтверждающего пакета
                    //указываем тип пакета
                    packets[shift_fifo-TYPEPACKET]=0x01;               //подтверждающий пакет
                    //подсчитываем и помещаем CRC пакета
                    crc=crc16(&packets[shift_fifo-NUMPACKET],NUMPACKET-2);
                    packets[shift_fifo-CRCPACKET]=crc>>8;
                    packets[shift_fifo-CRCPACKET+1]=crc;

                    //в справочном массиве указываем длину пакета
                    queue[x].len=NUMPACKET;
     #ifdef DEBUG_SERIAL
                    packet_in_fifo++;
                    if (packet_in_fifo_max<packet_in_fifo) packet_in_fifo_max=packet_in_fifo;
     #endif //DEBUG_SERIAL
                    queue[x].busy=NOTSENDED;
                }
                else{ // это пакет 0x01 типа, подтверждающий
                    queue[x].busy=FREEPLACE;
                }
            }
            else{ // crc не совпала
                queue[x].busy=FREEPLACE;
                error_packets_crc++;
            }
        }
    }
}


//u8 test[4]={'0','1','2','3'};
extern u16 last_sended_packet;


//инициализация serial port 0
void init_uart0(void){

}
//инициализация serial port 1
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



