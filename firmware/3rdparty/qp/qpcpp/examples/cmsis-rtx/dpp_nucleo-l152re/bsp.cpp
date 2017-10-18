///***************************************************************************
// Product: DPP example, NUCLEO-L152RE board, CMSIS-RTOS RTX kernel
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

#include "stm32l1xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

// namespace DPP *************************************************************
namespace DPP {

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
// LED pins available on the board (just one user LED LD2--Green on PA.5)
#define LED_LD2  (1U << 5)

// Button pins available on the board (just one user Button B1 on PC.13)
#define BTN_B1   (1U << 13)

static uint32_t l_rnd;  // random seed

#ifdef Q_SPY
    // event-source identifiers used for tracing
    static uint8_t const l_rtx_ticker = 0U;
    static uint8_t const l_EXTI0_IRQHandler = 0U;

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER
    };

#endif

extern "C" {

// ISRs used in this project =================================================
void EXTI0_IRQHandler(void); // prototype
void EXTI0_IRQHandler(void) {
    DPP::AO_Table->POST(Q_NEW(QP::QEvt, DPP::MAX_SIG), // for testing...
                        &l_EXTI0_IRQHandler);
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
        //GPIOA->BSRRL |= LED_LD2;  // turn LED on
        //GPIOA->BSRRH |= LED_LD2;  // turn LED off
        QF_INT_ENABLE();

#ifdef Q_SPY
        if ((USART2->SR & 0x80U) != 0) {  // is TXE empty?
            QF_INT_DISABLE();
            uint16_t b = QP::QS::getByte();
            QF_INT_ENABLE();

            if (b != QP::QS_EOD) {  // not End-Of-Data?
                USART2->DR = (b & 0xFFU); // put into the DR register
            }
        }
#elif defined NDEBUG
        // Put the CPU and peripherals to the low-power mode.
        // You might need to customize the clock management for your project,
        // see the datasheet for your particular Cortex-M3 MCU.
        //
        // !!!CAUTION!!!
        // The WFI instruction stops the CPU clock, which unfortunately
        // disables the JTAG port, so the ST-Link debugger can no longer
        // connect to the board. For that reason, the call to __WFI() has
        // to be used with CAUTION.
        //
        // NOTE: If you find your board "frozen" like this, strap BOOT0 to
        // VDD and reset the board, then connect with ST-Link Utilities and
        // erase the part. The trick with BOOT(0) is it gets the part to run
        // the System Loader instead of your broken code. When done
        // disconnect BOOT0, and start over.
        //
        //__WFI(); // Wait-For-Interrupt
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
    //GPIOA->BSRRL |= LED_LD2;  // turn LED on
    Q_ERROR_ID(error_code);  // NOTE: does not return
}

} // extern "C"

// BSP functions =============================================================
void BSP_init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // enable GPIOA clock port for the LED LD2
    RCC->AHBENR |= (1U << 0);

    // configure LED (PA.5) pin as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~((3U << 2*5));
    GPIOA->MODER   |=  ((1U << 2*5));
    GPIOA->OTYPER  &= ~((1U <<   5));
    GPIOA->OSPEEDR &= ~((3U << 2*5));
    GPIOA->OSPEEDR |=  ((1U << 2*5));
    GPIOA->PUPDR   &= ~((3U << 2*5));

    // enable GPIOC clock port for the Button B1
    RCC->AHBENR    |=  (1U << 2);

    // configure Button (PC.13) pins as input, no pull-up, pull-down
    GPIOC->MODER   &= ~(3U << 2*13);
    GPIOC->OSPEEDR &= ~(3U << 2*13);
    GPIOC->OSPEEDR |=  (1U << 2*13);
    GPIOC->PUPDR   &= ~(3U << 2*13);

    BSP_randomSeed(1234U);

    if (!QS_INIT((void *)0)) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_rtx_ticker);
    QS_OBJ_DICTIONARY(&l_EXTI0_IRQHandler);
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == 'h') {
        GPIOA->BSRRL |= LED_LD2;  // turn LED on
    }
    else {
        GPIOA->BSRRH |= LED_LD2;  // turn LED off
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t paused) {
    // not enough LEDs to implement this feature
    if (paused != (uint8_t)0) {
        //GPIOA->BSRRL |= LED_LD2;  // turn LED on
    }
    else {
        //GPIOA->BSRRH |= LED_LD2;  // turn LED off
    }
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
    NVIC_SetPriority(EXTI0_IRQn,   1U);
    // ...

    // enable IRQs in the NVIC...
    NVIC_EnableIRQ(EXTI0_IRQn);
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
    current = ~GPIOC->IDR; // read Port C with the state of Button B1
    tmp = buttons.depressed;     // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current);  // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    if ((tmp & BTN_B1) != 0U) {   // debounced B1 state changed?
        if ((buttons.depressed & BTN_B1) != 0U) { // is B1 depressed?
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

/*..........................................................................*/
#define __DIV(__PCLK, __BAUD)       (((__PCLK / 4) *25)/(__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   \
    (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) \
        * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) \
    ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[1024]; // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    // enable peripheral clock for USART2
    RCC->AHBENR   |=  (1U <<  0);      // Enable GPIOA clock
    RCC->APB1ENR  |=  (1U << 17);      // Enable USART#2 clock

    // Configure PA3 to USART2_RX, PA2 to USART2_TX
    GPIOA->AFR[0] &= ~((15U << 4*3) | (15U << 4*2));
    GPIOA->AFR[0] |=  (( 7U << 4*3) | ( 7U << 4*2));
    GPIOA->MODER  &= ~(( 3U << 2*3) | ( 3U << 2*2));
    GPIOA->MODER  |=  (( 2U << 2*3) | ( 2U << 2*2));

    USART2->BRR  = __USART_BRR(SystemCoreClock, 115200U);  // baud rate
    USART2->CR3  = 0x0000U;        // no flow control
    USART2->CR2  = 0x0000U;        // 1 stop bit
    USART2->CR1  = ((1U <<  2) |   // enable RX
                    (1U <<  3) |   // enable TX
                    (0U << 12) |   // 1 start bit, 8 data bits
                    (1U << 13));   // enable USART

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
QSTimeCtr QS::onGetTime(void) { // NOTE: invoked with interrupts DISABLED
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
    uint16_t b;

    QF_INT_DISABLE();
    while ((b = getByte()) != QS_EOD) { // while not End-Of-Data...
        QF_INT_ENABLE();
        while ((USART2->SR & 0x80U) == 0U) { // while TXE not empty
        }
        USART2->DR  = (b & 0xFFU);  // put into the DR register
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
