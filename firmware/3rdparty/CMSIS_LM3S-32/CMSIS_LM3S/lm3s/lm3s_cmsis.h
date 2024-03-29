/**************************************************************************//**
 * @file     ARMCM3.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           ARMCM3 Device Series
 * @version  V5.00
 * @date     07. September 2016
 ******************************************************************************/
/*
 * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#ifndef ARMCM3_H
//#define ARMCM3_H
#ifndef __LM3S_CMSIS_H__
#define __LM3S_CMSIS_H__

#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================== */
/* --------------------- Interrupt Number Definition ------------------------ */
/* ========================================================================== */
typedef enum IRQn {
  /* ---------------  Cortex-M3 Processor Exceptions Numbers  --------------- */
  NonMaskableInt_IRQn     = -14,     /*!< 2 Non Maskable Interrupt            */
  MemoryManagement_IRQn   = -12,     /*!< 4 Cortex-M3 Memory Management Int   */
  BusFault_IRQn           = -11,     /*!< 5 Cortex-M3 Bus Fault Interrupt     */
  UsageFault_IRQn         = -10,     /*!< 6 Cortex-M3 Usage Fault Interrupt   */
  SVCall_IRQn             = -5,      /*!< 11 Cortex-M3 SV Call Interrupt      */
  DebugMonitor_IRQn       = -4,      /*!< 12 Cortex-M3 Debug Monitor Interrupt*/
  PendSV_IRQn             = -2,      /*!< 14 Cortex-M3 Pend SV Interrupt      */
  SysTick_IRQn            = -1,      /*!< 15 Cortex-M3 System Tick Interrupt  */

  /* --------------  LM3S-Specific Specific Interrupt Numbers  -------------- */
  GPIOPortA_IRQn          =  0,      /*!<  GPIO Port A                        */
  GPIOPortB_IRQn          =  1,      /*!<  GPIO Port B                        */
  GPIOPortC_IRQn          =  2,      /*!<  GPIO Port C                        */
  GPIOPortD_IRQn          =  3,      /*!<  GPIO Port D                        */
  GPIOPortE_IRQn          =  4,      /*!<  GPIO Port E                        */
  UART0_IRQn              =  5,      /*!<  UART0                              */
  UART1_IRQn              =  6,      /*!<  UART1                              */
  SSI0_IRQn               =  7,      /*!<  SSI0                               */
  I2C0_IRQn               =  8,      /*!<  I2C0                               */
  PWMFault_IRQn           =  9,      /*!<  PWM Fault                          */
  PWMGen0_IRQn            = 10,      /*!<  PWM Generator 0                    */
  PWMGen1_IRQn            = 11,      /*!<  PWM Generator 1                    */
  PWMGen2_IRQn            = 12,      /*!<  PWM Generator 2                    */
  QEI0_IRQn               = 13,      /*!<  Quadrature Encoder 0               */
  ADCSeq0_IRQn            = 14,      /*!<  ADC Sequence 0                     */
  ADCSeq1_IRQn            = 15,      /*!<  ADC Sequence 1                     */
  ADCSeq2_IRQn            = 16,      /*!<  ADC Sequence 2                     */
  ADCSeq3_IRQn            = 17,      /*!<  ADC Sequence 3                     */
  Watchdog_IRQn           = 18,      /*!<  Watchdog                           */
  Timer0A_IRQn            = 19,      /*!<  Timer 0A                           */
  Timer0B_IRQn            = 20,      /*!<  Timer 0B                           */
  Timer1A_IRQn            = 21,      /*!<  Timer 1A                           */
  Timer1B_IRQn            = 22,      /*!<  Timer 1B                           */
  Timer2A_IRQn            = 23,      /*!<  Timer 2A                           */
  Timer2B_IRQn            = 24,      /*!<  Timer 2B                           */
  Comp0_IRQn              = 25,      /*!<  Comp 0                             */
  Comp1_IRQn              = 26,      /*!<  Comp 1                             */
  Comp2_IRQn              = 27,      /*!<  Comp 2                             */
  SysCtrl_IRQn            = 28,      /*!<  System Control                     */
  FlashCtrl_IRQn          = 29,      /*!<  Flash Control                      */
  GPIOPortF_IRQn          = 30,      /*!<  GPIO Port F                        */
  GPIOPortG_IRQn          = 31,      /*!<  GPIO Port G                        */
  GPIOPortH_IRQn          = 32,      /*!<  GPIO Port H                        */
  USART2_IRQn             = 33,      /*!<  UART2 Rx and Tx                    */
  SSI1_IRQn               = 34,      /*!<  SSI1 Rx and Tx                     */
  Timer3A_IRQn            = 35,      /*!<  Timer 3 subtimer A                 */
  Timer3B_IRQn            = 36,      /*!<  Timer 3 subtimer B                 */
  I2C1_IRQn               = 37,      /*!<  I2C1 Master and Slave              */
  QEI1_IRQn               = 38,      /*!<  Quadrature Encoder 1               */
  CAN0_IRQn               = 39,      /*!<  CAN0                               */
  CAN1_IRQn               = 40,      /*!<  CAN1                               */
  CAN2_IRQn               = 41,      /*!<  CAN2                               */
  Ethernet_IRQn           = 42,      /*!<  Ethernet                           */
  Hibernate_IRQn          = 43,      /*!<  Hibernate                          */
  USB0_IRQn               = 44,      /*!<  USB0                               */
  PWMGen3_IRQn            = 45,      /*!<  PWM Generator 3                    */
  uDMA_IRQn               = 46,      /*!<  uDMA Software Transfer             */
  uDMAErr_IRQn            = 47       /*!<  uDMA Error                         */
} IRQn_Type;


/* ========================================================================== */
/* =============      Processor and Core Peripheral Section     ============= */
/* ========================================================================== */

/* ----  Start of section using anonymous unions and disabling warnings  ---- */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

/* --------  Configuration of the Cortex-M3 Processor and Core Peripherals  ------- */
#define __CM3_REV                 0x0201U   /* Core revision r2p1 */
#define __MPU_PRESENT             1U        /* MPU present */
#define __VTOR_PRESENT            1U        /* VTOR present or not */
#define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */


#include "core/core_cm3.h"          /* Cortex-M3 processor and core periphs  */
#include "lm3s/system_lm3s.h"       /* LM3S Stellaris system init            */


/* ========================================================================== */
/* =============       Device Specific Peripheral Section       ============= */
/* ========================================================================== */

