///***************************************************************************
// Product: DPP example, EK-TM4C123GXL board, CMSIS-RTOS RTX kernel
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
#include "dpp.h"
#include "bsp.h"

#include "TM4C123GH6PM.h"        // the device specific header (TI)
#include "rom.h"                 // the built-in ROM functions (TI)
#include "sysctl.h"              // system control driver (TI)
#include "gpio.h"                // GPIO driver (TI)
// add other drivers if necessary...

// namespace DPP *************************************************************
namespace DPP {

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
#define LED_RED     (1U << 1)
#define LED_GREEN   (1U << 3)
#define LED_BLUE    (1U << 2)

#define BTN_SW1     (1U << 4)
#define BTN_SW2     (1U << 0)

static uint32_t l_rnd;  // random seed

#ifdef Q_SPY
    // event-source identifiers used for tracing
    static uint8_t const l_rtx_ticker = 0U;
    static uint8_t const l_GPIOPortA_IRQHandler = 0U;

    #define UART_BAUD_RATE      115200U
    #define UART_FR_TXFE        0x80U
    #define UART_TXFIFO_DEPTH   16U

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER
    };

#endif

extern "C" {

// ISRs used in this project =================================================
void GPIOPortA_IRQHandler(void);  // prototype
void GPIOPortA_IRQHandler(void) {
    DPP::AO_Table->POST(Q_NEW(QP::QEvt, DPP::MAX_SIG), // for testing...
                        &l_GPIOPortA_IRQHandler);
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

#ifdef Q_SPY
        if ((UART0->FR & UART_FR_TXFE) != 0U) { // TX done?
            uint16_t fifo = UART_TXFIFO_DEPTH; // max bytes we can accept

            QF_INT_DISABLE();
            // get next block to transmit
            uint8_t const *block = QP::QS::getBlock(&fifo);
            QF_INT_ENABLE();

            while (fifo-- != 0U) {    // any bytes in the block?
                UART0->DR = *block++; // put into the FIFO
            }
        }
#elif defined NDEBUG
        // Put the CPU and peripherals to the low-power mode.
        // You might need to customize the clock management for your project,
        // see the datasheet for your particular Cortex-M3 MCU.
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

    BSP_randomSeed(1234U);

    if (!QS_INIT((void *)0)) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_rtx_ticker);
    QS_OBJ_DICTIONARY(&l_GPIOPortA_IRQHandler);
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    // exercise the FPU with some floating point computations
    float volatile x;
    x = 3.1415926F;
    x = x + 2.7182818F;

    GPIOF->DATA_Bits[LED_RED]   = ((stat[0] == 'h') ? 0xFFU : 0U);
    GPIOF->DATA_Bits[LED_GREEN] = ((stat[0] == 'e') ? 0xFFU : 0U);

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t paused) {
    GPIOF->DATA_Bits[LED_RED] = ((paused != 0U) ? 0xFFU : 0U);
}
//............................................................................
uint32_t BSP_random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);

    return l_rnd >> 8;
}
//............................................................................
void BSP_randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
}

} // namespace DPP


// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // configure the QF ticker thread
    QF_setRtxTicker(1000U/DPP::BSP_TICKS_PER_SEC, osPriorityAboveNormal );

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
void QF_onRtxTicker() {

    // process all QF time events at tick rate 0
    QF::TICK_X(0U, &DPP::l_rtx_ticker);

    // state of the button debouncing, see below
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { ~0U, ~0U };
    uint32_t current;
    uint32_t tmp;

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    //
    current = ~GPIOF->DATA_Bits[BTN_SW1 | BTN_SW2];  // read SW1 and SW2
    tmp = buttons.depressed;     // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current);  // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    if ((tmp & BTN_SW1) != 0U) {  // debounced SW1 state changed?
        if ((buttons.depressed & BTN_SW1) != 0U) { // is SW1 depressed?
            static QEvt const pauseEvt = { DPP::PAUSE_SIG, 0U, 0U};
            QF::PUBLISH(&pauseEvt, &DPP::l_rtx_ticker);
        }
        else {            // the button is released
            static QEvt const serveEvt = { DPP::SERVE_SIG, 0U, 0U};
            QF::PUBLISH(&serveEvt, &DPP::l_rtx_ticker);
        }
    }
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

// QS callbacks ==============================================================
#ifdef Q_SPY
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[1024]; // buffer for Quantum Spy
    uint32_t tmp;
    initBuf(qsBuf, sizeof(qsBuf));

    // enable clock for UART0 and GPIOA (used by UART0 pins)
    SYSCTL->RCGCUART |= (1U << 0); // enable Run mode for UART0
    SYSCTL->RCGCGPIO |= (1U << 0); // enable Run mode for GPIOA

    // configure UART0 pins for UART operation
    tmp = (1U << 0) | (1U << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->AFSEL |= tmp;
    GPIOA->DR2R  |= tmp;   // set 2mA drive, DR4R and DR8R are cleared
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->DEN   |= tmp;

    // configure the UART for the desired baud rate, 8-N-1 operation
    tmp = (((SystemCoreClock * 8U) / UART_BAUD_RATE) + 1U) / 2U;
    UART0->IBRD   = tmp / 64U;
    UART0->FBRD   = tmp % 64U;
    UART0->LCRH   = 0x60U; // configure 8-N-1 operation
    UART0->LCRH  |= 0x10U;
    UART0->CTL   |= (1U << 0) | (1U << 8) | (1U << 9);

    // setup the QS filters...
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

    QS_FILTER_ON(DPP::PHILO_STAT);

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
extern "C" uint32_t svcKernelSysTick(void); // prototype declaration
QSTimeCtr QS::onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    // NOTE:
    // QS::onGetTime() cannot call the offical RTX osKernelSysTick() service,
    // because osKernelSysTick() is a SVC function, which can't execute
    // with interrupts disabled. Therefore, QS::onGetTime() calls directly
    // the function svcKernelSysTick().
    //
    return (QSTimeCtr)svcKernelSysTick();
}
//............................................................................
void QS::onFlush(void) {
    uint16_t fifo = UART_TXFIFO_DEPTH; // Tx FIFO depth
    uint8_t const *block;
    QF_INT_DISABLE();
    while ((block = getBlock(&fifo)) != static_cast<uint8_t *>(0)) {
        QF_INT_ENABLE();
        // busy-wait until TX FIFO empty
        while ((UART0->FR & UART_FR_TXFE) == 0U) {
        }

        while (fifo-- != 0U) {    // any bytes in the block?
            UART0->DR = *block++; // put into the TX FIFO
        }
        fifo = UART_TXFIFO_DEPTH; // re-load the Tx FIFO depth
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    //TBD
}
//............................................................................
//! callback function to execute a uesr command (to be implemented in BSP)
void QS::onCommand(uint8_t cmdId, uint32_t param) {
    (void)cmdId;
    (void)param;
    //TBD
}
#endif // Q_SPY
//--------------------------------------------------------------------------*/

} // namespace QP

//****************************************************************************
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
