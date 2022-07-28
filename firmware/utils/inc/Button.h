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
#include "PortPin.h"

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

    explicit Button(
        GPIO const &aGPIO,
        unsigned long aIntNbr,
        unsigned int aID
    ) noexcept;

    auto GetGPIOPinState() const -> enum State;

    void DisableInt() const;
    void EnableInt() const;
    void ClrInt() const;

private:
    unsigned long mIntNbr;
    unsigned int mID;
};


// [MG] IS THIS A VARIANT TYPE OR NOT?
// [MG] IF SO, THEN USE THE CLASS ABOVE.
struct Button_s
    : PortPin
{
    enum State {
        IS_LOW = 0,
        IS_HIGH = 1
    };

    unsigned long mIntNbr{};
    unsigned int mID{};

    auto GetGPIOPinState() const -> enum State;

    void DisableInt() const;
    void EnableInt() const;
    void ClrInt() const;
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