/*----------           System Controller (SYSCTL)                  -----------*/
// <g> System Controller (SYSCTL)
typedef struct {
  __I  uint32_t DID0;             // Device identification register 0
  __I  uint32_t DID1;             // Device identification register 1
  __I  uint32_t DC0;              // Device capabilities register 0
       uint8_t  RESERVED1[4];
  __I  uint32_t DC1;              // Device capabilities register 1
  __I  uint32_t DC2;              // Device capabilities register 2
  __I  uint32_t DC3;              // Device capabilities register 3
  __I  uint32_t DC4;              // Device capabilities register 4
  __I  uint32_t DC5;              // Device capabilities register 5
  __I  uint32_t DC6;              // Device capabilities register 6
  __I  uint32_t DC7;              // Device capabilities register 7
       uint8_t  RESERVED2[4];
  __IO uint32_t PBORCTL;          // POR/BOR reset control register
  __IO uint32_t LDOPCTL;          // LDO power control register
       uint8_t  RESERVED3[8];
  __IO uint32_t SRCR0;            // Software reset control reg 0
  __IO uint32_t SRCR1;            // Software reset control reg 1
  __IO uint32_t SRCR2;            // Software reset control reg 2
       uint8_t  RESERVED4[4];
  __I  uint32_t RIS;              // Raw interrupt status register
  __IO uint32_t IMC;              // Interrupt mask/control register
  __IO uint32_t MISC;             // Interrupt status register
  __IO uint32_t RESC;             // Reset cause register
  __IO uint32_t RCC;              // Run-mode clock config register
  __I  uint32_t PLLCFG;           // PLL configuration register
       uint8_t  RESERVED5[4];
  __IO uint32_t GPIOHSCTL;        // GPIO High Speed Control
  __IO uint32_t RCC2;             // Run-mode clock config register 2
       uint8_t  RESERVED6[8];
  __IO uint32_t MOSCCTL;          // Main Oscillator Control
       uint8_t  RESERVED7[128];
  __IO uint32_t RCGC0;            // Run-mode clock gating register 0
  __IO uint32_t RCGC1;            // Run-mode clock gating register 1
  __IO uint32_t RCGC2;            // Run-mode clock gating register 2
       uint8_t  RESERVED8[4];
  __IO uint32_t SCGC0;            // Sleep-mode clock gating reg 0
  __IO uint32_t SCGC1;            // Sleep-mode clock gating reg 1
  __IO uint32_t SCGC2;            // Sleep-mode clock gating reg 2
       uint8_t  RESERVED9[4];
  __IO uint32_t DCGC0;            // Deep Sleep-mode clock gate reg 0
  __IO uint32_t DCGC1;            // Deep Sleep-mode clock gate reg 1
  __IO uint32_t DCGC2;            // Deep Sleep-mode clock gate reg 2
       uint8_t  RESERVED10[24];
  __IO uint32_t DSLPCLKCFG;       // Deep Sleep-mode clock config reg
} SYSCTL_Type;
// </g>

/*----------        General Purpose Input/Output (GPIO)            -----------*/
// <g> General Purpose Input/Output (GPIO)
typedef struct {
  __IO uint32_t DATA_Bits[255];   // Bit specific data registers
  __IO uint32_t DATA;             // Data register.
  __IO uint32_t DIR;              // Data direction register.
  __IO uint32_t IS;               // Interrupt sense register.
  __IO uint32_t IBE;              // Interrupt both edges register.
  __IO uint32_t IEV;              // Interrupt event register.
  __IO uint32_t IM;               // Interrupt mask register.
  __I  uint32_t RIS;              // Raw interrupt status register.
  __I  uint32_t MIS;              // Masked interrupt status reg.
  __O  uint32_t ICR;              // Interrupt clear register.
  __IO uint32_t AFSEL;            // Mode control select register.
       uint8_t  RESERVED1[220];
  __IO uint32_t DR2R;             // 2ma drive select register.
  __IO uint32_t DR4R;             // 4ma drive select register.
  __IO uint32_t DR8R;             // 8ma drive select register.
  __IO uint32_t ODR;              // Open drain select register.
  __IO uint32_t PUR;              // Pull up select register.
  __IO uint32_t PDR;              // Pull down select register.
  __IO uint32_t SLR;              // Slew rate control enable reg.
  __IO uint32_t DEN;              // Digital input enable register.
  __IO uint32_t LOCK;             // Lock register.
  __O  uint32_t CR;               // Commit register.
  __IO uint32_t AMSEL;            // GPIO Analog Mode Select
} GPIO_Type;
// </g>

/*----------        General Purpose Timer (TIMER)                  -----------*/
// <g> General Purpose Timer (TIMER)
typedef struct {
  __IO uint32_t CFG;              // Configuration register
  __IO uint32_t TAMR;             // TimerA mode register
  __IO uint32_t TBMR;             // TimerB mode register
  __IO uint32_t CTL;              // Control register
       uint8_t  RESERVED0[8];
  __IO uint32_t IMR;              // Interrupt mask register
  __I  uint32_t RIS;              // Interrupt status register
  __I  uint32_t MIS;              // Masked interrupt status reg.
  __O  uint32_t ICR;              // Interrupt clear register
  __IO uint32_t TAILR;            // TimerA interval load register
  __IO uint32_t TBILR;            // TimerB interval load register
  __IO uint32_t TAMATCHR;         // TimerA match register
  __IO uint32_t TBMATCHR;         // TimerB match register
  __IO uint32_t TAPR;             // TimerA prescale register
  __IO uint32_t TBPR;             // TimerB prescale register
  __IO uint32_t TAPMR;            // TimerA prescale match register
  __IO uint32_t TBPMR;            // TimerB prescale match register
  __I  uint32_t TAR;              // TimerA register
  __I  uint32_t TBR;              // TimerB register
} TIMER_Type;
// </g>

/*----------        Analog to Digital Converter (ADC)              -----------*/
// <g> Analog to Digital Converter (ADC)
typedef struct {
  __IO uint32_t ACTSS;            // Active sample register
  __I  uint32_t RIS;              // Raw interrupt status register
  __IO uint32_t IM;               // Interrupt mask register
  __IO uint32_t ISC;              // Interrupt status/clear register
  __IO uint32_t OSTAT;            // Overflow status register
  __IO uint32_t EMUX;             // Event multiplexer select reg.
  __IO uint32_t USTAT;            // Underflow status register
       uint8_t  RESERVED0[4];
  __IO uint32_t SSPRI;            // Channel priority register
       uint8_t  RESERVED1[4];
  __O  uint32_t PSSI;             // Processor sample initiate reg.
       uint8_t  RESERVED2[4];
  __IO uint32_t SAC;              // Sample Averaging Control reg.
       uint8_t  RESERVED3[12];
  __IO uint32_t SSMUX0;           // Multiplexer select 0 register
  __IO uint32_t SSCTL0;           // Sample sequence control 0 reg.
  __I  uint32_t SSFIFO0;          // Result FIFO 0 register
  __I  uint32_t SSFSTAT0;         // FIFO 0 status register
       uint8_t  RESERVED4[16];
  __IO uint32_t SSMUX1;           // Multiplexer select 1 register
  __IO uint32_t SSCTL1;           // Sample sequence control 1 reg.
  __I  uint32_t SSFIFO1;          // Result FIFO 1 register
  __I  uint32_t SSFSTAT1;         // FIFO 1 status register
       uint8_t  RESERVED5[16];
  __IO uint32_t SSMUX2;           // Multiplexer select 2 register
  __IO uint32_t SSCTL2;           // Sample sequence control 2 reg.
  __I  uint32_t SSFIFO2;          // Result FIFO 2 register
  __I  uint32_t SSFSTAT2;         // FIFO 2 status register
       uint8_t  RESERVED6[16];
  __IO uint32_t SSMUX3;           // Multiplexer select 3 register
  __IO uint32_t SSCTL3;           // Sample sequence control 3 reg.
  __I  uint32_t SSFIFO3;          // Result FIFO 3 register
  __I  uint32_t SSFSTAT3;         // FIFO 3 status register
       uint8_t  RESERVED7[80];
  __IO uint32_t TMLB;             // Test mode loopback register
} ADC_Type;
// </g>

