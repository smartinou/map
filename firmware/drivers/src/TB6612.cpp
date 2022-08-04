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

// This project.
#include "TB6612.h"

// Statndard Libraries.
#include <cstdint>

// TI Library.
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

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

TB6612::TB6612(GPIO const &aIn1, GPIO const &aIn2, GPIO const &aPWM) noexcept
    : mIn1{aIn1}
    , mIn2{aIn2}
    , mPWM{aPWM}
{

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


void TB6612::TurnOnCW([[maybe_unused]] unsigned int const aDutyCycle) const noexcept {

    // PWM: H
    // In1: H
    // In2: L
    //MAP_PWMGenEnable(PWM_BASE, PWM_GEN_1);
    MAP_GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
    MAP_GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), mIn1.GetPin());
    MAP_GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), 0);
}


void TB6612::TurnOnCCW([[maybe_unused]] unsigned int const aDutyCycle) const noexcept {

    // PWM: H
    // In1: L
    // In2: H
    MAP_GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
    //MAP_PWMGenEnable(PWM_BASE, PWM_GEN_1);
    MAP_GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), 0);
    MAP_GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), mIn2.GetPin());
}


void TB6612::TurnOff() const noexcept {

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
