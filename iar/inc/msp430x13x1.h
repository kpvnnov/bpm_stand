/********************************************************************
*
* Standard register and bit definitions for the Texas Instruments
* MSP430 microcontroller.
*
* This file supports assembler and C development for
* MSP430x13x1 devices.
*
* Texas Instruments, Version 1.1
*
* Rev. 1.1, Enclose all #define statements with parentheses
*
*********************************************************************/

#ifndef __msp430x13x1
#define __msp430x13x1

/************************************************************
* STANDARD BITS
************************************************************/

#define BIT0                (0x0001)
#define BIT1                (0x0002)
#define BIT2                (0x0004)
#define BIT3                (0x0008)
#define BIT4                (0x0010)
#define BIT5                (0x0020)
#define BIT6                (0x0040)
#define BIT7                (0x0080)
#define BIT8                (0x0100)
#define BIT9                (0x0200)
#define BITA                (0x0400)
#define BITB                (0x0800)
#define BITC                (0x1000)
#define BITD                (0x2000)
#define BITE                (0x4000)
#define BITF                (0x8000)

/************************************************************
* STATUS REGISTER BITS
************************************************************/

#define C                   (0x0001)
#define Z                   (0x0002)
#define N                   (0x0004)
#define V                   (0x0100)
#define GIE                 (0x0008)
#define CPUOFF              (0x0010)
#define OSCOFF              (0x0020)
#define SCG0                (0x0040)
#define SCG1                (0x0080)

/* Low Power Modes coded with Bits 4-7 in SR */

#ifndef __IAR_SYSTEMS_ICC /* Begin #defines for assembler */
#define LPM0                (CPUOFF)
#define LPM1                (SCG0+CPUOFF)
#define LPM2                (SCG1+CPUOFF)
#define LPM3                (SCG1+SCG0+CPUOFF)
#define LPM4                (SCG1+SCG0+OSCOFF+CPUOFF)
/* End #defines for assembler */

#else /* Begin #defines for C */
#define LPM0_bits           (CPUOFF)
#define LPM1_bits           (SCG0+CPUOFF)
#define LPM2_bits           (SCG1+CPUOFF)
#define LPM3_bits           (SCG1+SCG0+CPUOFF)
#define LPM4_bits           (SCG1+SCG0+OSCOFF+CPUOFF)

#include <In430.h>

#define LPM0      _BIS_SR(LPM0_bits)     /* Enter Low Power Mode 0 */
#define LPM0_EXIT _BIC_SR_IRQ(LPM0_bits) /* Exit Low Power Mode 0 */
#define LPM1      _BIS_SR(LPM1_bits)     /* Enter Low Power Mode 1 */
#define LPM1_EXIT _BIC_SR_IRQ(LPM1_bits) /* Exit Low Power Mode 1 */
#define LPM2      _BIS_SR(LPM2_bits)     /* Enter Low Power Mode 2 */
#define LPM2_EXIT _BIC_SR_IRQ(LPM2_bits) /* Exit Low Power Mode 2 */
#define LPM3      _BIS_SR(LPM3_bits)     /* Enter Low Power Mode 3 */
#define LPM3_EXIT _BIC_SR_IRQ(LPM3_bits) /* Exit Low Power Mode 3 */
#define LPM4      _BIS_SR(LPM4_bits)     /* Enter Low Power Mode 4 */
#define LPM4_EXIT _BIC_SR_IRQ(LPM4_bits) /* Exit Low Power Mode 4 */
#endif /* End #defines for C */

/************************************************************
* PERIPHERAL FILE MAP
************************************************************/

/************************************************************
* SPECIAL FUNCTION REGISTER ADDRESSES + CONTROL BITS
************************************************************/

#define IE1_                (0x0000)  /* Interrupt Enable 1 */
sfrb    IE1               = IE1_;
#define WDTIE               (0x01)
#define OFIE                (0x02)
#define NMIIE               (0x10)
#define ACCVIE              (0x20)
#define URXIE0              (0x40)
#define UTXIE0              (0x80)

#define IFG1_               (0x0002)  /* Interrupt Flag 1 */
sfrb    IFG1              = IFG1_;
#define WDTIFG              (0x01)
#define OFIFG               (0x02)
#define NMIIFG              (0x10)
#define URXIFG0             (0x40)
#define UTXIFG0             (0x80)

#define ME1_                (0x0004)  /* Module Enable 1 */
sfrb    ME1               = ME1_;
#define URXE0               (0x40)
#define USPIE0              (0x40)
#define UTXE0               (0x80)

/************************************************************
* WATCHDOG TIMER
************************************************************/