/*----------        Controller Area Network (CAN)                  -----------*/
// <g> Controller Area Network (CAN)
typedef struct {
  __IO uint32_t CTL;              // Control register
  __IO uint32_t STS;              // Status register
  __I  uint32_t ERR;              // Error register
  __IO uint32_t BIT;              // Bit Timing register
  __I  uint32_t INT;              // Interrupt register
  __IO uint32_t TST;              // Test register
  __IO uint32_t BRPE;             // Baud Rate Prescaler register
       uint8_t  RESERVED0[4];
  __IO uint32_t IF1CRQ;           // Interface 1 Command Request reg.
  __IO uint32_t IF1CMSK;          // Interface 1 Command Mask reg.
  __IO uint32_t IF1MSK1;          // Interface 1 Mask 1 register
  __IO uint32_t IF1MSK2;          // Interface 1 Mask 2 register
  __IO uint32_t IF1ARB1;          // Interface 1 Arbitration 1 reg.
  __IO uint32_t IF1ARB2;          // Interface 1 Arbitration 2 reg.
  __IO uint32_t IF1MCTL;          // Interface 1 Message Control reg.
  __IO uint32_t IF1DA1;           // Interface 1 DataA 1 register
  __IO uint32_t IF1DA2;           // Interface 1 DataA 2 register
  __IO uint32_t IF1DB1;           // Interface 1 DataB 1 register
  __IO uint32_t IF1DB2;           // Interface 1 DataB 2 register
       uint8_t  RESERVED1[52];
  __IO uint32_t IF2CRQ;           // Interface 2 Command Request reg.
  __IO uint32_t IF2CMSK;          // Interface 2 Command Mask reg.
  __IO uint32_t IF2MSK1;          // Interface 2 Mask 1 register
  __IO uint32_t IF2MSK2;          // Interface 2 Mask 2 register
  __IO uint32_t IF2ARB1;          // Interface 2 Arbitration 1 reg.
  __IO uint32_t IF2ARB2;          // Interface 2 Arbitration 2 reg.
  __IO uint32_t IF2MCTL;          // Interface 2 Message Control reg.
  __IO uint32_t IF2DA1;           // Interface 2 DataA 1 register
  __IO uint32_t IF2DA2;           // Interface 2 DataA 2 register
  __IO uint32_t IF2DB1;           // Interface 2 DataB 1 register
  __IO uint32_t IF2DB2;           // Interface 2 DataB 2 register
       uint8_t  RESERVED2[84];
  __I  uint32_t TXRQ1;            // Transmission Request 1 register
  __I  uint32_t TXRQ2;            // Transmission Request 2 register
       uint8_t  RESERVED3[24];
  __I  uint32_t NWDA1;            // New Data 1 register
  __I  uint32_t NWDA2;            // New Data 2 register
       uint8_t  RESERVED4[24];
  __I  uint32_t MSG1INT;          // CAN Message 1 Interrupt Pending
  __I  uint32_t MSG2INT;          // CAN Message 2 Interrupt Pending
       uint8_t  RESERVED5[24];
  __I  uint32_t MSG1VAL;          // CAN Message 1 Valid
  __I  uint32_t MSG2VAL;          // CAN Message 2 Valid
} CAN_Type;
// </g>

/*----------        Analog Comparators (COMP)                      -----------*/
// <g> Analog Comparators (COMP)
typedef struct {
  __IO uint32_t ACMIS;            // Analog Comparator Masked
                                  // Interrupt Status
  __I  uint32_t ACRIS;            // Analog Comparator Raw Interrupt Status
  __IO uint32_t ACINTEN;          // Analog Comparator Interrupt Enable
       uint8_t  RESERVED0[4];
  __IO uint32_t ACREFCTL;         // Analog Comparator Reference Voltage
                                  // Control
       uint8_t  RESERVED1[12];
  __I  uint32_t ACSTAT0;          // Comp0 status register
  __IO uint32_t ACCTL0;           // Comp0 control register
       uint8_t  RESERVED2[24];
  __I  uint32_t ACSTAT1;          // Comp1 status register
  __IO uint32_t ACCTL1;           // Comp1 control register
       uint8_t  RESERVED3[24];
  __I  uint32_t ACSTAT2;          // Comp2 status register
  __IO uint32_t ACCTL2;           // Comp2 control register
} COMP_Type;
// </g>

/*----------        Ethernet Controller (MAC)                      -----------*/
// <g> Ethernet Controller (MAC)
typedef struct {
  __IO uint32_t RIS;              // (__I) Ethernet MAC Raw Interrupt Status
#define IACK RIS                    // (__O)Interrupt Acknowledge Register
  __IO uint32_t IM;               // Interrupt Mask Register
  __IO uint32_t RCTL;             // Receive Control Register
  __IO uint32_t TCTL;             // Transmit Control Register
  __IO uint32_t DATA;             // Data Register
  __IO uint32_t IA0;              // Individual Address Register 0
  __IO uint32_t IA1;              // Individual Address Register 1
  __IO uint32_t THR;              // Threshold Register
  __IO uint32_t MCTL;             // Management Control Register
  __IO uint32_t MDV;              // Management Divider Register
       uint8_t  RESERVED1[4];
  __IO uint32_t MTXD;             // Management Transmit Data Reg
  __IO uint32_t MRXD;             // Management Receive Data Reg
  __I  uint32_t NP;               // Number of Packets Register
  __IO uint32_t TR;               // Transmission Request Register
  __IO uint32_t TS;               // Timer Support Register
} MAC_Type;
// </g>

/*----------        Flash Memory Controller (FLASH)                -----------*/
// <g> Flash Memory Controller (FLASH)
typedef struct {
  __IO uint32_t FMA;              // Memory address register
  __IO uint32_t FMD;              // Memory data register
  __IO uint32_t FMC;              // Memory control register
  __I  uint32_t FCRIS;            // Raw interrupt status register
  __IO uint32_t FCIM;             // Interrupt mask register
  __IO uint32_t FCMISC;           // Interrupt status register
       uint8_t  RESERVED1[4312];
  __IO uint32_t RMCTL;            // ROM Control
  __I  uint32_t RMVER;            // ROM Version Register
       uint8_t  RESERVED2[56];
  __IO uint32_t FMPRE;            // FLASH read protect register
  __IO uint32_t FMPPE;            // FLASH program protect register
       uint8_t  RESERVED3[8];
  __IO uint32_t USECRL;           // uSec reload register
       uint8_t  RESERVED4[140];
  __IO uint32_t USERDBG;          // User Debug
       uint8_t  RESERVED5[12];
  __IO uint32_t USERREG0;         // User Register 0
  __IO uint32_t USERREG1;         // User Register 1
  __IO uint32_t USERREG2;         // User Register 2
  __IO uint32_t USERREG3;         // User Register 3
       uint8_t  RESERVED6[16];
  __IO uint32_t FMPRE0;           // FLASH read protect register 0
  __IO uint32_t FMPRE1;           // FLASH read protect register 1
  __IO uint32_t FMPRE2;           // FLASH read protect register 2
  __IO uint32_t FMPRE3;           // FLASH read protect register 3
       uint8_t  RESERVED7[496];
  __IO uint32_t FMPPE0;           // FLASH program protect register 0
  __IO uint32_t FMPPE1;           // FLASH program protect register 1
  __IO uint32_t FMPPE2;           // FLASH program protect register 2
  __IO uint32_t FMPPE3;           // FLASH program protect register 3
} FLASH_Type;
// </g>

