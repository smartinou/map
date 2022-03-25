#pragma once
// *******************************************************************************
//
// Project: Utilities.
//
// Module: Button.
//
// *******************************************************************************

//! \file
//! \brief Button class.
//! \ingroup utils_button

// ****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

//! \brief Button class.
class Button
    : public GPIO {
public:
    enum State {
        IS_LOW = 0,
        IS_HIGH = 1
    };

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

    enum State GetGPIOPinState(void) const;

    void DisableInt(void) const;
    void EnableInt(void) const;
    void ClrInt(void) const;

private:
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