#define WDTCTL_             (0x0120)  /* Watchdog Timer Control */
sfrw    WDTCTL            = WDTCTL_;
/* The bit names have been prefixed with "WDT" */
#define WDTIS0              (0x0001)
#define WDTIS1              (0x0002)
#define WDTSSEL             (0x0004)
#define WDTCNTCL            (0x0008)
#define WDTTMSEL            (0x0010)
#define WDTNMI              (0x0020)
#define WDTNMIES            (0x0040)
#define WDTHOLD             (0x0080)

#define WDTPW               (0x5A00)

/* WDT-interval times [1ms] coded with Bits 0-2 */
/* WDT is clocked by fMCLK (assumed 1MHz) */
#define WDT_MDLY_32         (WDTPW+WDTTMSEL+WDTCNTCL)                         /* 32ms interval (default) */
#define WDT_MDLY_8          (WDTPW+WDTTMSEL+WDTCNTCL+WDTIS0)                  /* 8ms     " */
#define WDT_MDLY_0_5        (WDTPW+WDTTMSEL+WDTCNTCL+WDTIS1)                  /* 0.5ms   " */
#define WDT_MDLY_0_064      (WDTPW+WDTTMSEL+WDTCNTCL+WDTIS1+WDTIS0)           /* 0.064ms " */
/* WDT is clocked by fACLK (assumed 32KHz) */
#define WDT_ADLY_1000       (WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL)                 /* 1000ms  " */
#define WDT_ADLY_250        (WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL+WDTIS0)          /* 250ms   " */
#define WDT_ADLY_16         (WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL+WDTIS1)          /* 16ms    " */
#define WDT_ADLY_1_9        (WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL+WDTIS1+WDTIS0)   /* 1.9ms   " */
/* Watchdog mode -> reset after expired time */
/* WDT is clocked by fMCLK (assumed 1MHz) */
#define WDT_MRST_32         (WDTPW+WDTCNTCL)                                  /* 32ms interval (default) */
#define WDT_MRST_8          (WDTPW+WDTCNTCL+WDTIS0)                           /* 8ms     " */
#define WDT_MRST_0_5        (WDTPW+WDTCNTCL+WDTIS1)                           /* 0.5ms   " */
#define WDT_MRST_0_064      (WDTPW+WDTCNTCL+WDTIS1+WDTIS0)                    /* 0.064ms " */
/* WDT is clocked by fACLK (assumed 32KHz) */
#define WDT_ARST_1000       (WDTPW+WDTCNTCL+WDTSSEL)                          /* 1000ms  " */
#define WDT_ARST_250        (WDTPW+WDTCNTCL+WDTSSEL+WDTIS0)                   /* 250ms   " */
#define WDT_ARST_16         (WDTPW+WDTCNTCL+WDTSSEL+WDTIS1)                   /* 16ms    " */
#define WDT_ARST_1_9        (WDTPW+WDTCNTCL+WDTSSEL+WDTIS1+WDTIS0)            /* 1.9ms   " */

/* INTERRUPT CONTROL */
/* These two bits are defined in the Special Function Registers */
/* #define WDTIE               0x01 */
/* #define WDTIFG              0x01 */

/************************************************************
* DIGITAL I/O Port1/2
************************************************************/

#define P1IN_               (0x0020)  /* Port 1 Input */
const sfrb P1IN           = P1IN_;
#define P1OUT_              (0x0021)  /* Port 1 Output */
sfrb    P1OUT             = P1OUT_;
#define P1DIR_              (0x0022)  /* Port 1 Direction */
sfrb    P1DIR             = P1DIR_;
#define P1IFG_              (0x0023)  /* Port 1 Interrupt Flag */
sfrb    P1IFG             = P1IFG_;
#define P1IES_              (0x0024)  /* Port 1 Interrupt Edge Select */
sfrb    P1IES             = P1IES_;
#define P1IE_               (0x0025)  /* Port 1 Interrupt Enable */
sfrb    P1IE              = P1IE_;
#define P1SEL_              (0x0026)  /* Port 1 Selection */
sfrb    P1SEL             = P1SEL_;

#define P2IN_               (0x0028)  /* Port 2 Input */
const sfrb P2IN           = P2IN_;
#define P2OUT_              (0x0029)  /* Port 2 Output */
sfrb    P2OUT             = P2OUT_;
#define P2DIR_              (0x002A)  /* Port 2 Direction */
sfrb    P2DIR             = P2DIR_;
#define P2IFG_              (0x002B)  /* Port 2 Interrupt Flag */
sfrb    P2IFG             = P2IFG_;
#define P2IES_              (0x002C)  /* Port 2 Interrupt Edge Select */
sfrb    P2IES             = P2IES_;
#define P2IE_               (0x002D)  /* Port 2 Interrupt Enable */
sfrb    P2IE              = P2IE_;
#define P2SEL_              (0x002E)  /* Port 2 Selection */
sfrb    P2SEL             = P2SEL_;

