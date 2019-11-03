/**************************************************************************//**
 * @file     startup_ARMCM3.s
 * @brief    CMSIS Core Device Startup File for
 *           ARMCM3 Device Series
 * @version  V5.00
 * @date     26. April 2016
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

#include <stdint.h>


/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __copy_table_start__;
extern uint32_t __copy_table_end__;
extern uint32_t __zero_table_start__;
extern uint32_t __zero_table_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackTop;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );


/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
#ifndef __START
extern void  _start(void) __attribute__((noreturn));    /* PreeMain (C library entry point) */
#else
extern int  __START(void) __attribute__((noreturn));    /* main entry point */
#endif

#ifndef __NO_SYSTEM_INIT
extern void SystemInit (void);            /* CMSIS System Initialization      */
#endif


/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Default_Handler(void);                          /* Default empty handler */
void Reset_Handler(void);                            /* Reset Handler */


/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
// Those should really be called "min" values:
// In linker file, the start of heap is specified after the RAM variables,
// while the start of stack is set at the end of the RAM memory (growing downward).
// This leaves whatever is free in RAM for both the heap and stack.
// Defining a .stack and .heap regions will reserve those in memory,
// but they are not monitored if they grow out of this allocated space.
#ifndef __STACK_SIZE
  #define	__STACK_SIZE  0x00000400
#endif
#if __STACK_SIZE > 0
static uint8_t stack[__STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));
#endif

#ifndef __HEAP_SIZE
  #define	__HEAP_SIZE   0x00010000
#endif
#if __HEAP_SIZE > 0
static uint8_t heap[__HEAP_SIZE]   __attribute__ ((aligned(8), used, section(".heap")));
#endif


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Cortex-M3 Processor Exceptions */
void NMI_Handler           (void) __attribute__ ((weak));
void HardFault_Handler     (void) __attribute__ ((weak));
void MemManage_Handler     (void) __attribute__ ((weak));
void BusFault_Handler      (void) __attribute__ ((weak));
void UsageFault_Handler    (void) __attribute__ ((weak));

/* Cortex-M Processor non-fault exceptions... */
void SVC_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));

/* external interrupts...   */
void GPIOPortA_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortB_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortC_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortD_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortE_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMFault_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Watchdog_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer0A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer0B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer1A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer1B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer2A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer2B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp2_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SysCtrl_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void FlashCtrl_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortF_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortG_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortH_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer3A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer3B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Ethernet_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void Hibernate_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));


