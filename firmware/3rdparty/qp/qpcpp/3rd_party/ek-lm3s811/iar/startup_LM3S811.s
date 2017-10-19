;/***************************************************************************/
; * @file     startup_LM3S811.s for IAR ARM assembler
; * @brief    CMSIS Cortex-M# Core Device Startup File for LM3S811
; * @version  CMSIS 4.3.0
; * @date     20 August 2015
; *
; * @description
; * Created from the CMSIS template for the specified device
; * Quantum Leaps, www.state-machine.com
; *
; * @note
; * The function assert_failed defined at the end of this file defines
; * the error/assertion handling policy for the application and might
; * need to be customized for each project. This function is defined in
; * assembly to re-set the stack pointer, in case it is corrupted by the
; * time assert_failed is called.
; *
; ***************************************************************************/
;/* Copyright (c) 2012 ARM LIMITED
;
;  All rights reserved.
;  Redistribution and use in source and binary forms, with or without
;  modification, are permitted provided that the following conditions are met:
;  - Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
;  - Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
;  - Neither the name of ARM nor the names of its contributors may be used
;    to endorse or promote products derived from this software without
;    specific prior written permission.
;
;  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
;  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;  POSSIBILITY OF SUCH DAMAGE.
;---------------------------------------------------------------------------*/

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        PUBLIC  __vector_table
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;
        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler               ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     MemManage_Handler           ; The MPU fault handler
        DCD     BusFault_Handler            ; The bus fault handler
        DCD     UsageFault_Handler          ; The usage fault handler
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     SVC_Handler                 ; SVCall handler
        DCD     DebugMon_Handler            ; Debug monitor handler
        DCD     0                           ; Reserved
        DCD     PendSV_Handler              ; The PendSV handler
        DCD     SysTick_Handler             ; The SysTick handler

        ; IRQ handlers...
        DCD     GPIOPortA_IRQHandler        ; GPIO Port A
        DCD     GPIOPortB_IRQHandler        ; GPIO Port B
        DCD     GPIOPortC_IRQHandler        ; GPIO Port C
        DCD     GPIOPortD_IRQHandler        ; GPIO Port D
        DCD     GPIOPortE_IRQHandler        ; GPIO Port E
        DCD     UART0_IRQHandler            ; UART0 Rx and Tx
        DCD     UART1_IRQHandler            ; UART1 Rx and Tx
        DCD     SSI0_IRQHandler             ; SSI0 Rx and Tx
        DCD     I2C0_IRQHandler             ; I2C0 Master and Slave
        DCD     0                           ; PWM Fault
        DCD     PWMGen0_IRQHandler          ; PWM Generator 0
        DCD     PWMGen1_IRQHandler          ; PWM Generator 1
        DCD     PWMGen2_IRQHandler          ; PWM Generator 2
        DCD     0                           ; Quadrature Encoder 0
        DCD     ADCSeq0_IRQHandler          ; ADC Sequence 0
        DCD     ADCSeq1_IRQHandler          ; ADC Sequence 1
        DCD     ADCSeq2_IRQHandler          ; ADC Sequence 2
        DCD     ADCSeq3_IRQHandler          ; ADC Sequence 3
        DCD     Watchdog_IRQHandler         ; Watchdog timer
        DCD     Timer0A_IRQHandler          ; Timer 0 subtimer A
        DCD     Timer0B_IRQHandler          ; Timer 0 subtimer B
        DCD     Timer1A_IRQHandler          ; Timer 1 subtimer A
        DCD     Timer1B_IRQHandler          ; Timer 1 subtimer B
        DCD     Timer2A_IRQHandler          ; Timer 2 subtimer A
        DCD     Timer2B_IRQHandler          ; Timer 2 subtimer B
        DCD     Comp0_IRQHandler            ; Analog Comparator 0
        DCD     0                           ; Analog Comparator 1
        DCD     0                           ; Analog Comparator 2
        DCD     SysCtrl_IRQHandler          ; System Control (PLL,OSC,BO)
        DCD     FlashCtrl_IRQHandler        ; FLASH Control

__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors

;******************************************************************************
;
; Weak fault handlers...
;
        SECTION .text:CODE:REORDER:NOROOT(2)

;.............................................................................
        PUBWEAK Reset_Handler
        EXTERN  SystemInit
        EXTERN  __iar_program_start
Reset_Handler
        BL      SystemInit  ; CMSIS system initialization
        BL      __iar_program_start ; IAR startup code
;.............................................................................
        PUBWEAK NMI_Handler
NMI_Handler
        MOVS    r0,#0
        MOVS    r1,#2       ; NMI exception number
        B       assert_failed
;.............................................................................
        PUBWEAK HardFault_Handler