/*----------        Hibernation Module (HIB)                       -----------*/
// <g> Hibernation Module (HIB)
typedef struct {
  __I  uint32_t HIB_RTCC;         // Hibernate RTC counter
  __IO uint32_t HIB_RTCM0;        // Hibernate RTC match 0
  __IO uint32_t HIB_RTCM1;        // Hibernate RTC match 1
  __IO uint32_t HIB_RTCLD;        // Hibernate RTC load
  __IO uint32_t HIB_CTL;          // Hibernate RTC control
  __IO uint32_t HIB_IM;           // Hibernate interrupt mask
  __I  uint32_t HIB_RIS;          // Hibernate raw interrupt status
  __I  uint32_t HIB_MIS;          // Hibernate masked interrupt stat
  __IO uint32_t HIB_IC;           // Hibernate interrupt clear
  __IO uint32_t HIB_RTCT;         // Hibernate RTC trim
       uint8_t  RESERVED1[8];
  __IO uint32_t HIB_DATA[64];     // Hibernate data area
} HIB_Type;
// </g>

/*--------  Inter-Integrated Circuit Controller Master (I2C_MASTER) ----------*/
// <g> Inter-Integrated Circuit Controller Master (I2C_MASTER)
typedef struct {
  __IO uint32_t MSA;              // I2C Master Slave Address
  __IO uint32_t MCS;              // I2C Master Control/Status
  __IO uint32_t MDR;              // I2C Master Data
  __IO uint32_t MTPR;             // I2C Master Timer Period
  __IO uint32_t MIMR;             // I2C Master Interrupt Mask
  __I  uint32_t MRIS;             // I2C Master Raw Interrupt Status
  __I  uint32_t MMIS;             // I2C Master Masked Interrupt Status
  __O  uint32_t MICR;             // I2C Master Interrupt Clear
  __IO uint32_t MCR;              // I2C Master Configuration
} I2C_MASTER_Type;
// </g>

/*---------  Inter-Integrated Circuit Controller Slave (I2C_SLAVE) -----------*/
// <g> Inter-Integrated Circuit Controller Slave (I2C_SLAVE)
typedef struct {
  __IO uint32_t SOAR;             // I2C Slave Own Address
  __I  uint32_t SCSR;             // I2C Slave Control/Status
  __IO uint32_t SDR;              // I2C Slave Data
  __IO uint32_t SIMR;             // I2C Slave Interrupt Mask
  __I  uint32_t SRIS;             // I2C Slave Raw Interrupt Status
  __I  uint32_t SMIS;             // I2C Slave Masked Interrupt Status
  __O  uint32_t SICR;             // I2C Slave Interrupt Clear
} I2C_SLAVE_Type;
// </g>

/*----------        Pulse Width Modulation (PWM)                   -----------*/
// <g> Pulse Width Modulation (PWM)
typedef struct {
  __IO uint32_t CTL;              // PWM Master Control register
  __IO uint32_t SYNC;             // PWM Time Base Sync register
  __IO uint32_t ENABLE;           // PWM Output Enable register
  __IO uint32_t INVERT;           // PWM Output Inversion register
  __IO uint32_t FAULT;            // PWM Output Fault register
  __IO uint32_t INTEN;            // PWM Interrupt Enable register
  __I  uint32_t RIS;              // PWM Interrupt Raw Status reg.
  __IO uint32_t ISC;              // PWM Interrupt Status register
  __I  uint32_t STATUS;           // PWM Status register
  __IO uint32_t FAULTVAL;         // PWM Fault Condition Value
       uint8_t  RESERVED0[24];
  __IO uint32_t GEN0_CTL;            // PWM0 Control
  __IO uint32_t GEN0_INTEN;          // PWM0 Interrupt and Trigger Enable
  __I  uint32_t GEN0_RIS;            // PWM0 Raw Interrupt Status
  __IO uint32_t GEN0_ISC;            // PWM0 Interrupt Status and Clear
  __IO uint32_t GEN0_LOAD;           // PWM0 Load
  __I  uint32_t GEN0_COUNT;          // PWM0 Counter
  __IO uint32_t GEN0_CMPA;           // PWM0 Compare A
  __IO uint32_t GEN0_CMPB;           // PWM0 Compare B
  __IO uint32_t GEN0_GENA;           // PWM0 Generator A Control
  __IO uint32_t GEN0_GENB;           // PWM0 Generator B Control
  __IO uint32_t GEN0_DBCTL;          // PWM0 Dead-Band Control
  __IO uint32_t GEN0_DBRISE;         // PWM0 Dead-Band Rising-Edge Delay
  __IO uint32_t GEN0_DBFALL;         // PWM0 Dead-Band Falling-Edge-Delay
  __IO uint32_t GEN0_FLTSRC0;        // PWM0 Fault Source 0
       uint8_t  RESERVED1[4];
  __IO uint32_t GEN0_MINFLTPER;      // PWM0 Minimum Fault Period
  __IO uint32_t GEN1_CTL;            // PWM1 Control
  __IO uint32_t GEN1_INTEN;          // PWM1 Interrupt Enable
  __I  uint32_t GEN1_RIS;            // PWM1 Raw Interrupt Status
  __IO uint32_t GEN1_ISC;            // PWM1 Interrupt Status and Clear
  __IO uint32_t GEN1_LOAD;           // PWM1 Load
  __I  uint32_t GEN1_COUNT;          // PWM1 Counter
  __IO uint32_t GEN1_CMPA;           // PWM1 Compare A
  __IO uint32_t GEN1_CMPB;           // PWM1 Compare B
  __IO uint32_t GEN1_GENA;           // PWM1 Generator A Control
  __IO uint32_t GEN1_GENB;           // PWM1 Generator B Control
  __IO uint32_t GEN1_DBCTL;          // PWM1 Dead-Band Control
  __IO uint32_t GEN1_DBRISE;         // PWM1 Dead-Band Rising-Edge Delay
  __IO uint32_t GEN1_DBFALL;         // PWM1 Dead-Band Falling-Edge-Delay
  __IO uint32_t GEN1_FLTSRC0;        // PWM1 Fault Source 0
       uint8_t  RESERVED2[4];
  __IO uint32_t GEN1_MINFLTPER;      // PWM1 Minimum Fault Period
  __IO uint32_t GEN2_CTL;            // PWM2 Control
  __IO uint32_t GEN2_INTEN;          // PWM2 InterruptEnable
  __I  uint32_t GEN2_RIS;            // PWM2 Raw Interrupt Status
  __IO uint32_t GEN2_ISC;            // PWM2 Interrupt Status and Clear
  __IO uint32_t GEN2_LOAD;           // PWM2 Load
  __I  uint32_t GEN2_COUNT;          // PWM2 Counter
  __IO uint32_t GEN2_CMPA;           // PWM2 Compare A
  __IO uint32_t GEN2_CMPB;           // PWM2 Compare B
  __IO uint32_t GEN2_GENA;           // PWM2 Generator A Control
  __IO uint32_t GEN2_GENB;           // PWM2 Generator B Control
  __IO uint32_t GEN2_DBCTL;          // PWM2 Dead-Band Control
  __IO uint32_t GEN2_DBRISE;         // PWM2 Dead-Band Rising-Edge Delay
  __IO uint32_t GEN2_DBFALL;         // PWM2 Dead-Band Falling-Edge-Delay
  __IO uint32_t GEN2_FLTSRC0;        // PWM2 Fault Source 0
       uint8_t  RESERVED3[4];
  __IO uint32_t GEN2_MINFLTPER;      // PWM2 Minimum Fault Period
  __IO uint32_t GEN3_CTL;            // PWM3 Control
  __IO uint32_t GEN3_INTEN;          // PWM3 Interrupt and Trigger Enable
  __I  uint32_t GEN3_RIS;            // PWM3 Raw Interrupt Status
  __IO uint32_t GEN3_ISC;            // PWM3 Interrupt Status and Clear
  __IO uint32_t GEN3_LOAD;           // PWM3 Load
  __I  uint32_t GEN3_COUNT;          // PWM3 Counter
  __IO uint32_t GEN3_CMPA;           // PWM3 Compare A
  __IO uint32_t GEN3_CMPB;           // PWM3 Compare B
  __IO uint32_t GEN3_GENA;           // PWM3 Generator A Control
  __IO uint32_t GEN3_GENB;           // PWM3 Generator B Control
  __IO uint32_t GEN3_DBCTL;          // PWM3 Dead-Band Control
  __IO uint32_t GEN3_DBRISE;         // PWM3 Dead-Band Rising-Edge Delay
  __IO uint32_t GEN3_DBFALL;         // PWM3 Dead-Band Falling-Edge-Delay
  __IO uint32_t GEN3_FLTSRC0;        // PWM3 Fault Source 0
       uint8_t  RESERVED4[4];
  __IO uint32_t GEN3_MINFLTPER;      // PWM3 Minimum Fault Period
       uint8_t  RESERVED5[1728];
  __IO uint32_t GEN0_FLTSEN;         // PWM0 Fault Pin Logic Sense
  __I  uint32_t GEN0_FLTSTAT0;       // PWM0 Fault Status 0
       uint8_t  RESERVED6[120];
  __IO uint32_t GEN1_FLTSEN;         // PWM1 Fault Pin Logic Sense
  __I  uint32_t GEN1_FLTSTAT0;       // PWM1 Fault Status 0
       uint8_t  RESERVED7[120];
  __IO uint32_t GEN2_FLTSEN;         // PWM2 Fault Pin Logic Sense
  __I  uint32_t GEN2_FLTSTAT0;       // PWM2 Fault Status 0
       uint8_t  RESERVED8[120];
  __IO uint32_t GEN3_FLTSEN;         // PWM3 Fault Pin Logic Sense
  __I  uint32_t GEN3_FLTSTAT0;       // PWM3 Fault Status 0
} PWM_Type;
// </g>

