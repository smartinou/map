///***************************************************************************
// Product: "Fly 'n' Shoot" game example on EK-LM3S811, cooperative QV kernel
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
#include "game.h"
#include "bsp.h"

#include "LM3S811.h"        // the device specific header (TI)
#include "display96x16x1.h" // the OLED display driver (TI)
// add other drivers if necessary...


// namespace GAME ************************************************************
namespace GAME {

Q_DEFINE_THIS_FILE

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
// DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
//
enum KernelUnawareISRs { // see NOTE00
    // ...
    MAX_KERNEL_UNAWARE_CMSIS_PRI  // keep always last
};
// "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI <= QF_AWARE_ISR_CMSIS_PRI);

enum KernelAwareISRs {
    ADCSEQ3_PRIO = QF_AWARE_ISR_CMSIS_PRI, // see NOTE00
    GPIOPORTA_PRIO,
    SYSTICK_PRIO,
    // ...
    MAX_KERNEL_AWARE_CMSIS_PRI // keep always last
};
// "kernel-aware" interrupts should not overlap the PendSV priority
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));

// Local-scope objects -------------------------------------------------------
// LEDs available on the board
#define USER_LED  (1U << 5)

// Push-Button wired externally to DIP8 (P0.6)
#define USER_BTN  (1U << 4)

// other useful registers...
#define ADC_TRIGGER_TIMER  0x00000005U
#define ADC_CTL_IE         0x00000040U
#define ADC_CTL_END        0x00000020U
#define ADC_CTL_CH0        0x00000000U
#define ADC_SSFSTAT0_EMPTY 0x00000100U
#define UART_FR_TXFE       0x00000080U

#ifdef Q_SPY

    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    // event-source identifiers used for tracing
    static uint8_t const l_SysTick_Handler      = 0U;
    static uint8_t const l_ADCSeq3_IRQHandler   = 0U;
    static uint8_t const l_GPIOPortA_IRQHandler = 0U;

    #define UART_BAUD_RATE      115200U
    #define UART_TXFIFO_DEPTH   16U

#endif

// ISRs used in this project =================================================
extern "C" {

//............................................................................
void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {

#ifdef Q_SPY
    {
        uint32_t volatile tmp = SysTick->CTRL; // clear SysTick_CTRL_COUNTFLAG
        (void)tmp; // unused local variable
        QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
    }
#endif

    static QP::QEvt const tickEvt = { TIME_TICK_SIG, 0U, 0U };
    QP::QF::TICK_X(0U, &l_SysTick_Handler); // process time events for rate 0
    QP::QF::PUBLISH(&tickEvt, &l_SysTick_Handler); // publish to subscribers
}
//............................................................................
void ADCSeq3_IRQHandler(void); // prototype
void ADCSeq3_IRQHandler(void) {
    static uint32_t adcLPS = 0U; // Low-Pass-Filtered ADC reading
    static uint32_t wheel  = 0U; // the last wheel position
    uint32_t tmp;

    ADC->ISC = (1U << 3); // clear the ADCSeq3 interrupt

    // the ADC Sequence 3 FIFO must have a sample
    Q_ASSERT((ADC->SSFSTAT3 & ADC_SSFSTAT0_EMPTY) == 0U);


    // 1st order low-pass filter: time constant ~= 2^n samples
    // TF = (1/2^n)/(z-((2^n - 1)/2^n)),
    // eg, n=3, y(k+1) = y(k) - y(k)/8 + x(k)/8 => y += (x - y)/8
    //
    tmp = ADC->SSFIFO3; // read the data from the ADC
    adcLPS += (((int)tmp - (int)adcLPS + 4) >> 3);

    // compute the next position of the wheel
    tmp = (((1U << 10) - adcLPS)*(BSP_SCREEN_HEIGHT - 2U)) >> 10;

    if (tmp != wheel) { // did the wheel position change?
        ObjectPosEvt *ope = Q_NEW(ObjectPosEvt, PLAYER_SHIP_MOVE_SIG);
        ope->x = (uint8_t)GAME_SHIP_X; // x-position is fixed
        ope->y = (uint8_t)tmp;
        AO_Ship->POST(ope, &l_ADCSeq3_IRQHandler);
        wheel = tmp; // save the last position of the wheel
    }

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    //
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { ~0U, ~0U };
    uint32_t current;

    current = ~GPIOC->DATA; // read the port with the User Button
    tmp = buttons.depressed; // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    if ((tmp & USER_BTN) != 0U) { // debounced USER_BTN state changed?
        if ((buttons.depressed & USER_BTN) != 0U) { // is USER_BTN depressed?
            static QP::QEvt const fireEvt = { PLAYER_TRIGGER_SIG, 0U, 0U};
            QP::QF::PUBLISH(&fireEvt, &l_ADCSeq3_IRQHandler);
        }
        else { // the button is released
        }
    }
}
//............................................................................
void GPIOPortA_IRQHandler(void); // prototype
void GPIOPortA_IRQHandler(void) {
    AO_Tunnel->POST(Q_NEW(QP::QEvt, MAX_PUB_SIG), // for testing...
                    &l_GPIOPortA_IRQHandler);
}

} // extern "C"