/************************************************************
* DIGITAL I/O Port3/4
************************************************************/

#define P3IN_               (0x0018)  /* Port 3 Input */
const sfrb P3IN           = P3IN_;
#define P3OUT_              (0x0019)  /* Port 3 Output */
sfrb    P3OUT             = P3OUT_;
#define P3DIR_              (0x001A)  /* Port 3 Direction */
sfrb    P3DIR             = P3DIR_;
#define P3SEL_              (0x001B)  /* Port 3 Selection */
sfrb    P3SEL             = P3SEL_;

#define P4IN_               (0x001C)  /* Port 4 Input */
const sfrb P4IN           = P4IN_;
#define P4OUT_              (0x001D)  /* Port 4 Output */
sfrb    P4OUT             = P4OUT_;
#define P4DIR_              (0x001E)  /* Port 4 Direction */
sfrb    P4DIR             = P4DIR_;
#define P4SEL_              (0x001F)  /* Port 4 Selection */
sfrb    P4SEL             = P4SEL_;

/************************************************************
* DIGITAL I/O Port5/6
************************************************************/

#define P5IN_               (0x0030)  /* Port 5 Input */
const sfrb P5IN           = P5IN_;
#define P5OUT_              (0x0031)  /* Port 5 Output */
sfrb    P5OUT             = P5OUT_;
#define P5DIR_              (0x0032)  /* Port 5 Direction */
sfrb    P5DIR             = P5DIR_;
#define P5SEL_              (0x0033)  /* Port 5 Selection */
sfrb    P5SEL             = P5SEL_;

#define P6IN_               (0x0034)  /* Port 6 Input */
const sfrb P6IN           = P6IN_;
#define P6OUT_              (0x0035)  /* Port 6 Output */
sfrb    P6OUT             = P6OUT_;
#define P6DIR_              (0x0036)  /* Port 6 Direction */
sfrb    P6DIR             = P6DIR_;
#define P6SEL_              (0x0037)  /* Port 6 Selection */
sfrb    P6SEL             = P6SEL_;

/************************************************************
* USART
************************************************************/

#define PENA                (0x80)        /* UCTL */
#define PEV                 (0x40)
#define SPB                 (0x20)        /* to distinguish from stackpointer SP */
#define CHAR                (0x10)
#define LISTEN              (0x08)
#define SYNC                (0x04)
#define MM                  (0x02)
#define SWRST               (0x01)

#define CKPH                (0x80)        /* UTCTL */
#define CKPL                (0x40)
#define SSEL1               (0x20)
#define SSEL0               (0x10)
#define URXSE               (0x08)
#define TXWAKE              (0x04)
#define STC                 (0x02)
#define TXEPT               (0x01)

#define FE                  (0x80)        /* URCTL */
#define PE                  (0x40)
#define OE                  (0x20)
#define BRK                 (0x10)
#define URXEIE              (0x08)
#define URXWIE              (0x04)
#define RXWAKE              (0x02)
#define RXERR               (0x01)

/************************************************************
* USART 0
************************************************************/

#define U0CTL_              (0x0070)  /* UART 0 Control */
sfrb    U0CTL             = U0CTL_;
#define U0TCTL_             (0x0071)  /* UART 0 Transmit Control */
sfrb    U0TCTL            = U0TCTL_;
#define U0RCTL_             (0x0072)  /* UART 0 Receive Control */
sfrb    U0RCTL            = U0RCTL_;
#define U0MCTL_             (0x0073)  /* UART 0 Modulation Control */
sfrb    U0MCTL            = U0MCTL_;
#define U0BR0_              (0x0074)  /* UART 0 Baud Rate 0 */
sfrb    U0BR0             = U0BR0_;
#define U0BR1_              (0x0075)  /* UART 0 Baud Rate 1 */
sfrb    U0BR1             = U0BR1_;
#define U0RXBUF_            (0x0076)  /* UART 0 Receive Buffer */
const sfrb U0RXBUF        = U0RXBUF_;
#define U0TXBUF_            (0x0077)  /* UART 0 Transmit Buffer */
sfrb    U0TXBUF           = U0TXBUF_;

/* Alternate register names */