HardFault_Handler
        MOVS    r0,#0
        MOVS    r1,#3       ; HardFault exception number
        B       assert_failed
;.............................................................................
        PUBWEAK MemManage_Handler
MemManage_Handler
        MOVS    r0,#0
        MOVS    r1,#4       ; MemManage exception number
        B       assert_failed
;.............................................................................
        PUBWEAK BusFault_Handler
BusFault_Handler
        MOVS    r0,#0
        MOVS    r1,#5       ; BusFault exception number
        B       assert_failed
;.............................................................................
        PUBWEAK UsageFault_Handler
UsageFault_Handler
        MOVS    r0,#0
        MOVS    r1,#6       ; UsageFault exception number
        B       assert_failed


;******************************************************************************
;
; Weak non-fault handlers...
;

        PUBWEAK SVC_Handler
SVC_Handler
        MOVS    r0,#0
        MOVS    r1,#11      ; SVCall exception number
        B       assert_failed
;.............................................................................
        PUBWEAK DebugMon_Handler
DebugMon_Handler
        MOVS    r0,#0
        MOVS    r1,#12      ; DebugMon exception number
        B       assert_failed
;.............................................................................
        PUBWEAK PendSV_Handler
PendSV_Handler
        MOVS    r0,#0
        MOVS    r1,#14      ; PendSV exception number
        B       assert_failed
;.............................................................................
        PUBWEAK SysTick_Handler
SysTick_Handler
        MOVS    r0,#0
        MOVS    r1,#15      ; SysTick exception number
        B       assert_failed


;******************************************************************************
;
; Weak IRQ handlers...
;

        PUBWEAK GPIOPortA_IRQHandler
        PUBWEAK GPIOPortB_IRQHandler
        PUBWEAK GPIOPortC_IRQHandler
        PUBWEAK GPIOPortD_IRQHandler
        PUBWEAK GPIOPortE_IRQHandler
        PUBWEAK UART0_IRQHandler
        PUBWEAK UART1_IRQHandler
        PUBWEAK SSI0_IRQHandler
        PUBWEAK I2C0_IRQHandler
        PUBWEAK PWMGen0_IRQHandler
        PUBWEAK PWMGen1_IRQHandler
        PUBWEAK PWMGen2_IRQHandler
        PUBWEAK ADCSeq0_IRQHandler
        PUBWEAK ADCSeq1_IRQHandler
        PUBWEAK ADCSeq2_IRQHandler
        PUBWEAK ADCSeq3_IRQHandler
        PUBWEAK Watchdog_IRQHandler
        PUBWEAK Timer0A_IRQHandler
        PUBWEAK Timer0B_IRQHandler
        PUBWEAK Timer1A_IRQHandler
        PUBWEAK Timer1B_IRQHandler
        PUBWEAK Timer2A_IRQHandler
        PUBWEAK Timer2B_IRQHandler
        PUBWEAK Comp0_IRQHandler
        PUBWEAK SysCtrl_IRQHandler
        PUBWEAK FlashCtrl_IRQHandler

GPIOPortA_IRQHandler
GPIOPortB_IRQHandler
GPIOPortC_IRQHandler
GPIOPortD_IRQHandler
GPIOPortE_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
SSI0_IRQHandler
I2C0_IRQHandler
PWMGen0_IRQHandler
PWMGen1_IRQHandler
PWMGen2_IRQHandler
ADCSeq0_IRQHandler
ADCSeq1_IRQHandler
ADCSeq2_IRQHandler
ADCSeq3_IRQHandler
Watchdog_IRQHandler
Timer0A_IRQHandler
Timer0B_IRQHandler
Timer1A_IRQHandler
Timer1B_IRQHandler
Timer2A_IRQHandler
Timer2B_IRQHandler
Comp0_IRQHandler
SysCtrl_IRQHandler
FlashCtrl_IRQHandler
        MOV     r0,#0
        MOV     r1,#-1      ; 0xFFFFFFF
        B       assert_failed

;******************************************************************************
;
; The function assert_failed defines the error/assertion handling policy
; for the application. After making sure that the stack is OK, this function
; calls Q_onAssert, which should NOT return (typically reset the CPU).
;
; NOTE: the function Q_onAssert should NOT return.
;
; The C proptotype of the assert_failed() and Q_onAssert() functions are:
; void assert_failed(char const *file, int line);
; void Q_onAssert   (char const *file, int line);
;******************************************************************************
        PUBLIC  assert_failed
        EXTERN  Q_onAssert
assert_failed
        LDR    sp,=sfe(CSTACK)   ; re-set the SP in case of stack overflow
        BL     Q_onAssert        ; call the application-specific handler

        B      .                 ; should not be reached, but just in case...


        END                      ; end of module
