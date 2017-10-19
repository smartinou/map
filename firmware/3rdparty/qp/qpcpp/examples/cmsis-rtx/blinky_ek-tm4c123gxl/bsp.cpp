///***************************************************************************
// Product: Blinky example, EK-TM4C123GXL board, CMSIS-RTOS RTX kernel
// Last updated for version 5.5.0
// Last updated on  2015-09-23
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// http://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "blinky.h"
#include "bsp.h"

#include "TM4C123GH6PM.h"        // the device specific header (TI)
#include "rom.h"                 // the built-in ROM functions (TI)
#include "sysctl.h"              // system control driver (TI)
#include "gpio.h"                // GPIO driver (TI)
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

// Local-scope objects -------------------------------------------------------
#define LED_RED     (1U << 1)
#define LED_GREEN   (1U << 3)
#define LED_BLUE    (1U << 2)

#define BTN_SW1     (1U << 4)
#define BTN_SW2     (1U << 0)

extern "C" {

// ISRs used in this project =================================================
void GPIOPortA_IRQHandler(void);  // prototype
void GPIOPortA_IRQHandler(void) {
    AO_Blinky->POST(Q_NEW(QEvt, DUMMY_SIG), // for testing...
                    (void *)0);
    // NOTE:
    // There is no need to explicitly pend the PendSV exception, because
    // RTX handles this when signaling the task. (See OS_PEND_IRQ() macro
    // in RTX source code).
    //
}

// RTX callbacks =============================================================
void os_idle_demon(void); // prototype
void os_idle_demon(void) {
    // The RTX idle demon is a system thread, running when no other thread
    // is ready to run.

    for (;;) { // idle-loop
        QF_INT_DISABLE();
        GPIOF->DATA_Bits[LED_BLUE] = 0xFFU; // turn LED on
        GPIOF->DATA_Bits[LED_BLUE] = 0x00U; // turn LED off
        QF_INT_ENABLE();

#ifdef NDEBUG
        // Put the CPU and peripherals to the low-power mode.
        // You might need to customize the clock management for your
        // application, see the datasheet for your particular MCU.
        //
        __WFI(); // Wait-For-Interrupt
#endif
    } // idle-loop
}
//............................................................................
// This function is called when RTX detects a runtime error.
// Parameter 'error_code' holds the runtime error code.
//
void os_error(uint32_t err_code); // prototype
void os_error(uint32_t error_code) {
    // perform customized error handling...
    GPIOF->DATA_Bits[LED_RED] = 0xFFU; // turn LED on
    Q_ERROR_ID(error_code);  // NOTE: does not return
}

} // extern "C"

// BSP functions =============================================================
void BSP_init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // enable clock for to the peripherals used by this application...
    SYSCTL->RCGCGPIO |= (1U << 5); // enable Run mode for GPIOF

    // configure the LEDs and push buttons
    GPIOF->DIR |= (LED_RED | LED_GREEN | LED_BLUE); // set direction: output
    GPIOF->DEN |= (LED_RED | LED_GREEN | LED_BLUE); // digital enable
    GPIOF->DATA_Bits[LED_RED]   = 0U;  // turn the LED off
    GPIOF->DATA_Bits[LED_GREEN] = 0U;  // turn the LED off
    GPIOF->DATA_Bits[LED_BLUE]  = 0U;  // turn the LED off

    // configure the Buttons
    GPIOF->DIR &= ~(BTN_SW1 | BTN_SW2); //  set direction: input
    ROM_GPIOPadConfigSet(GPIOF_BASE, (BTN_SW1 | BTN_SW2),
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}
//............................................................................
void BSP_ledOff(void) {
    GPIOF->DATA_Bits[LED_GREEN] = 0U;  // turn the LED off
}
//............................................................................
void BSP_ledOn(void) {
    // exercise the FPU with some floating point computations...
    float volatile x;
    x = 3.1415926F;
    x = x + 2.7182818F;

    GPIOF->DATA_Bits[LED_GREEN] = 0xFFU; // turn the LED on
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
}

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // configure the QF ticker thread
    QF_setRtxTicker(1000U/BSP_TICKS_PER_SEC, osPriorityAboveNormal );

    // set priorities of ISRs used in the system...
    NVIC_SetPriority(GPIOA_IRQn,   1U);
    // ...

    // enable IRQs in the NVIC...
    NVIC_EnableIRQ(GPIOA_IRQn);
    // ...
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QP::QF_onRtxTicker() {
    QF::TICK_X(0U, (void *)0); // process all QF time events at tick rate 0
}

//............................................................................
extern "C" void Q_onAssert(char const *module, int loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
    NVIC_SystemReset();
}

//****************************************************************************
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