#define UCTL0_              (0x0070)  /* UART 0 Control */
sfrb    UCTL0             = UCTL0_;
#define UTCTL0_             (0x0071)  /* UART 0 Transmit Control */
sfrb    UTCTL0            = UTCTL0_;
#define URCTL0_             (0x0072)  /* UART 0 Receive Control */
sfrb    URCTL0            = URCTL0_;
#define UMCTL0_             (0x0073)  /* UART 0 Modulation Control */
sfrb    UMCTL0            = UMCTL0_;
#define UBR00_              (0x0074)  /* UART 0 Baud Rate 0 */
sfrb    UBR00             = UBR00_;
#define UBR10_              (0x0075)  /* UART 0 Baud Rate 1 */
sfrb    UBR10             = UBR10_;
#define RXBUF0_             (0x0076)  /* UART 0 Receive Buffer */
const sfrb RXBUF0         = RXBUF0_;
#define TXBUF0_             (0x0077)  /* UART 0 Transmit Buffer */
sfrb    TXBUF0            = TXBUF0_;

#define UCTL_0_             (0x0070)  /* UART 0 Control */
sfrb    UCTL_0            = UCTL_0_;
#define UTCTL_0_            (0x0071)  /* UART 0 Transmit Control */
sfrb    UTCTL_0           = UTCTL_0_;
#define URCTL_0_            (0x0072)  /* UART 0 Receive Control */
sfrb    URCTL_0           = URCTL_0_;
#define UMCTL_0_            (0x0073)  /* UART 0 Modulation Control */
sfrb    UMCTL_0           = UMCTL_0_;
#define UBR0_0_             (0x0074)  /* UART 0 Baud Rate 0 */
sfrb    UBR0_0            = UBR0_0_;
#define UBR1_0_             (0x0075)  /* UART 0 Baud Rate 1 */
sfrb    UBR1_0            = UBR1_0_;
#define RXBUF_0_            (0x0076)  /* UART 0 Receive Buffer */
const sfrb RXBUF_0        = RXBUF_0_;
#define TXBUF_0_            (0x0077)  /* UART 0 Transmit Buffer */
sfrb    TXBUF_0           = TXBUF_0_;

/************************************************************
* Timer A
************************************************************/

#define TAIV_               (0x012E)  /* Timer A Interrupt Vector Word */
const sfrw TAIV           = TAIV_;
#define TACTL_              (0x0160)  /* Timer A Control */
sfrw    TACTL             = TACTL_;
#define TACCTL0_            (0x0162)  /* Timer A Capture/Compare Control 0 */
sfrw    TACCTL0           = TACCTL0_;
#define TACCTL1_            (0x0164)  /* Timer A Capture/Compare Control 1 */
sfrw    TACCTL1           = TACCTL1_;
#define TACCTL2_            (0x0166)  /* Timer A Capture/Compare Control 2 */
sfrw    TACCTL2           = TACCTL2_;
#define TAR_                (0x0170)  /* Timer A */
sfrw    TAR               = TAR_;
#define TACCR0_             (0x0172)  /* Timer A Capture/Compare 0 */
sfrw    TACCR0            = TACCR0_;
#define TACCR1_             (0x0174)  /* Timer A Capture/Compare 1 */
sfrw    TACCR1            = TACCR1_;
#define TACCR2_             (0x0176)  /* Timer A Capture/Compare 2 */
sfrw    TACCR2            = TACCR2_;

/* Alternate register names */
#define CCTL0_              (0x0162)  /* Timer A Capture/Compare Control 0 */
sfrw    CCTL0             = CCTL0_;
#define CCTL1_              (0x0164)  /* Timer A Capture/Compare Control 1 */
sfrw    CCTL1             = CCTL1_;
#define CCTL2_              (0x0166)  /* Timer A Capture/Compare Control 2 */
sfrw    CCTL2             = CCTL2_;
#define CCR0_               (0x0172)  /* Timer A Capture/Compare 0 */
sfrw    CCR0              = CCR0_;
#define CCR1_               (0x0174)  /* Timer A Capture/Compare 1 */
sfrw    CCR1              = CCR1_;
#define CCR2_               (0x0176)  /* Timer A Capture/Compare 2 */
sfrw    CCR2              = CCR2_;

#define TASSEL2             (0x0400)  /* unused */        /* to distinguish from UART SSELx */
#define TASSEL1             (0x0200)  /* Timer A clock source select 0 */
#define TASSEL0             (0x0100)  /* Timer A clock source select 1 */
#define ID1                 (0x0080)  /* Timer A clock input devider 1 */
#define ID0                 (0x0040)  /* Timer A clock input devider 0 */
#define MC1                 (0x0020)  /* Timer A mode control 1 */
#define MC0                 (0x0010)  /* Timer A mode control 0 */
#define TACLR               (0x0004)  /* Timer A counter clear */
#define TAIE                (0x0002)  /* Timer A counter interrupt enable */
#define TAIFG               (0x0001)  /* Timer A counter interrupt flag */