/*----------        Pulse Width Modulation Generator (PWMGEN)      -----------*/
// <g> Pulse Width Modulation Generator (PWMGEN)
typedef struct {
  __IO uint32_t CTL;                 // PWM0 Control
  __IO uint32_t INTEN;               // PWM0 Interrupt and Trigger Enable
  __I  uint32_t RIS;                 // PWM0 Raw Interrupt Status
  __IO uint32_t ISC;                 // PWM0 Interrupt Status and Clear
  __IO uint32_t LOAD;                // PWM0 Load
  __I  uint32_t COUNT;               // PWM0 Counter
  __IO uint32_t CMPA;                // PWM0 Compare A
  __IO uint32_t CMPB;                // PWM0 Compare B
  __IO uint32_t GENA;                // PWM0 Generator A Control
  __IO uint32_t GENB;                // PWM0 Generator B Control
  __IO uint32_t DBCTL;               // PWM0 Dead-Band Control
  __IO uint32_t DBRISE;              // PWM0 Dead-Band Rising-Edge Delay
  __IO uint32_t DBFALL;              // PWM0 Dead-Band Falling-Edge-Delay
  __IO uint32_t FLTSRC0;             // PWM0 Fault Source 0
       uint8_t  RESERVED1[4];
  __IO uint32_t MINFLTPER;           // PWM0 Minimum Fault Period
} PWMGEN_Type;
// </g>

/*----------        Quadrature Encoded Input (QEI)                 -----------*/
// <g> Quadrature Encoded Input (QEI)
typedef struct {
  __IO uint32_t CTL;                  // Configuration and control reg.
  __I  uint32_t STAT;                 // Status register
  __IO uint32_t POS;                  // Current position register
  __IO uint32_t MAXPOS;               // Maximum position register
  __IO uint32_t LOAD;                 // Velocity timer load register
  __I  uint32_t TIME;                 // Velocity timer register
  __I  uint32_t COUNT;                // Velocity pulse count register
  __I  uint32_t SPEED;                // Velocity speed register
  __IO uint32_t INTEN;                // Interrupt enable register
  __I  uint32_t RIS;                  // Raw interrupt status register
  __IO uint32_t ISC;                  // Interrupt status register
} QEI_Type;
// </g>

/*----------        Synchronous Serial Interface (SSI)             -----------*/
// <g> Synchronous Serial Interface (SSI)
typedef struct {
  __IO uint32_t CR0;              // Control register 0
  __IO uint32_t CR1;              // Control register 1
  __IO uint32_t DR;               // Data register
  __I  uint32_t SR;               // Status register
  __IO uint32_t CPSR;             // Clock prescale register
  __IO uint32_t IM;               // Int mask set and clear register
  __I  uint32_t RIS;              // Raw interrupt register
  __I  uint32_t MIS;              // Masked interrupt register
  __O  uint32_t ICR;              // Interrupt clear register
  __IO uint32_t DMACTL;           // SSI DMA Control
} SSI_Type;
// </g>

/*----------        Asynchronous Serial (UART)                     -----------*/
// <g> Asynchronous Serial (UART)
typedef struct {
  __IO uint32_t DR;               // Data Register
  __IO uint32_t RSR;              // Receive Status Register (read)
#define ECR RSR                     // Error Clear Register (write)
       uint8_t  RESERVED1[16];
  __I  uint32_t FR;               // Flag Register (read only)
       uint8_t  RESERVED2[4];
  __IO uint32_t ILPR;             // UART IrDA Low-Power Register
  __IO uint32_t IBRD;             // Integer Baud Rate Divisor Reg
  __IO uint32_t FBRD;             // Fractional Baud Rate Divisor Reg
  __IO uint32_t LCRH;             // UART Line Control
  __IO uint32_t CTL;              // Control Register
  __IO uint32_t IFLS;             // Interrupt FIFO Level Select Reg
  __IO uint32_t IM;               // Interrupt Mask Set/Clear Reg
  __I  uint32_t RIS;              // Raw Interrupt Status Register
  __I  uint32_t MIS;              // Masked Interrupt Status Register
  __O  uint32_t ICR;              // Interrupt Clear Register
  __IO uint32_t DMACTL;           // UART DMA Control
} UART_Type;
// </g>

