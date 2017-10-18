//****************************************************************************
// Product: Blinky example, Arduino-UNO board, cooperative QV kernel, IAR-AVR
// Last Updated for Version: 5.5.0
// Date of the Last Update:  2015-09-28
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

#include <iom328p.h>   // the AVR MCU variant used

// add other drivers if necessary...

//Q_DEFINE_THIS_FILE

#ifdef Q_SPY
    #error Blinky example does not support the Spy build configuration
#endif

// Local-scope objects -------------------------------------------------------
// Arduino-UNO runs off the 16MHz oscillator
#define F_CPU   16000000UL

// the on-board LED labeled "L" on Arduino-UNO
#define LED_L   (1U << 5)

// ISRs used in this project =================================================
#pragma vector = TIMER2_COMPA_vect
__interrupt void timer2_ISR(void) {
    QF::TICK_X(0U, 0); // process time events at rate 0
}

// BSP functions =============================================================
void BSP_init(void) {
    DDRB  = 0xFFU; // All PORTB pins are outputs (user LED)
    PORTB = 0x00U; // drive all pins low
}
//............................................................................
void BSP_ledOff(void) {
    PORTB &= ~LED_L;
}
//............................................................................
void BSP_ledOn(void) {
    PORTB |= LED_L;
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
}

//............................................................................
extern "C" void Q_onAssert(char const Q_ROM * const module, int loc) {
    // implement the error-handling policy for your application!!!
    QF_INT_DISABLE(); // disable all interrupts
    QS_ASSERTION(module, loc, 1000U); // report assertion to QS
    QF_RESET();  // reset the CPU
}

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // set Timer2 in CTC mode, 1/1024 prescaler, start the timer ticking...
    TCCR2A = (1U << WGM21) | (0U << WGM20);
    TCCR2B = (1U << CS22 ) | (1U << CS21) | (1U << CS20); // 1/2^10
    ASSR  &= ~(1U << AS2);
    TIMSK2 = (1U << OCIE2A); // Enable TIMER2 compare Interrupt
    TCNT2  = 0U;
    OCR2A  = (F_CPU / BSP_TICKS_PER_SEC / 1024U) - 1U;
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QV::onIdle(void) {  // called with interrupts DISABLED, see NOTE1
    // toggle the User LED, see NOTE2 , not enough LEDs to implement!
    //QF_INT_DISABLE();
    //PORTB |= LED_L;
    //PORTB &= ~LED_L;
    //QF_INT_ENABLE();

#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular AVR MCU.
    //
    SMCR = (0 << SM0) | (1 << SE); // idle mode, adjust to your project
    QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
#else
    QF_INT_ENABLE(); // just enable interrupts
#endif
}

//****************************************************************************
// NOTE01:
// The QV::onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV::onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
// NOTE2:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