/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
const pFunc __Vectors[] __attribute__ ((section(".vectors"))) = {
  /* Cortex-M3 Exceptions Handler */
  (pFunc)((uint32_t)&__StackTop),          /* Initial Stack Pointer           */
  (pFunc)Reset_Handler,                    /* Reset Handler                   */
  (pFunc)&NMI_Handler,                     /* NMI Handler                     */
  (pFunc)&HardFault_Handler,               /* Hard Fault Handler              */
  (pFunc)&MemManage_Handler,               /* The MPU fault handler           */
  (pFunc)&BusFault_Handler,                /* The bus fault handler           */
  (pFunc)&UsageFault_Handler,              /* The usage fault handler         */
  0,                                       /* Reserved                        */
  0,                                       /* Reserved                        */
  0,                                       /* Reserved                        */
  0,                                       /* Reserved                        */
  (pFunc)&SVC_Handler,                     /* SVCall handler                  */
  (pFunc)&DebugMon_Handler,                /* Debug monitor handler           */
  0,                                       /* Reserved                        */
  (pFunc)&PendSV_Handler,                  /* The PendSV handler              */
  (pFunc)&SysTick_Handler,                 /* The SysTick handler             */

  /*IRQ handlers... */
  (pFunc)&GPIOPortA_IRQHandler,            /* GPIO Port A                     */
  (pFunc)&GPIOPortB_IRQHandler,            /* GPIO Port B                     */
  (pFunc)&GPIOPortC_IRQHandler,            /* GPIO Port C                     */
  (pFunc)&GPIOPortD_IRQHandler,            /* GPIO Port D                     */
  (pFunc)&GPIOPortE_IRQHandler,            /* GPIO Port E                     */
  (pFunc)&UART0_IRQHandler,                /* UART0 Rx and Tx                 */
  (pFunc)&UART1_IRQHandler,                /* UART1 Rx and Tx                 */
  (pFunc)&SSI0_IRQHandler,                 /* SSI0 Rx and Tx                  */
  (pFunc)&I2C0_IRQHandler,                 /* I2C0 Master and Slave           */
  (pFunc)&PWMFault_IRQHandler,             /* PWM Fault                       */
  (pFunc)&PWMGen0_IRQHandler,              /* PWM Generator 0                 */
  (pFunc)&PWMGen1_IRQHandler,              /* PWM Generator 1                 */
  (pFunc)&PWMGen2_IRQHandler,              /* PWM Generator 2                 */
  (pFunc)&QEI0_IRQHandler,                 /* Quadrature Encoder 0            */
  (pFunc)&ADCSeq0_IRQHandler,              /* ADC Sequence 0                  */
  (pFunc)&ADCSeq1_IRQHandler,              /* ADC Sequence 1                  */
  (pFunc)&ADCSeq2_IRQHandler,              /* ADC Sequence 2                  */
  (pFunc)&ADCSeq3_IRQHandler,              /* ADC Sequence 3                  */
  (pFunc)&Watchdog_IRQHandler,             /* Watchdog timer                  */
  (pFunc)&Timer0A_IRQHandler,              /* Timer 0 subtimer A              */
  (pFunc)&Timer0B_IRQHandler,              /* Timer 0 subtimer B              */
  (pFunc)&Timer1A_IRQHandler,              /* Timer 1 subtimer A              */
  (pFunc)&Timer1B_IRQHandler,              /* Timer 1 subtimer B              */
  (pFunc)&Timer2A_IRQHandler,              /* Timer 2 subtimer A              */
  (pFunc)&Timer2B_IRQHandler,              /* Timer 2 subtimer B              */
  (pFunc)&Comp0_IRQHandler,                /* Analog Comparator 0             */
  (pFunc)&Comp1_IRQHandler,                /* Analog Comparator 1             */
  (pFunc)&Comp2_IRQHandler,                /* Analog Comparator 2             */
  (pFunc)&SysCtrl_IRQHandler,              /* System Control (PLL,OSC,BO)     */
  (pFunc)&FlashCtrl_IRQHandler,            /* FLASH Control                   */
  (pFunc)&GPIOPortF_IRQHandler,            /* GPIO Port F                     */
  (pFunc)&GPIOPortG_IRQHandler,            /* GPIO Port G                     */
  (pFunc)&GPIOPortH_IRQHandler,            /* GPIO Port H                     */
  (pFunc)&UART2_IRQHandler,                /* UART2 Rx and Tx                 */
  (pFunc)&SSI1_IRQHandler,                 /* SSI1 Rx and Tx                  */
  (pFunc)&Timer3A_IRQHandler,              /* Timer 3 subtimer A              */
  (pFunc)&Timer3B_IRQHandler,              /* Timer 3 subtimer B              */
  (pFunc)&I2C1_IRQHandler,                 /* I2C1 Master and Slave           */
  (pFunc)&QEI1_IRQHandler,                 /* Quadrature Encoder 1            */
  (pFunc)&CAN0_IRQHandler,                 /* CAN0                            */
  (pFunc)&CAN1_IRQHandler,                 /* CAN1                            */
  (pFunc)&CAN2_IRQHandler,                 /* CAN2                            */
  (pFunc)&Ethernet_IRQHandler,             /* Ethernet                        */
  (pFunc)&Hibernate_IRQHandler,            /* Hibernate                       */
};