#define MC_0                (0*0x10)  /* Timer A mode control: 0 - Stop */
#define MC_1                (1*0x10)  /* Timer A mode control: 1 - Up to CCR0 */
#define MC_2                (2*0x10)  /* Timer A mode control: 2 - Continous up */
#define MC_3                (3*0x10)  /* Timer A mode control: 3 - Up/Down */
#define ID_0                (0*0x40)  /* Timer A input divider: 0 - /1 */
#define ID_1                (1*0x40)  /* Timer A input divider: 1 - /2 */
#define ID_2                (2*0x40)  /* Timer A input divider: 2 - /4 */
#define ID_3                (3*0x40)  /* Timer A input divider: 3 - /8 */
#define TASSEL_0            (0*0x100) /* Timer A clock source select: 0 - TACLK */
#define TASSEL_1            (1*0x100) /* Timer A clock source select: 1 - ACLK  */
#define TASSEL_2            (2*0x100) /* Timer A clock source select: 2 - SMCLK */
#define TASSEL_3            (3*0x100) /* Timer A clock source select: 3 - INCLK */

#define CM1                 (0x8000)  /* Capture mode 1 */
#define CM0                 (0x4000)  /* Capture mode 0 */
#define CCIS1               (0x2000)  /* Capture input select 1 */
#define CCIS0               (0x1000)  /* Capture input select 0 */
#define SCS                 (0x0800)  /* Capture sychronize */
#define SCCI                (0x0400)  /* Latched capture signal (read) */
#define CAP                 (0x0100)  /* Capture mode: 1 /Compare mode : 0 */
#define OUTMOD2             (0x0080)  /* Output mode 2 */
#define OUTMOD1             (0x0040)  /* Output mode 1 */
#define OUTMOD0             (0x0020)  /* Output mode 0 */
#define CCIE                (0x0010)  /* Capture/compare interrupt enable */
#define CCI                 (0x0008)  /* Capture input signal (read) */
#define OUT                 (0x0004)  /* PWM Output signal if output mode 0 */
#define COV                 (0x0002)  /* Capture/compare overflow flag */
#define CCIFG               (0x0001)  /* Capture/compare interrupt flag */

#define OUTMOD_0            (0*0x20)  /* PWM output mode: 0 - output only */
#define OUTMOD_1            (1*0x20)  /* PWM output mode: 1 - set */
#define OUTMOD_2            (2*0x20)  /* PWM output mode: 2 - PWM toggle/reset */
#define OUTMOD_3            (3*0x20)  /* PWM output mode: 3 - PWM set/reset */
#define OUTMOD_4            (4*0x20)  /* PWM output mode: 4 - toggle */
#define OUTMOD_5            (5*0x20)  /* PWM output mode: 5 - Reset */
#define OUTMOD_6            (6*0x20)  /* PWM output mode: 6 - PWM toggle/set */
#define OUTMOD_7            (7*0x20)  /* PWM output mode: 7 - PWM reset/set */
#define CCIS_0              (0*0x1000) /* Capture input select: 0 - CCIxA */
#define CCIS_1              (1*0x1000) /* Capture input select: 1 - CCIxB */
#define CCIS_2              (2*0x1000) /* Capture input select: 2 - GND */
#define CCIS_3              (3*0x1000) /* Capture input select: 3 - Vcc */
#define CM_0                (0*0x4000) /* Capture mode: 0 - disabled */
#define CM_1                (1*0x4000) /* Capture mode: 1 - pos. edge */
#define CM_2                (2*0x4000) /* Capture mode: 1 - neg. edge */
#define CM_3                (3*0x4000) /* Capture mode: 1 - both edges */

/************************************************************
* Timer B
************************************************************/

#define TBIV_               (0x011E)  /* Timer B Interrupt Vector Word */
const sfrw TBIV           = TBIV_;
#define TBCTL_              (0x0180)  /* Timer B Control */
sfrw    TBCTL             = TBCTL_;
#define TBCCTL0_            (0x0182)  /* Timer B Capture/Compare Control 0 */
sfrw    TBCCTL0           = TBCCTL0_;
#define TBCCTL1_            (0x0184)  /* Timer B Capture/Compare Control 1 */
sfrw    TBCCTL1           = TBCCTL1_;
#define TBCCTL2_            (0x0186)  /* Timer B Capture/Compare Control 2 */
sfrw    TBCCTL2           = TBCCTL2_;
#define TBR_                (0x0190)  /* Timer B */
sfrw    TBR               = TBR_;
#define TBCCR0_             (0x0192)  /* Timer B Capture/Compare 0 */
sfrw    TBCCR0            = TBCCR0_;
#define TBCCR1_             (0x0194)  /* Timer B Capture/Compare 1 */
sfrw    TBCCR1            = TBCCR1_;
#define TBCCR2_             (0x0196)  /* Timer B Capture/Compare 2 */
sfrw    TBCCR2            = TBCCR2_;

