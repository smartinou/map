// *****************************************************************************
//
// Project: Component drivers.
//
// Module: TB6612.
//
// *******************************************************************************

//! \file
//! \brief TB6612 class.
//! \ingroup ext_peripherals

// *****************************************************************************
//
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stdio.h>

// TI Library.
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

// This project.
#include "TB6612.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************
#if 0
TB6612::TB6612(unsigned long aIn1GPIOPort,
               unsigned int  aIn1GPIOPin,
               unsigned long aIn2GPIOPort,
               unsigned int  aIn2GPIOPin,
               unsigned long aPWMGPIOPort,
               unsigned int  aPWMGPIOPin):
    mIn1GPIOPort(aIn1GPIOPort),
    mIn2GPIOPort(aIn2GPIOPort),
    mPWMGPIOPort(aPWMGPIOPort),
    mIn1GPIOPin(aIn1GPIOPin),
    mIn2GPIOPin(aIn2GPIOPin),
    mPWMGPIOPin(aPWMGPIOPin) {

    // In1.
    MAP_GPIOPinTypeGPIOOutput(mIn1GPIOPort, mIn1GPIOPin);
    MAP_GPIOPadConfigSet(
        mIn1GPIOPort,
        mIn1GPIOPin,
        GPIO_STRENGTH_4MA,
        GPIO_PIN_TYPE_STD);

    // In2.
    MAP_GPIOPinTypeGPIOOutput(mIn2GPIOPort, mIn2GPIOPin);
    MAP_GPIOPadConfigSet(
        mIn2GPIOPort,
        mIn2GPIOPin,
        GPIO_STRENGTH_4MA,
        GPIO_PIN_TYPE_STD);

    // PWM.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
    MAP_GPIOPinTypePWM(mPWMGPIOPort, mPWMGPIOPin);
    MAP_GPIOPadConfigSet(
        mPWMGPIOPort,
        mPWMGPIOPin,
        GPIO_STRENGTH_4MA,
        GPIO_PIN_TYPE_STD);

    MAP_PWMGenConfigure(
        PWM_BASE,
        PWM_GEN_1,
        PWM_GEN_MODE_DOWN |
        PWM_GEN_MODE_NO_SYNC |
        PWM_GEN_MODE_DBG_STOP |
        PWM_GEN_MODE_GEN_NO_SYNC |
        PWM_GEN_MODE_DB_NO_SYNC);

    // 8MHz SysClk = 125ns period.
    // 125ns * 400 = 50us = 20KHz PWM period.
    MAP_PWMGenPeriodSet(
        PWM_BASE,
        PWM_GEN_1,
        (400 - 1)); //ulPeriod);
    MAP_PWMPulseWidthSet(
        PWM_BASE,
        PWM_GEN_1,
        (320 - 1)); //ulWidth);
    //MAP_PWMGenEnable(PWM_BASE, PWM_GEN_1);
    MAP_PWMOutputState(PWM_BASE,
                 PWM_OUT_2_BIT,
                 true);
}
#endif

TB6612::TB6612(GPIO const &aIn1, GPIO const &aIn2, GPIO const &aPWM)
    : mIn1(aIn1)
    , mIn2(aIn2)
    , mPWM(aPWM) {

    // In1.
    MAP_GPIOPinTypeGPIOOutput(mIn1.GetPort(), mIn1.GetPin());
    MAP_GPIOPadConfigSet(
        mIn1.GetPort(),
        mIn1.GetPin(),
        GPIO_STRENGTH_4MA,
        GPIO_PIN_TYPE_STD
    );

    // In2.
    MAP_GPIOPinTypeGPIOOutput(mIn2.GetPort(), mIn2.GetPin());
    MAP_GPIOPadConfigSet(
        mIn2.GetPort(),
        mIn2.GetPin(),
        GPIO_STRENGTH_4MA,
        GPIO_PIN_TYPE_STD
    );

    // PWM.
    MAP_GPIOPinTypeGPIOOutput(mPWM.GetPort(), mPWM.GetPin());
    MAP_GPIOPadConfigSet(
        mPWM.GetPort(),
        mPWM.GetPin(),
        GPIO_STRENGTH_4MA,
        GPIO_PIN_TYPE_STD
    );
}


void TB6612::TurnOnCW(unsigned int const aDutyCycle) const {

    // PWM: H
    // In1: H
    // In2: L
    //MAP_PWMGenEnable(PWM_BASE, PWM_GEN_1);
    MAP_GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
    MAP_GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), mIn1.GetPin());
    MAP_GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), 0);
}


void TB6612::TurnOnCCW(unsigned int const aDutyCycle) const {

    // PWM: H
    // In1: L
    // In2: H
    MAP_GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
    //MAP_PWMGenEnable(PWM_BASE, PWM_GEN_1);
    MAP_GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), 0);
    MAP_GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), mIn2.GetPin());
}


void TB6612::TurnOff(void) const {

    // PWM: H
    // In1: L
    // In2: L
    //MAP_PWMGenDisable(PWM_BASE, PWM_GEN_1);
    MAP_GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
    MAP_GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), 0);
    MAP_GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), 0);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
