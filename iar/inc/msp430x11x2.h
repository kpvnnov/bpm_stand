/********************************************************************
*
* Standard register and bit definitions for the Texas Instruments
* MSP430 microcontroller.
*
* This file supports assembler and C development for
* MSP430x11x2 devices.
*
* Texas Instruments, Version 1.1
*
* Rev. 1.1, Enclose all #define statements with parentheses
*
********************************************************************/

#ifndef __msp430x11x2
#define __msp430x11x2

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

#define IFG1_               (0x0002)  /* Interrupt Flag 1 */
sfrb    IFG1              = IFG1_;
#define WDTIFG              (0x01)
#define OFIFG               (0x02)
#define NMIIFG              (0x10)

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
* ADC10
************************************************************/

#define ADC10DTC0_          (0x0048)    /* ADC10 Data Transfer Control 0 */
sfrb    ADC10DTC0         = ADC10DTC0_;
#define ADC10DTC1_          (0x0049)    /* ADC10 Data Transfer Control 1 */
sfrb    ADC10DTC1         = ADC10DTC1_;
#define ADC10AE_            (0x004A)    /* ADC10 Analog Enable */
sfrb    ADC10AE           = ADC10AE_;

#define ADC10CTL0_          (0x01B0)    /* ADC10 Control 0 */
sfrw    ADC10CTL0         = ADC10CTL0_;
#define ADC10CTL1_          (0x01B2)    /* ADC10 Control 1 */
sfrw    ADC10CTL1         = ADC10CTL1_;
#define ADC10MEM_           (0x01B4)    /* ADC10 Memory */
sfrw    ADC10MEM          = ADC10MEM_;
#define ADC10SA_            (0x01BC)    /* ADC10 Data Transfer Start Address */
sfrw    ADC10SA           = ADC10SA_;

#define ADC10SC             (0x001)     /* ADC10CTL0 */
#define ENC                 (0x002)
#define ADC10IFG            (0x004)
#define ADC10IE             (0x008)
#define ADC10ON             (0x010)
#define REFON               (0x020)
#define REF2_5V             (0x040)
#define MSC                 (0x080)
#define REFBURST            (0x100)
#define REFOUT              (0x200)
#define ADC10SR             (0x400)

#define ADC10SHT_0          (0*0x800)   /* 4 x ADC10CLKs */
#define ADC10SHT_1          (1*0x800)   /* 8 x ADC10CLKs */
#define ADC10SHT_2          (2*0x800)   /* 16 x ADC10CLKs */
#define ADC10SHT_3          (3*0x800)   /* 64 x ADC10CLKs */

#define SREF_0              (0*0x2000)  /* VR+ = AVCC and VR- = AVSS */
#define SREF_1              (1*0x2000)  /* VR+ = VREF+ and VR- = AVSS */
#define SREF_2              (2*0x2000)  /* VR+ = VEREF+ and VR- = AVSS */
#define SREF_3              (3*0x2000)  /* VR+ = VEREF+ and VR- = AVSS */
#define SREF_4              (4*0x2000)  /* VR+ = AVCC and VR- = VREF-/VEREF- */
#define SREF_5              (5*0x2000)  /* VR+ = VREF+ and VR- = VREF-/VEREF- */
#define SREF_6              (6*0x2000)  /* VR+ = VEREF+ and VR- = VREF-/VEREF- */
#define SREF_7              (7*0x2000)  /* VR+ = VEREF+ and VR- = VREF-/VEREF- */

#define ADC10BUSY           (0x0001)    /* ADC10CTL1 */

#define CONSEQ_0            (0*2)       /* Single channel single conversion */
#define CONSEQ_1            (1*2)       /* Sequence of channels */
#define CONSEQ_2            (2*2)       /* Repeat single channel */
#define CONSEQ_3            (3*2)       /* Repeat sequence of channels */

#define ADC10SSEL_0         (0*8)       /* ADC10OSC */
#define ADC10SSEL_1         (1*8)       /* ACLK */
#define ADC10SSEL_2         (2*8)       /* MCLK */
#define ADC10SSEL_3         (3*8)       /* SMCLK */

#define ADC10DIV_0          (0*0x20)
#define ADC10DIV_1          (1*0x20)
#define ADC10DIV_2          (2*0x20) 
#define ADC10DIV_3          (3*0x20)
#define ADC10DIV_4          (4*0x20)
#define ADC10DIV_5          (5*0x20)
#define ADC10DIV_6          (6*0x20)
#define ADC10DIV_7          (7*0x20)

#define ISSH                (0x0100)
#define ADC10DF             (0x0200)

#define SHS_0               (0*0x400)   /* ADC10SC */
#define SHS_1               (1*0x400)   /* TA3 OUT1 */
#define SHS_2               (2*0x400)   /* TA3 OUT0 */
#define SHS_3               (3*0x400)   /* TA3 OUT2 */

#define INCH_0               (0*0x1000)
#define INCH_1               (1*0x1000)
#define INCH_2               (2*0x1000)
#define INCH_3               (3*0x1000)
#define INCH_4               (4*0x1000)
#define INCH_5               (5*0x1000)
#define INCH_6               (6*0x1000)
#define INCH_7               (7*0x1000)
#define INCH_8               (8*0x1000)
#define INCH_9               (9*0x1000)
#define INCH_10             (10*0x1000)
#define INCH_11             (11*0x1000)
#define INCH_12             (12*0x1000) /* Selects Channel 11 */
#define INCH_13             (13*0x1000) /* Selects Channel 11 */
#define INCH_14             (14*0x1000) /* Selects Channel 11 */
#define INCH_15             (15*0x1000) /* Selects Channel 11 */

#define ADC10FETCH          (0x001)     /* ADC10DTC0 */
#define ADC10B1             (0x002)
#define ADC10CT             (0x004)
#define ADC10TB             (0x008)

#define ADC10DISABLE        (0x000)     /* ADC10DTC1 */

/************************************************************
* Interrupt Vectors (offset from 0xFFE0)
************************************************************/

#define PORT1_VECTOR        (2 * 2)  /* 0xFFE4 Port 1 */
#define PORT2_VECTOR        (3 * 2)  /* 0xFFE6 Port 2 */
#define ADC10_VECTOR        (5 * 2)  /* 0xFFEA ADC10 */
#define TIMERA1_VECTOR      (8 * 2)  /* 0xFFF0 Timer A CC1-2, TA */
#define TIMERA0_VECTOR      (9 * 2)  /* 0xFFF2 Timer A CC0 */
#define WDT_VECTOR          (10 * 2) /* 0xFFF4 Watchdog Timer */
#define NMI_VECTOR          (14 * 2) /* 0xFFFC Non-maskable */
#define RESET_VECTOR        (15 * 2) /* 0xFFFE Reset [Highest Priority] */

/************************************************************
* End of Modules
************************************************************/

#endif /* #ifndef __msp430x11x2 */

