// $Id: uart.c,v 1.2 2003-04-29 11:07:46 peter Exp $
#include  <msp430x14x.h>
#include "global.h"




#define  SERIAL_FIFO_RCV_LEN  64           /* size of fifo serial buffer   */
#define  SERIAL_FIFO_TRN_LEN  32           /* size of fifo serial buffer   */

volatile unsigned int  asp_trn_fifo_start;      /* serial transmit buffer start index      */

unsigned int  asp_trn_fifo_end;        /* serial transmit flash buffer end index        */
u8    asp_trn_fifo_buf[SERIAL_FIFO_TRN_LEN];           /* storage for serial transmit  buffer      */

unsigned int  asp_rcv_fifo_start;      /* serial receive buffer start index      */

volatile unsigned int  asp_rcv_fifo_end;        /* serial receive flash buffer end index        */

u8    asp_rcv_fifo_buf[SERIAL_FIFO_RCV_LEN];           /* storage for receive transmit  buffer      */


void init_uart0(void){
  UCTL0 = CHAR;                         // 8-bit character
  UTCTL0 = SSEL0;                       // UCLK = ACLK
  UBR00 = 0x45;                         // 8MHz 115200
  UBR10 = 0x00;                         // 8MHz 115200
  UMCTL0 = 0x00;                        // 8MHz 115200 modulation
  ME1 |= UTXE0 + URXE0;                 // Enable USART0 TXD/RXD
  IE1 |= URXIE0;                        // Enable USART0 RX interrupt
  P3SEL |= 0x30;                        // P3.4,5 = USART0 TXD/RXD
  P3DIR |= 0x10;                        // P3.4 output direction
}





interrupt[UART0TX_VECTOR] void usart0_tx (void)
{
 if (asp_trn_fifo_start!=asp_trn_fifo_end)
  TXBUF0 =asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
}
interrupt[UART0RX_VECTOR] void usart0_rx (void)
{

//  while ((IFG1 & UTXIFG0) == 0);        // USART0 TX buffer ready?
//  TXBUF0 = ;                      // RXBUF0 to TXBUF0


//⠪ ��� ��� inline, � ���뢠��  write_asp_rcv_fifo(RXBUF0);
//u8 write_asp_rcv_fifo(u8 data_wr){
// if ( ( (asp_rcv_fifo_end+1)&(SERIAL_FIFO_RCV_LEN-1))== (asp_rcv_fifo_start&(SERIAL_FIFO_RCV_LEN-1)) )
//  return 0;
// asp_rcv_fifo_buf[asp_rcv_fifo_end++ & (SERIAL_FIFO_RCV_LEN-1)]=data_wr;
// return 1;
//}


// - �������� � �⫠��筮� ०��� ��� �஢��� if ( ( (asp_rcv_fifo_end+1)&(SERIAL_FIFO_RCV_LEN-1))!= (asp_rcv_fifo_start&(SERIAL_FIFO_RCV_LEN-1)) )
  asp_rcv_fifo_buf[asp_rcv_fifo_end++ & (SERIAL_FIFO_RCV_LEN-1)]=RXBUF0;



// if (temp_iosr_serial&( (1<<bit_OE)| //Overrun occurs - ��९������� �ਥ��
//                 (1<<bit_FE)| //framing error
//                 (1<<bit_BI)   //break
//               )
//    ) { 
//  portADTR;
//  portADTR;	//��� ࠧ� �⠥� ����, �⮡� �᢮�������� �� ������
//  rDIOSR&=0x00FF;
//  rDIOSR|=0x2600;	//��� 䫠��� BI(break interrupt)
//                        //FE(framing error)
//                        //��९�������(OE-overrun error) � IOSR
//  portIOSR=rDIOSR;

}
u16 read_asp_rcv_fifo(void){ 
 if (asp_rcv_fifo_start==asp_rcv_fifo_end) return 0;
 return (asp_rcv_fifo_buf[asp_rcv_fifo_start++ & (SERIAL_FIFO_RCV_LEN-1)]|0x0100);
}
u16 send_serial_massiv(u8* data,u16 len){
u16 counter;
u16 counter1;
u16 t_end;
u16 t_start;
// do {
  t_end=  asp_trn_fifo_end&(SERIAL_FIFO_TRN_LEN-1);
  t_start=asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1);
  if (t_end>=t_start) //��।� �� �� ���諠 �१ �ࠩ � 墮�� ���।�
   {
   counter=SERIAL_FIFO_TRN_LEN-t_end;
   if (t_start==0) counter--;
   }
  else
   counter=t_start-t_end-1;
//  if (counter==0) break;
  if (counter==0) return len;
  if (counter>len) counter=len;
  counter1=counter;
  while(counter){
   asp_trn_fifo_buf[t_end++]=*data++;
   counter--;
   }
//  memcpy(&asp_trn_fifo_buf[t_end],data,counter);
  len-=counter1;
//  len-=counter;
  asp_trn_fifo_end+=counter1;
//  asp_trn_fifo_end+=counter;
  disable_int_no_interrupt();
  //�᫨ ��誠 �� ����� � ���뢠��� ����饭�, � ࠧ�蠥� ��
  if ( ((asp_trn_fifo_end&(SERIAL_FIFO_TRN_LEN-1))!=(asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)))&&
        ((IFG1 & UTXIFG0) != 0)  ){	//���뢠��� ��।�� ����饭�?
   TXBUF0 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
   }
  enable_int_no_interrupt();
// }while(len);
return len;
}
u8 write_asp_trn_fifo(u8 data_wr){

 if ( ( (asp_trn_fifo_end+1)&(SERIAL_FIFO_TRN_LEN-1))== (asp_trn_fifo_start&(SERIAL_FIFO_TRN_LEN-1)) )
  return 0;
 disable_int_no_interrupt();
 asp_trn_fifo_buf[asp_trn_fifo_end++ & (SERIAL_FIFO_TRN_LEN-1)]=data_wr;

 if ((IFG1 & UTXIFG0) != 0)        // USART0 TX buffer ready?
  TXBUF0 = asp_trn_fifo_buf[asp_trn_fifo_start++ & (SERIAL_FIFO_TRN_LEN-1)];
 enable_int_no_interrupt();
 return 1;
}


/*
������⢮ ���祭��
  ����
_____

1          ���浪���
           ����� 
           �����
1          ⨯
           �����
1          ����� �����
2          CRC
1          x7B - ��થ� �����


���
�����

0x01      ����� ���⢥ত����.
          ���⢥ত����� �ਥ� ����� � 㪠����� ���浪��� ����஬
0x02      ����� ᨭ�஭���樨 �६���
          � ����� ��।����� ���� �� ������ ��।�� �� ��᫥����⥫쭮�� �����
          ����� �� �ॡ�� ���⢥ত���� � ���� ��� ���浪����� �����
0x03      
*/