/*----------        DMA Controller (UDMA)                          -----------*/
// <g> DMA Controller (UDMA)
typedef struct {
  __I  uint32_t STAT;             // DMA Status
  __O  uint32_t CFG;              // DMA Configuration
  __IO uint32_t CTLBASE;          // DMA Channel Control Base Pointer
  __I  uint32_t ALTBASE;          // DMA Alternate Channel Control Base
                                  // Pointer
  __I  uint32_t WAITSTAT;         // DMA Channel Wait on Request Status
  __O  uint32_t SWREQ;            // DMA Channel Software Request
  __IO uint32_t USEBURSTSET;      // DMA Channel Useburst Set
  __O  uint32_t USEBURSTCLR;      // DMA Channel Useburst Clear
  __IO uint32_t REQMASKSET;       // DMA Channel Request Mask Set
  __O  uint32_t REQMASKCLR;       // DMA Channel Request Mask Clear
  __IO uint32_t ENASET;           // DMA Channel Enable Set
  __O  uint32_t ENACLR;           // DMA Channel Enable Clear
  __IO uint32_t ALTSET;           // DMA Channel Primary Alternate Set
  __O  uint32_t ALTCLR;           // DMA Channel Primary Alternate Clear
  __IO uint32_t PRIOSET;          // DMA Channel Priority Set
  __O  uint32_t PRIOCLR;          // DMA Channel Priority Clear
       uint8_t  RESERVED1[12];
  __IO uint32_t ERRCLR;           // DMA Bus Error Clear
} UDMA_Type;
// </g>

/*----------        DMA Channel Control Structure (UDMA_CTRL)      -----------*/
// <g> DMA Channel Control Structure (UDMA_CTRL)
typedef struct {
  __IO uint32_t SRCENDP;          // DMA Channel Source Address End Pointer
  __IO uint32_t DSTENDP;          // DMA Channel Destination Address End Pointer
  __IO uint32_t CHCTL;            // DMA Channel Control Word
} UDMA_CTRL_Type;
// </g>

