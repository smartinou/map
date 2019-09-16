#pragma once
// *******************************************************************************
//
// Project: Utilities.
//
// Module: GPIO.
//
// *******************************************************************************

//! \file
//! \brief GPIO class.
//! \ingroup utils_gpio

// ****************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "GPIO.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class Button
    : public GPIO {
public:
    Button(
        unsigned long const aPort,
        unsigned int  const aPin,
        unsigned long const aIntNbr,
        unsigned int  const aID
    );
    Button(
        GPIO          const &aGPIO,
        unsigned long const aIntNbr,
        unsigned int  const aID
    );

    unsigned int GetGPIOPinState(void) const;

    void DisableInt(void) const;
    void EnableInt(void) const;
    void ClrInt(void) const;

    enum State {
        RELEASED = 0,
        PRESSED  = 1
    };

private:
    static unsigned int PortToSysClockPeripheral(unsigned long aPort);

    unsigned long const mIntNbr;
    unsigned int  const mID;
};

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