/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) {
  uint32_t *pSrc, *pDest;
  uint32_t *pTable __attribute__((unused));

/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
  pTable = &__copy_table_start__;

  for (; pTable < &__copy_table_end__; pTable = pTable + 3) {
		pSrc  = (uint32_t*)*(pTable + 0);
		pDest = (uint32_t*)*(pTable + 1);
		for (; pDest < (uint32_t*)(*(pTable + 1) + *(pTable + 2)) ; ) {
      *pDest++ = *pSrc++;
		}
	}
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
  pSrc  = &__etext;
  pDest = &__data_start__;

  for ( ; pDest < &__data_end__ ; ) {
    *pDest++ = *pSrc++;
  }
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
  pTable = &__zero_table_start__;

  for (; pTable < &__zero_table_end__; pTable = pTable + 2) {
		pDest = (uint32_t*)*(pTable + 0);
		for (; pDest < (uint32_t*)(*(pTable + 0) + *(pTable + 1)) ; ) {
      *pDest++ = 0;
		}
	}
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
  pDest = &__bss_start__;

  for ( ; pDest < &__bss_end__ ; ) {
    *pDest++ = 0ul;
  }
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

#ifndef __NO_SYSTEM_INIT
	SystemInit();
#endif

#ifndef __START
#define __START _start
#endif
	__START();

}


/* fault exception handlers ------------------------------------------------*/
__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void) {
    static char const * const __attribute__((used)) str_nmi = "NMI";

    __asm volatile (
        "    ldr r0,%0\n\t"
        "    mov r1,#1\n\t"
        "    b assert_failed\n\t"
        : // No outputs.
        : "m" (str_nmi)
    );

}
/*..........................................................................*/
__attribute__((naked)) void MemManage_Handler(void);
void MemManage_Handler(void) {
    static char const * const __attribute__((used)) str_mem = "MemManage";

    __asm volatile (
        "    ldr r0,%0\n\t"
        "    mov r1,#1\n\t"
        "    b assert_failed\n\t"
        : // No outputs.
        : "m" (str_mem)
        );
}
/*..........................................................................*/
__attribute__((naked)) void HardFault_Handler(void);
void HardFault_Handler(void) {
    static char const * const __attribute__((used)) str_hrd = "HardFault";

    __asm volatile (
        "    ldr r0,%0\n\t"
        "    mov r1,#1\n\t"
        "    b assert_failed\n\t"
        : // No outputs.
        : "m" (str_hrd)
    );
}
/*..........................................................................*/
__attribute__((naked)) void BusFault_Handler(void);
void BusFault_Handler(void) {
    static char const * const __attribute__((used)) str_bus = "BusFault";

    __asm volatile (
        "    ldr r0,%0\n\t"
        "    mov r1,#1\n\t"
        "    b assert_failed\n\t"
        : // No outputs.
        : "m" (str_bus)
    );
}
/*..........................................................................*/
__attribute__((naked)) void UsageFault_Handler(void);
void UsageFault_Handler(void) {
    static char const * const __attribute__((used)) str_usage = "UsageFault";

    __asm volatile (
        "    ldr r0,%0\n\t"
        "    mov r1,#1\n\t"
        "    b assert_failed\n\t"
        : // No outputs.
        : "m" (str_usage)
    );
}
#if 0
/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void) {

    while (1);
}
#endif

/*..........................................................................*/
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
    static char const * const __attribute__((used)) str_dflt = "Default";

    __asm volatile (
        "    ldr r0,%0\n\t"
        "    mov r1,#1\n\t"
        "    b assert_failed\n\t"
        : // No outputs.
        : "m" (str_dflt)
    );
}

#if 0
/*..........................................................................*/
void _init(void) { /* dummy */
}
/*..........................................................................*/
void _fini(void) { /* dummy */
}
#endif

/*****************************************************************************
* The function assert_failed defines the error/assertion handling policy
* for the application. After making sure that the stack is OK, this function
* calls Q_onAssert, which should NOT return (typically reset the CPU).
*
* NOTE: the function Q_onAssert should NOT return.
*****************************************************************************/
__attribute__ ((naked))
void assert_failed(char const *module, int loc) {
    /* re-set the SP in case of stack overflow */
    __asm volatile (
        "  MOV sp,%0\n\t"
        : : "r" (&__StackTop));

    //Q_onAssert(module, loc); /* call the application-specific QP handler */

    for (;;) { /* should not be reached, but just in case loop forever... */
    }
}

/****** End Of File *********************************************************/