/*----------        Universal Serial Bus Controller (USB)          -----------*/
// <g> Universal Serial Bus Controller (USB)
typedef struct {
  __IO uint8_t  FADDR;            // USB Device Functional Address
  __IO uint8_t  POWER;            // USB Power
  __I  uint16_t TXIS;             // USB Transmit Interrupt Status
  __I  uint16_t RXIS;             // USB Receive Interrupt Status
  __IO uint16_t TXIE;             // USB Transmit Interrupt Enable
  __IO uint16_t RXIE;             // USB Receive Interrupt Enable
  __I  uint8_t  IS;               // USB General Interrupt Status
  __IO uint8_t  IE;               // USB Interrupt Enable
  __I  uint16_t FRAME;            // USB Frame Value
  __IO uint8_t  EPIDX;            // USB Endpoint Index
  __IO uint8_t  TEST;             // USB Test Mode
       uint8_t  RESERVED1[16];
  __IO uint32_t FIFO0;            // USB FIFO Endpoint 0
  __IO uint32_t FIFO1;            // USB FIFO Endpoint 1
  __IO uint32_t FIFO2;            // USB FIFO Endpoint 2
  __IO uint32_t FIFO3;            // USB FIFO Endpoint 3
       uint8_t  RESERVED2[48];
  __IO uint8_t  DEVCTL;           // USB Device Control
       uint8_t  RESERVED3[1];
  __IO uint8_t  TXFIFOSZ;         // USB Transmit Dynamic FIFO Sizing
  __IO uint8_t  RXFIFOSZ;         // USB Receive Dynamic FIFO Sizing
  __IO uint16_t TXFIFOADD;        // USB Transmit FIFO Start Address
  __IO uint16_t RXFIFOADD;        // USB Receive FIFO Start Address
       uint8_t  RESERVED4[18];
  __IO uint8_t  CONTIM;           // USB Connect Timing
  __IO uint8_t  VPLEN;            // USB OTG VBus Pulse Timing
       uint8_t  RESERVED5[1];
  __IO uint8_t  FSEOF;            // USB Full-Speed Last Transaction to End
                                  // of Frame Timing
  __IO uint8_t  LSEOF;            // USB Low-Speed Last Transaction to End of
                                  // Frame Timing
       uint8_t  RESERVED6[1];
  __IO uint8_t  TXFUNCADDR0;      // USB Transmit Functional Address
                                  // Endpoint 0
       uint8_t  RESERVED7[1];
  __IO uint8_t  TXHUBADDR0;       // USB Transmit Hub Address Endpoint 0
  __IO uint8_t  TXHUBPORT0;       // USB Transmit Hub Port Endpoint 0
       uint8_t  RESERVED8[4];
  __IO uint8_t  TXFUNCADDR1;      // USB Transmit Functional Address
                                  // Endpoint 1
       uint8_t  RESERVED9[1];
  __IO uint8_t  TXHUBADDR1;       // USB Transmit Hub Address Endpoint 1
  __IO uint8_t  TXHUBPORT1;       // USB Transmit Hub Port Endpoint 1
  __IO uint8_t  RXFUNCADDR1;      // USB Receive Functional Address
                                  // Endpoint 1
       uint8_t  RESERVED10[1];
  __IO uint8_t  RXHUBADDR1;       // USB Receive Hub Address Endpoint 1
  __IO uint8_t  RXHUBPORT1;       // USB Receive Hub Port Endpoint 1
  __IO uint8_t  TXFUNCADDR2;      // USB Transmit Functional Address
                                  // Endpoint 2
       uint8_t  RESERVED11[1];
  __IO uint8_t  TXHUBADDR2;       // USB Transmit Hub Address Endpoint 2
  __IO uint8_t  TXHUBPORT2;       // USB Transmit Hub Port Endpoint 2
  __IO uint8_t  RXFUNCADDR2;      // USB Receive Functional Address
                                  // Endpoint 2
       uint8_t  RESERVED12[1];
  __IO uint8_t  RXHUBADDR2;       // USB Receive Hub Address Endpoint 2
  __IO uint8_t  RXHUBPORT2;       // USB Receive Hub Port Endpoint 2
  __IO uint8_t  TXFUNCADDR3;      // USB Transmit Functional Address
                                  // Endpoint 3
       uint8_t  RESERVED13[1];
  __IO uint8_t  TXHUBADDR3;       // USB Transmit Hub Address Endpoint 3
  __IO uint8_t  TXHUBPORT3;       // USB Transmit Hub Port Endpoint 3
  __IO uint8_t  RXFUNCADDR3;      // USB Receive Functional Address
                                  // Endpoint 3
       uint8_t  RESERVED14[1];
  __IO uint8_t  RXHUBADDR3;       // USB Receive Hub Address Endpoint 3
  __IO uint8_t  RXHUBPORT3;       // USB Receive Hub Port Endpoint 3
       uint8_t  RESERVED15[98];
  __O  uint8_t  CSRL0;            // USB Control and Status Endpoint 0 Low
  __O  uint8_t  CSRH0;            // USB Control and Status Endpoint 0 High
       uint8_t  RESERVED16[4];
  __I  uint8_t  COUNT0;           // USB Receive Byte Count Endpoint 0
       uint8_t  RESERVED17[1];
  __IO uint8_t  TYPE0;            // USB Type Endpoint 0
  __IO uint8_t  NAKLMT;           // USB NAK Limit
       uint8_t  RESERVED18[4];
  __IO uint16_t TXMAXP1;          // USB Maximum Transmit Data Endpoint 1
  __IO uint8_t  TXCSRL1;          // USB Transmit Control and Status
                                  // Endpoint 1 Low
  __IO uint8_t  TXCSRH1;          // USB Transmit Control and Status
                                  // Endpoint 1 High
  __IO uint16_t RXMAXP1;          // USB Maximum Receive Data Endpoint 1
  __IO uint8_t  RXCSRL1;          // USB Receive Control and Status
                                  // Endpoint 1 Low
  __IO uint8_t  RXCSRH1;          // USB Receive Control and Status
                                  // Endpoint 1 High
  __I  uint16_t RXCOUNT1;         // USB Receive Byte Count Endpoint 1
  __IO uint8_t  TXTYPE1;          // USB Host Transmit Configure Type
                                  // Endpoint 1
  __IO uint8_t  TXINTERVAL1;      // USB Host Transmit Interval Endpoint 1
  __IO uint8_t  RXTYPE1;          // USB Host Configure Receive Type
                                  // Endpoint 1
  __IO uint8_t  RXINTERVAL1;      // USB Host Receive Polling Interval
                                  // Endpoint 1
       uint8_t  RESERVED19[2];
  __IO uint16_t TXMAXP2;          // USB Maximum Transmit Data Endpoint 2
  __IO uint8_t  TXCSRL2;          // USB Transmit Control and Status
                                  // Endpoint 2 Low
  __IO uint8_t  TXCSRH2;          // USB Transmit Control and Status
                                  // Endpoint 2 High
  __IO uint16_t RXMAXP2;          // USB Maximum Receive Data Endpoint 2
  __IO uint8_t  RXCSRL2;          // USB Receive Control and Status
                                  // Endpoint 2 Low
  __IO uint8_t  RXCSRH2;          // USB Receive Control and Status
                                  // Endpoint 2 High
  __I  uint16_t RXCOUNT2;         // USB Receive Byte Count Endpoint 2
  __IO uint8_t  TXTYPE2;          // USB Host Transmit Configure Type
                                  // Endpoint 2
  __IO uint8_t  TXINTERVAL2;      // USB Host Transmit Interval Endpoint 2
  __IO uint8_t  RXTYPE2;          // USB Host Configure Receive Type
                                  // Endpoint 2
  __IO uint8_t  RXINTERVAL2;      // USB Host Receive Polling Interval
                                  // Endpoint 2
       uint8_t  RESERVED20[2];
  __IO uint16_t TXMAXP3;          // USB Maximum Transmit Data Endpoint 3
  __IO uint8_t  TXCSRL3;          // USB Transmit Control and Status
                                  // Endpoint 3 Low
  __IO uint8_t  TXCSRH3;          // USB Transmit Control and Status
                                  // Endpoint 3 High
  __IO uint16_t RXMAXP3;          // USB Maximum Receive Data Endpoint 3
  __IO uint8_t  RXCSRL3;          // USB Receive Control and Status
                                  // Endpoint 3 Low
  __IO uint8_t  RXCSRH3;          // USB Receive Control and Status
                                  // Endpoint 3 High
  __I  uint16_t RXCOUNT3;         // USB Receive Byte Count Endpoint 3
  __IO uint8_t  TXTYPE3;          // USB Host Transmit Configure Type
                                  // Endpoint 3
  __IO uint8_t  TXINTERVAL3;      // USB Host Transmit Interval Endpoint 3
  __IO uint8_t  RXTYPE3;          // USB Host Configure Receive Type
                                  // Endpoint 3
  __IO uint8_t  RXINTERVAL3;      // USB Host Receive Polling Interval
                                  // Endpoint 3
       uint8_t  RESERVED21[454];
  __IO uint16_t RQPKTCOUNT1;      // USB Request Packet Count in Block
                                  // Transfer Endpoint 1
       uint8_t  RESERVED22[2];
  __IO uint16_t RQPKTCOUNT2;      // USB Request Packet Count in Block
                                  // Transfer Endpoint 2
       uint8_t  RESERVED23[2];
  __IO uint16_t RQPKTCOUNT3;      // USB Request Packet Count in Block
                                  // Transfer Endpoint 3
       uint8_t  RESERVED24[50];
  __IO uint16_t RXDPKTBUFDIS;     // USB Receive Double Packet Buffer Disable
  __IO uint16_t TXDPKTBUFDIS;     // USB Transmit Double Packet Buffer
                                  // Disable
       uint8_t  RESERVED25[188];
  __IO uint32_t EPC;              // USB External Power Control
  __I  uint32_t EPCRIS;           // USB External Power Control Raw Interrupt Status
  __IO uint32_t EPCIM;            // USB External Power Control Interrupt
                                  // Mask
  __IO uint32_t EPCISC;           // USB External Power Control Interrupt
                                  // Status and Clear
  __I  uint32_t DRRIS;            // USB Device Resume Raw Interrupt Status
  __IO uint32_t DRIM;             // USB Device Resume Interrupt Mask
  __O  uint32_t DRISC;            // USB Device Resume Interrupt Status and
                                  // Clear
  __IO uint32_t GPCS;             // USB General-Purpose Control and Status
} USB_Type;
// </g>

/*----------        Watchdog Timer (WDT)                           -----------*/
// <g> Watchdog Timer (WDT)
typedef struct {
  __IO uint32_t LOAD;             // Load register
  __I  uint32_t VALUE;            // Current value register
  __IO uint32_t CTL;              // Control register
  __O  uint32_t ICR;              // Interrupt clear register
  __I  uint32_t RIS;              // Raw interrupt status register
  __I  uint32_t MIS;              // Masked interrupt status register
       uint8_t  RESERVED0[1024];
  __IO uint32_t TEST;             // Test register
       uint8_t  RESERVED1[2020];
  __IO uint32_t LOCK;             // Lock register
} WDT_Type;
// </g>

/* -----  End of section using anonymous unions and disabling warnings  ----- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

/* ========================================================================== */
/* =============              Peripheral memory map             ============= */
/* ========================================================================== */

#define ARM_FLASH_BASE          (0x00000000UL)
#define ARM_RAM_BASE            (0x20000000UL)
#define ARM_PERIPH_BASE         (0x40000000UL)

