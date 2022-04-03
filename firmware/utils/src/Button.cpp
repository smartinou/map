// *****************************************************************************
//
// Project: Utilities.
//
// Module: Button.
//
// *****************************************************************************

//! \file
//! \brief Button class.
//! \ingroup utils_button

// *****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
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
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

// This project.
#include "inc/Button.h"

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

Button::Button(
    unsigned long const aGPIOPort,
    unsigned int  const aGPIOPin,
    unsigned long const aIntNbr,
    unsigned int  const aID
)   : GPIO(aGPIOPort, aGPIOPin)
    , mIntNbr(aIntNbr)
    , mID(aID)
{

    // Make sure the peripheral clock is enabled or else the following calls will raise an exception.
    GPIO::EnableSysCtlPeripheral(aGPIOPort);

    DisableInt();

    // Set specified GPIO as edge triggered input.
    // Don't enable interrupt just yet.
    MAP_GPIOPinTypeGPIOInput(GetPort(), GetPin());
    MAP_GPIOIntTypeSet(GetPort(), GetPin(), GPIO_BOTH_EDGES);
    MAP_GPIOPadConfigSet(
        GetPort(),
        GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD_WPU
    );

    // Enable the interrupt of the selected GPIO.
    // Don't enable the interrupt globally yet.
#ifdef USE_TIVAWARE
    MAP_GPIOIntEnable(GetPort(), GetPin());
    MAP_GPIOIntClear(GetPort(), GetPin());
#elif defined (USE_STELLARISWARE)
    MAP_GPIOPinIntEnable(GetPort(), GetPin());
    MAP_GPIOPinIntClear(GetPort(), GetPin());
#else
#error Must either define USE_TIVAWARE or USE_STELLARISWARE.
#endif
}


Button::Button(
    GPIO          const &aGPIO,
    unsigned long const aIntNbr,
    unsigned int  const aID
)   : Button(aGPIO.GetPort(), aGPIO.GetPin(), aIntNbr, aID) {
    // Ctor body left intentionally empty.
}


Button::State Button::GetGPIOPinState(void) const {

    unsigned long lGPIOPin = MAP_GPIOPinRead(GetPort(), GetPin());
    if (lGPIOPin & GetPin()) {
        return IS_HIGH;
    }

    return IS_LOW;
}


void Button::DisableInt(void) const {
    MAP_IntDisable(mIntNbr);
}


void Button::EnableInt(void) const {
    MAP_IntEnable(mIntNbr);
}


void Button::ClrInt(void) const {
#ifdef USE_TIVAWARE
    MAP_GPIOIntClear(GetPort(), GetPin());
#elif defined (USE_STELLARISWARE)
    MAP_GPIOPinIntClear(GetPort(), GetPin());
#endif
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
