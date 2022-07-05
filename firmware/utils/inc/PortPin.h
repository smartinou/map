#pragma once
// *******************************************************************************
//
// Project: Utilities.
//
// Module: GPIO.
//
// *******************************************************************************

//! \file
//! \brief PortPin class.
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

//! \brief Describes a port pin.
// No invariants, so implement as struct.
struct PortPin {
    unsigned long mPort {0};
    unsigned int mPin {0};
#if 0
    explicit constexpr PortPin(
        unsigned long const aPort, unsigned int const aPin
    ) noexcept
        : mPort{aPort}, mPin{aPin} {}
    static void EnableSysCtlPeripheral(unsigned long aPort);
#endif
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
