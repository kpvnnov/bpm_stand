// $Id: loader.c,v 1.1 2004-03-10 16:31:40 peter Exp $
#include  <msp430x14x.h>
//���� ����������
#include "type_def.h"

// ������ ��� ������ � ��������
#include "uart_p.h"
//������ �������
extern u8 packets[MAXQUE*MAXPACKETLEN];
// ��������� ������ ��� �������
extern struct que queue[MAXQUE];

#pragma codeseg(LOADER_CODE)

int check_loader(void);
void load(void);
void main(void){
 check_loader();
 load();
 }
void load(void){
}

int check_loader(void){
}