#define SHR1                (0x4000)  /* Timer B Compare latch load group 1 */
#define SHR0                (0x2000)  /* Timer B Compare latch load group 0 */
#define TBCLGRP1            (0x4000)  /* Timer B Compare latch load group 1 */
#define TBCLGRP0            (0x2000)  /* Timer B Compare latch load group 0 */
#define CNTL1               (0x1000)  /* Counter lenght 1 */
#define CNTL0               (0x0800)  /* Counter lenght 0 */
#define TBSSEL2             (0x0400)  /* unused */
#define TBSSEL1             (0x0200)  /* Clock source 1 */
#define TBSSEL0             (0x0100)  /* Clock source 0 */
#define TBCLR               (0x0004)  /* Timer B counter clear */
#define TBIE                (0x0002)  /* Timer B interrupt enable */
#define TBIFG               (0x0001)  /* Timer B interrupt flag */

#define TBSSEL_0            (0*0x0100)  /* Clock Source: TBCLK */
#define TBSSEL_1            (1*0x0100)  /* Clock Source: ACLK  */
#define TBSSEL_2            (2*0x0100)  /* Clock Source: SMCLK */
#define TBSSEL_3            (3*0x0100)  /* Clock Source: INCLK */
#define CNTL_0              (0*0x0800)  /* Counter lenght: 16 bit */
#define CNTL_1              (1*0x0800)  /* Counter lenght: 12 bit */
#define CNTL_2              (2*0x0800)  /* Counter lenght: 10 bit */
#define CNTL_3              (3*0x0800)  /* Counter lenght:  8 bit */
#define SHR_0               (0*0x2000)  /* Timer B Group: 0 - individually */
#define SHR_1               (1*0x2000)  /* Timer B Group: 1 - 3 groups (1-2, 3-4, 5-6) */
#define SHR_2               (2*0x2000)  /* Timer B Group: 2 - 2 groups (1-3, 4-6)*/
#define SHR_3               (3*0x2000)  /* Timer B Group: 3 - 1 group (all) */
#define TBCLGRP_0           (0*0x2000)  /* Timer B Group: 0 - individually */
#define TBCLGRP_1           (1*0x2000)  /* Timer B Group: 1 - 3 groups (1-2, 3-4, 5-6) */
#define TBCLGRP_2           (2*0x2000)  /* Timer B Group: 2 - 2 groups (1-3, 4-6)*/
#define TBCLGRP_3           (3*0x2000)  /* Timer B Group: 3 - 1 group (all) */

/* Additional Timer B Control Register bits are defined in Timer A */

#define SLSHR1              (0x0400)  /* Compare latch load source 1 */
#define SLSHR0              (0x0200)  /* Compare latch load source 0 */
#define CLLD1               (0x0400)  /* Compare latch load source 1 */
#define CLLD0               (0x0200)  /* Compare latch load source 0 */

#define SLSHR_0             (0*0x0200)  /* Compare latch load sourec : 0 - immediate */
#define SLSHR_1             (1*0x0200)  /* Compare latch load sourec : 1 - TBR counts to 0 */
#define SLSHR_2             (2*0x0200)  /* Compare latch load sourec : 2 - up/down */
#define SLSHR_3             (3*0x0200)  /* Compare latch load sourec : 3 - TBR counts to TBCTL0 */

#define CLLD_0              (0*0x0200)  /* Compare latch load sourec : 0 - immediate */
#define CLLD_1              (1*0x0200)  /* Compare latch load sourec : 1 - TBR counts to 0 */
#define CLLD_2              (2*0x0200)  /* Compare latch load sourec : 2 - up/down */
#define CLLD_3              (3*0x0200)  /* Compare latch load sourec : 3 - TBR counts to TBCTL0 */

/************************************************************
* Basic Clock Module
************************************************************/

#define DCOCTL_             (0x0056)  /* DCO Clock Frequency Control */
sfrb    DCOCTL            = DCOCTL_;
#define BCSCTL1_            (0x0057)  /* Basic Clock System Control 1 */
sfrb    BCSCTL1           = BCSCTL1_;
#define BCSCTL2_            (0x0058)  /* Basic Clock System Control 2 */
sfrb    BCSCTL2           = BCSCTL2_;

#define MOD0                (0x01)   /* Modulation Bit 0 */
#define MOD1                (0x02)   /* Modulation Bit 1 */
#define MOD2                (0x04)   /* Modulation Bit 2 */
#define MOD3                (0x08)   /* Modulation Bit 3 */
#define MOD4                (0x10)   /* Modulation Bit 4 */
#define DCO0                (0x20)   /* DCO Select Bit 0 */
#define DCO1                (0x40)   /* DCO Select Bit 1 */
#define DCO2                (0x80)   /* DCO Select Bit 2 */

