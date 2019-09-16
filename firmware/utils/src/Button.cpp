// *****************************************************************************
//
// Project: Utilities.
//
// Module: GPIO.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup utils_gpio

// *****************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// TI Library.
#include <hw_types.h>
#include <hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>

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
)
    : GPIO(aGPIOPort, aGPIOPin)
    , mIntNbr(aIntNbr)
    , mID(aID) {

    // Make sure the peripheral clock is enabled or else the following calls will raise an exception.
    SysCtlPeripheralEnable(PortToSysClockPeripheral(aGPIOPort));

    DisableInt();

    // Set specified GPIO as edge triggered input.
    // Don't enable interrupt just yet.
    GPIOPinTypeGPIOInput(GetPort(), GetPin());
    GPIOIntTypeSet(GetPort(), GetPin(), GPIO_BOTH_EDGES);
    GPIOPadConfigSet(
        GetPort(),
        GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD_WPU
    );

    // Enable the interrupt of the selected GPIO.
    // Don't enable the interrupt globally yet.
    GPIOPinIntEnable(GetPort(), GetPin());
    GPIOPinIntClear(GetPort(), GetPin());
}


Button::Button(
    GPIO          const &aGPIO,
    unsigned long const aIntNbr,
    unsigned int  const aID
)
    : GPIO(aGPIO)
    , mIntNbr(aIntNbr)
    , mID(aID) {

    // Ctor body left intentionally empty.
}


unsigned int Button::GetGPIOPinState(void) const {

    unsigned long lGPIOPin = GPIOPinRead(GetPort(), GetPin());
    unsigned int  lState = RELEASED;
    if (lGPIOPin & GetPin()) {
        lState = PRESSED;
    }

    return lState;
}


void Button::DisableInt(void) const {
    IntDisable(mIntNbr);
}


void Button::EnableInt(void) const {
    IntEnable(mIntNbr);
}


void Button::ClrInt(void) const {
    GPIOPinIntClear(GetPort(), GetPin());
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

unsigned int Button::PortToSysClockPeripheral(unsigned long aPort) {
    switch (aPort) {
    case GPIO_PORTA_BASE: return SYSCTL_PERIPH_GPIOA;
    case GPIO_PORTB_BASE: return SYSCTL_PERIPH_GPIOB;
    case GPIO_PORTC_BASE: return SYSCTL_PERIPH_GPIOC;
    case GPIO_PORTD_BASE: return SYSCTL_PERIPH_GPIOD;
    case GPIO_PORTE_BASE: return SYSCTL_PERIPH_GPIOE;
    case GPIO_PORTF_BASE: return SYSCTL_PERIPH_GPIOF;
    case GPIO_PORTG_BASE: return SYSCTL_PERIPH_GPIOG;
    case GPIO_PORTH_BASE: return SYSCTL_PERIPH_GPIOH;
    }

    return 0;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