// BSP functions =============================================================
void BSP_init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // enable clock to the peripherals used by the application
    SYSCTL->RCGC0 |= (1U << 16);              // enable clock to ADC
    SYSCTL->RCGC1 |= (1U << 16) | (1U << 17); // enable clock to TIMER0 & 1
    SYSCTL->RCGC2 |= (1U <<  0) | (1U <<  2); // enable clock to GPIOA & C
    __NOP();                                  // wait after enabling clocks
    __NOP();
    __NOP();

    // Configure the ADC Sequence 3 to sample the potentiometer when the
    // timer expires. Set the sequence priority to 0 (highest).
    //
    ADC->EMUX   = (ADC->EMUX   & ~(0xFU << (3*4)))
                  | (ADC_TRIGGER_TIMER << (3*4));
    ADC->SSPRI  = (ADC->SSPRI  & ~(0xFU << (3*4)))
                  | (0U << (3*4));

    // set ADC Sequence 3 step to 0
    ADC->SSMUX3 = (ADC->SSMUX3 & ~(0xFU << (0*4)))
                  | ((ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END) << (0*4));
    ADC->SSCTL3 = (ADC->SSCTL3 & ~(0xFU << (0*4)))
                  | (((ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END) >> 4) <<(0*4));
    ADC->ACTSS |= (1U << 3);

    // configure TIMER1 to trigger the ADC to sample the potentiometer
    TIMER1->CTL  &= ~((1U << 0) | (1U << 16));
    TIMER1->CFG   = 0x00U;
    TIMER1->TAMR  = 0x02U;
    TIMER1->TAILR = SystemCoreClock / 120U;
    TIMER1->CTL  |= 0x02U;
    TIMER1->CTL  |= 0x20U;

    // configure the User LED...
    GPIOC->DIR |= USER_LED;          // set direction: output
    GPIOC->DEN |= USER_LED;          // digital enable
    GPIOC->DATA_Bits[USER_LED] = 0U; // turn the User LED off

    // configure the User Button...
    GPIOC->DIR &= ~USER_BTN;         //  set direction: input
    GPIOC->DEN |= USER_BTN;          // digital enable

    Display96x16x1Init(1); // initialize the OLED display

    if (QS_INIT((void *)0) == 0U) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_ADCSeq3_IRQHandler);
    QS_OBJ_DICTIONARY(&l_GPIOPortA_IRQHandler);
}
//............................................................................
void BSP_drawBitmap(uint8_t const *bitmap) {
    Display96x16x1ImageDraw(bitmap, 0, 0,
                            BSP_SCREEN_WIDTH, (BSP_SCREEN_HEIGHT >> 3));
}
//............................................................................
void BSP_drawBitmapXY(uint8_t const *bitmap, uint8_t x, uint8_t y) {
    Display96x16x1ImageDraw(bitmap, x, y,
                            BSP_SCREEN_WIDTH, (BSP_SCREEN_HEIGHT >> 3));
}
//............................................................................
void BSP_drawNString(uint8_t x, uint8_t y, char const *str) {
    Display96x16x1StringDraw(str, x, y);
}
//............................................................................
void BSP_updateScore(uint16_t score) {
    // no room on the OLED display of the EV-LM3S811 board for the score
}
//............................................................................
void BSP_displayOn(void) {
    Display96x16x1DisplayOn();
}
//............................................................................
void BSP_displayOff(void) {
    Display96x16x1DisplayOff();
}

} // namespace GAME


// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    SysTick_Config(SystemCoreClock / GAME::BSP_TICKS_PER_SEC);

    // assing all priority bits for preemption-prio. and none to sub-prio.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE00
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority()
    // DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    //
    NVIC_SetPriority(ADCSeq3_IRQn, GAME::ADCSEQ3_PRIO);
    NVIC_SetPriority(SysTick_IRQn, GAME::GPIOPORTA_PRIO);
    NVIC_SetPriority(SysTick_IRQn, GAME::SYSTICK_PRIO);
    // ...

    // enable ADC
    ADC->ISC = (1U << 3);
    ADC->IM |= (1U << 3);

    TIMER1->CTL |= ((1U << 0) | (1U << 16)); // enable TIMER1

    // enable IRQs...
    NVIC_EnableIRQ(GPIOPortA_IRQn);
    NVIC_EnableIRQ(ADCSeq3_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QV::onIdle(void) {  // called with interrupts disabled, see NOTE01
    // toggle the User LED on and then off, see NOTE02
    QF_INT_DISABLE();
    GPIOC->DATA_Bits[USER_LED] = 0xFFU; // turn the User LED on
    GPIOC->DATA_Bits[USER_LED] = 0x00U; // turn the User LED off
    QF_INT_ENABLE();

#ifdef Q_SPY
    if ((UART0->FR & UART_FR_TXFE) != 0U) { // TX done?
        uint16_t fifo = UART_TXFIFO_DEPTH;  // max bytes we can accept
        uint8_t const *block;

        QF_INT_DISABLE();
        block = QS::getBlock(&fifo); // try to get next block to transmit
        QF_INT_ENABLE();

        while (fifo-- != 0U) {       // any bytes in the block?
            UART0->DR = *block++;    // put into the FIFO
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
#else
    QF_INT_ENABLE(); // just enable interrupts
#endif
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
    static uint8_t qsBuf[2*1024]; // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    // enable the peripherals used by the UART0
    SYSCTL->RCGC1 |= (1U << 0);  // enable clock to UART0
    SYSCTL->RCGC2 |= (1U << 0);  // enable clock to GPIOA
    __NOP();                     // wait after enabling clocks
    __NOP();
    __NOP();

    // configure UART0 pins for UART operation
    uint32_t tmp = (1U << 0) | (1U << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->AFSEL |= tmp;
    GPIOA->DR2R  |= tmp; // set 2mA drive, DR4R and DR8R are cleared
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

    GAME::QS_tickPeriod_ = SystemCoreClock / GAME::BSP_TICKS_PER_SEC;
    GAME::QS_tickTime_ = GAME::QS_tickPeriod_; // to start timestamp at zero

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

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) { // not set?
        return GAME::QS_tickTime_ - static_cast<QSTimeCtr>(SysTick->VAL);
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return GAME::QS_tickTime_ + GAME::QS_tickPeriod_
               - static_cast<QSTimeCtr>(SysTick->VAL);
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t fifo = UART_TXFIFO_DEPTH; // Tx FIFO depth
    uint8_t const *block;
    QF_INT_DISABLE();
    while ((block = getBlock(&fifo)) != (uint8_t *)0) {
        QF_INT_ENABLE();
        // busy-wait until TX FIFO empty
        while ((UART0->FR & UART_FR_TXFE) == 0U) {
        }

        while (fifo-- != 0U) { // any bytes in the block?
            UART0->DR = *block++;  // put into the TX FIFO
        }
        fifo = UART_TXFIFO_DEPTH;  // re-load the Tx FIFO depth
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
// NOTE00:
// The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
// ISR priority that is disabled by the QF framework. The value is suitable
// for the NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
// with the numerical values of priorities equal or higher than
// QF_AWARE_ISR_CMSIS_PRI) are allowed to call the QK_ISR_ENTRY/QK_ISR_ENTRY
// macros or any other QF/QK  services. These ISRs are "QF-aware".
//
// Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
// level (i.e., with the numerical values of priorities less than
// QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
// Such "QF-unaware" ISRs cannot call any QF/QK services. In particular they
// can NOT call the macros QK_ISR_ENTRY/QK_ISR_ENTRY. The only mechanism
// by which a "QF-unaware" ISR can communicate with the QF framework is by
// triggering a "QF-aware" ISR, which can post/publish events.
//
// NOTE01:
// The QV::onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV::onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
// NOTE02:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
