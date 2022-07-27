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

// ******************************************************************************
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

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Describes a GPIO pin.
class GPIO {
public:
    constexpr explicit GPIO(unsigned long const aPort, unsigned int const aPin) noexcept
        : mPort(aPort), mPin(aPin) {}

    constexpr auto GetPort() const noexcept -> unsigned long {return mPort;}
    constexpr auto GetPin() const noexcept -> unsigned int {return mPin;}

    static void EnableSysCtlPeripheral(unsigned long aPort) noexcept;
    static void EnableSysCtlPeripheral(GPIO const &aGPIO) noexcept;

private:
    unsigned long mPort;
    unsigned int mPin;
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
