/*			 - in430.h -

   Intrinsic functions for the MSP430

   $Revision: 1.1 $

*/

#ifndef __IN430_INCLUDED
#define __IN430_INCLUDED

#if __TID__ & 0x8000
#pragma function=intrinsic(0)
#endif

unsigned short _BIS_SR(unsigned short);

unsigned short _BIC_SR(unsigned short);

unsigned short _BIS_SR_IRQ(unsigned short);

unsigned short _BIC_SR_IRQ(unsigned short);


void _DINT(void);

void _EINT(void);

void _NOP(void);

void _OPC(const unsigned short op);

#if __TID__ & 0x8000
#pragma function=default
#endif

#endif	/* __IN430_INCLUDED */