#define RSEL0               (0x01)   /* Resistor Select Bit 0 */
#define RSEL1               (0x02)   /* Resistor Select Bit 1 */
#define RSEL2               (0x04)   /* Resistor Select Bit 2 */
#define XT5V                (0x08)   /* XT5V should always be reset */
#define DIVA0               (0x10)   /* ACLK Divider 0 */
#define DIVA1               (0x20)   /* ACLK Divider 1 */
#define XTS                 (0x40)   /* LFXTCLK 0:Low Freq. / 1: High Freq. */
#define XT2OFF              (0x80)   /* Enable XT2CLK */

#define DIVA_0              (0x00)   /* ACLK Divider 0: /1 */
#define DIVA_1              (0x10)   /* ACLK Divider 1: /2 */
#define DIVA_2              (0x20)   /* ACLK Divider 2: /4 */
#define DIVA_3              (0x30)   /* ACLK Divider 3: /8 */

#define DCOR                (0x01)   /* Enable External Resistor : 1 */
#define DIVS0               (0x02)   /* SMCLK Divider 0 */
#define DIVS1               (0x04)   /* SMCLK Divider 1 */
#define SELS                (0x08)   /* SMCLK Source Select 0:DCOCLK / 1:XT2CLK/LFXTCLK */
#define DIVM0               (0x10)   /* MCLK Divider 0 */
#define DIVM1               (0x20)   /* MCLK Divider 1 */
#define SELM0               (0x40)   /* MCLK Source Select 0 */
#define SELM1               (0x80)   /* MCLK Source Select 1 */

#define DIVS_0              (0x00)   /* SMCLK Divider 0: /1 */
#define DIVS_1              (0x02)   /* SMCLK Divider 1: /2 */
#define DIVS_2              (0x04)   /* SMCLK Divider 2: /4 */
#define DIVS_3              (0x06)   /* SMCLK Divider 3: /8 */

#define DIVM_0              (0x00)   /* MCLK Divider 0: /1 */
#define DIVM_1              (0x10)   /* MCLK Divider 1: /2 */
#define DIVM_2              (0x20)   /* MCLK Divider 2: /4 */
#define DIVM_3              (0x30)   /* MCLK Divider 3: /8 */

#define SELM_0              (0x00)   /* MCLK Source Select 0: DCOCLK */
#define SELM_1              (0x40)   /* MCLK Source Select 1: DCOCLK */
#define SELM_2              (0x80)   /* MCLK Source Select 2: XT2CLK/LFXTCLK */
#define SELM_3              (0xC0)   /* MCLK Source Select 3: LFXTCLK */

/*************************************************************
* Flash Memory
*************************************************************/

#define FCTL1_              (0x0128)  /* FLASH Control 1 */
sfrw    FCTL1             = FCTL1_;
#define FCTL2_              (0x012A)  /* FLASH Control 2 */
sfrw    FCTL2             = FCTL2_;
#define FCTL3_              (0x012C)  /* FLASH Control 3 */
sfrw    FCTL3             = FCTL3_;

#define FRKEY               (0x9600)  /* Flash key returned by read */
#define FWKEY               (0xA500)  /* Flash key for write */
#define FXKEY               (0x3300)  /* for use with XOR instruction */

#define ERASE               (0x0002)  /* Enable bit for Flash segment erase */
#define MERAS               (0x0004)  /* Enable bit for Flash mass erase */
#define WRT                 (0x0040)  /* Enable bit for Flash write */
#define BLKWRT              (0x0080)  /* Enable bit for Flash segment write */
#define SEGWRT              (0x0080)  /* old definition */ /* Enable bit for Flash segment write */

#define FN0                 (0x0001)  /* Devide Flash clock by: 2^0 */
#define FN1                 (0x0002)  /* Devide Flash clock by: 2^1 */
#define FN2                 (0x0004)  /* Devide Flash clock by: 2^2 */
#define FN3                 (0x0008)  /* Devide Flash clock by: 2^3 */
#define FN4                 (0x0010)  /* Devide Flash clock by: 2^4 */
#define FN5                 (0x0020)  /* Devide Flash clock by: 2^5 */
#define FSSEL0              (0x0040)  /* Flash clock select 0 */        /* to distinguish from UART SSELx */
#define FSSEL1              (0x0080)  /* Flash clock select 1 */

#define FSSEL_0             (0x0000)  /* Flash clock select: 0 - ACLK */
#define FSSEL_1             (0x0040)  /* Flash clock select: 1 - MCLK */
#define FSSEL_2             (0x0080)  /* Flash clock select: 2 - SMCLK */
#define FSSEL_3             (0x00C0)  /* Flash clock select: 3 - SMCLK */