#define WATCHDOG_BASE           (0x40000000UL)  // Watchdog
#define GPIO_PORTA_BASE         (0x40004000UL)  // GPIO Port A
#define GPIO_PORTB_BASE         (0x40005000UL)  // GPIO Port B
#define GPIO_PORTC_BASE         (0x40006000UL)  // GPIO Port C
#define GPIO_PORTD_BASE         (0x40007000UL)  // GPIO Port D
#define SSI0_BASE               (0x40008000UL)  // SSI0
#define SSI1_BASE               (0x40009000UL)  // SSI1
#define UART0_BASE              (0x4000C000UL)  // UART0
#define UART1_BASE              (0x4000D000UL)  // UART1
#define UART2_BASE              (0x4000E000UL)  // UART2
#define I2C0_MASTER_BASE        (0x40020000UL)  // I2C0 Master
#define I2C0_SLAVE_BASE         (0x40020800UL)  // I2C0 Slave
#define I2C1_MASTER_BASE        (0x40021000UL)  // I2C1 Master
#define I2C1_SLAVE_BASE         (0x40021800UL)  // I2C1 Slave
#define GPIO_PORTE_BASE         (0x40024000UL)  // GPIO Port E
#define GPIO_PORTF_BASE         (0x40025000UL)  // GPIO Port F
#define GPIO_PORTG_BASE         (0x40026000UL)  // GPIO Port G
#define GPIO_PORTH_BASE         (0x40027000UL)  // GPIO Port H
#define PWM_BASE                (0x40028000UL)  // PWM
#define PWM_GEN_0_OFFSET        (0x00000040UL)  // PWM0 base
#define PWM_GEN_1_OFFSET        (0x00000080UL)  // PWM1 base
#define PWM_GEN_2_OFFSET        (0x000000C0UL)  // PWM2 base
#define PWM_GEN_3_OFFSET        (0x00000100UL)  // PWM3 base
#define QEI0_BASE               (0x4002C000UL)  // QEI0
#define QEI1_BASE               (0x4002D000UL)  // QEI1
#define TIMER0_BASE             (0x40030000UL)  // Timer0
#define TIMER1_BASE             (0x40031000UL)  // Timer1
#define TIMER2_BASE             (0x40032000UL)  // Timer2
#define TIMER3_BASE             (0x40033000UL)  // Timer3
#define ADC_BASE                (0x40038000UL)  // ADC
#define COMP_BASE               (0x4003C000UL)  // Analog comparators
#define CAN0_BASE               (0x40040000UL)  // CAN0
#define CAN1_BASE               (0x40041000UL)  // CAN1
#define CAN2_BASE               (0x40042000UL)  // CAN2
#define ETH_BASE                (0x40048000UL)  // Ethernet
#define MAC_BASE                (0x40048000UL)  // Ethernet
#define USB0_BASE               (0x40050000UL)  // USB 0 Controller
#define GPIO_PORTA_AHB_BASE     (0x40058000UL)  // GPIO Port A (high speed)
#define GPIO_PORTB_AHB_BASE     (0x40059000UL)  // GPIO Port B (high speed)
#define GPIO_PORTC_AHB_BASE     (0x4005A000UL)  // GPIO Port C (high speed)
#define GPIO_PORTD_AHB_BASE     (0x4005B000UL)  // GPIO Port D (high speed)
#define GPIO_PORTE_AHB_BASE     (0x4005C000UL)  // GPIO Port E (high speed)
#define GPIO_PORTF_AHB_BASE     (0x4005D000UL)  // GPIO Port F (high speed)
#define GPIO_PORTG_AHB_BASE     (0x4005E000UL)  // GPIO Port G (high speed)
#define GPIO_PORTH_AHB_BASE     (0x4005F000UL)  // GPIO Port H (high speed)
#define HIB_BASE                (0x400FC000UL)  // Hibernation Module
#define FLASH_CTRL_BASE         (0x400FD000UL)  // FLASH Controller
#define SYSCTL_BASE             (0x400FE000UL)  // System Control
#define UDMA_BASE               (0x400FF000UL)  // uDMA Controller


/* ========================================================================== */
/* =============             Peripheral declaration             ============= */
/* ========================================================================== */
#define SYSCTL              ((SYSCTL_Type *)SYSCTL_BASE)

#define GPIOA               ((GPIO_Type *)GPIO_PORTA_BASE)
#define GPIOB               ((GPIO_Type *)GPIO_PORTB_BASE)
#define GPIOC               ((GPIO_Type *)GPIO_PORTC_BASE)
#define GPIOD               ((GPIO_Type *)GPIO_PORTD_BASE)
#define GPIOE               ((GPIO_Type *)GPIO_PORTE_BASE)
#define GPIOF               ((GPIO_Type *)GPIO_PORTF_BASE)
#define GPIOG               ((GPIO_Type *)GPIO_PORTG_BASE)
#define GPIOA_HS            ((GPIO_Type *)GPIO_PORTA_AHB_BASE)
#define GPIOB_HS            ((GPIO_Type *)GPIO_PORTB_AHB_BASE)
#define GPIOC_HS            ((GPIO_Type *)GPIO_PORTC_AHB_BASE)
#define GPIOD_HS            ((GPIO_Type *)GPIO_PORTD_AHB_BASE)
#define GPIOE_HS            ((GPIO_Type *)GPIO_PORTE_AHB_BASE)
#define GPIOF_HS            ((GPIO_Type *)GPIO_PORTF_AHB_BASE)
#define GPIOG_HS            ((GPIO_Type *)GPIO_PORTG_AHB_BASE)

#define TIMER0              ((TIMER_Type *)TIMER0_BASE)
#define TIMER1              ((TIMER_Type *)TIMER1_BASE)
#define TIMER2              ((TIMER_Type *)TIMER2_BASE)
#define TIMER3              ((TIMER_Type *)TIMER3_BASE)

#define ADC                 ((ADC_Type *)ADC_BASE)

#define COMP                ((COMP_Type *)COMP_BASE)

#define CAN0                ((CAN_Type *)CAN0_BASE)
#define CAN1                ((CAN_Type *)CAN1_BASE)
#define CAN2                ((CAN_Type *)CAN2_BASE)

#define ETH                 ((MAC_Type *)ETH_BASE)

#define FLASH_CTRL          ((FLASH_Type *)FLASH_CTRL_BASE)

#define HIB                 ((HIB_Type *)HIB_BASE)

#define I2C0_MASTER         ((I2C_MASTER_Type *)I2C0_MASTER_BASE)
#define I2C0_SLAVE          ((I2C_SLAVE_Type *)I2C0_SLAVE_BASE)
#define I2C1_MASTER         ((I2C_MASTER_Type *)I2C1_MASTER_BASE)
#define I2C1_SLAVE          ((I2C_SLAVE_Type *)I2C1_SLAVE_BASE)

#define PWM                 ((PWM_Type *)PWM_BASE)
#define PWMGEN0             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_0_OFFSET))
#define PWMGEN1             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_1_OFFSET))
#define PWMGEN2             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_2_OFFSET))
#define PWMGEN3             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_3_OFFSET))

#define QEI0                ((QEI_Type *)QEI0_BASE)
#define QEI1                ((QEI_Type *)QEI1_BASE)

#define SSI0                ((SSI_Type *)SSI0_BASE)
#define SSI1                ((SSI_Type *)SSI1_BASE)

#define UART0               ((UART_Type *)UART0_BASE)
#define UART1               ((UART_Type *)UART1_BASE)
#define UART2               ((UART_Type *)UART2_BASE)

#define UDMA                ((UDMA_Type *)UDMA_BASE)

#define USB0                ((USB_Type *)USB0_BASE)

#define WDT                 ((WDT_Type *)WATCHDOG_BASE)

#ifdef __cplusplus
}
#endif

//#endif  /* __LM3_CMSIS_H__ */
#endif  /* ARMCM3_H */