#define BUSY                (0x0001)  /* Flash busy: 1 */
#define KEYV                (0x0002)  /* Flash Key violation flag */
#define ACCVIFG             (0x0004)  /* Flash Access violation flag */
#define WAIT                (0x0008)  /* Wait flag for segment write */
#define LOCK                (0x0010)  /* Lock bit: 1 - Flash is locked (read only) */
#define EMEX                (0x0020)  /* Flash Emergency Exit */

/************************************************************
* Comparator A
************************************************************/

#define CACTL1_             (0x0059)  /* Comparator A Control 1 */
sfrb    CACTL1            = CACTL1_;
#define CACTL2_             (0x005A)  /* Comparator A Control 2 */
sfrb    CACTL2            = CACTL2_;
#define CAPD_               (0x005B)  /* Comparator A Port Disable */
sfrb    CAPD              = CAPD_;

#define CAIFG               (0x01)    /* Comp. A Interrupt Flag */
#define CAIE                (0x02)    /* Comp. A Interrupt Enable */
#define CAIES               (0x04)    /* Comp. A Int. Edge Select: 0:rising / 1:falling */
#define CAON                (0x08)    /* Comp. A enable */
#define CAREF0              (0x10)    /* Comp. A Internal Reference Select 0 */
#define CAREF1              (0x20)    /* Comp. A Internal Reference Select 1 */
#define CARSEL              (0x40)    /* Comp. A Internal Reference Enable */
#define CAEX                (0x80)    /* Comp. A Exchange Inputs */

#define CAREF_0             (0x00)    /* Comp. A Int. Ref. Select 0 : Off */
#define CAREF_1             (0x10)    /* Comp. A Int. Ref. Select 1 : 0.25*Vcc */
#define CAREF_2             (0x20)    /* Comp. A Int. Ref. Select 2 : 0.5*Vcc */
#define CAREF_3             (0x30)    /* Comp. A Int. Ref. Select 3 : Vt*/

#define CAOUT               (0x01)    /* Comp. A Output */
#define CAF                 (0x02)    /* Comp. A Enable Output Filter */
#define P2CA0               (0x04)    /* Comp. A Connect External Signal to CA0 : 1 */
#define P2CA1               (0x08)    /* Comp. A Connect External Signal to CA1 : 1 */
#define CACTL24             (0x10)
#define CACTL25             (0x20)
#define CACTL26             (0x40)
#define CACTL27             (0x80)

#define CAPD0               (0x01)    /* Comp. A Disable Input Buffer of Port Register .0 */
#define CAPD1               (0x02)    /* Comp. A Disable Input Buffer of Port Register .1 */
#define CAPD2               (0x04)    /* Comp. A Disable Input Buffer of Port Register .2 */
#define CAPD3               (0x08)    /* Comp. A Disable Input Buffer of Port Register .3 */
#define CAPD4               (0x10)    /* Comp. A Disable Input Buffer of Port Register .4 */
#define CAPD5               (0x20)    /* Comp. A Disable Input Buffer of Port Register .5 */
#define CAPD6               (0x40)    /* Comp. A Disable Input Buffer of Port Register .6 */
#define CAPD7               (0x80)    /* Comp. A Disable Input Buffer of Port Register .7 */

/************************************************************
* Interrupt Vectors (offset from 0xFFE0)
************************************************************/

#define PORT2_VECTOR        (1 * 2)  /* 0xFFE2 Port 2 */
#define PORT1_VECTOR        (4 * 2)  /* 0xFFE8 Port 1 */
#define TIMERA1_VECTOR      (5 * 2)  /* 0xFFEA Timer A CC1-2, TA */
#define TIMERA0_VECTOR      (6 * 2)  /* 0xFFEC Timer A CC0 */
#define UART0TX_VECTOR      (8 * 2)  /* 0xFFF0 UART 0 Transmit */
#define UART0RX_VECTOR      (9 * 2)  /* 0xFFF2 UART 0 Receive */
#define WDT_VECTOR          (10 * 2) /* 0xFFF4 Watchdog Timer */
#define COMPARATORA_VECTOR  (11 * 2) /* 0xFFF6 Comparator A */
#define TIMERB1_VECTOR      (12 * 2) /* 0xFFF8 Timer B 1-2 */
#define TIMERB0_VECTOR      (13 * 2) /* 0xFFFA Timer B 0 */
#define NMI_VECTOR          (14 * 2) /* 0xFFFC Non-maskable */
#define RESET_VECTOR        (15 * 2) /* 0xFFFE Reset [Highest Priority] */

/************************************************************
* End of Modules
************************************************************/

#endif /* #ifndef __msp430x13x1 */

