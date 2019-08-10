#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: Motor controller.
//
// *******************************************************************************

//! \file
//! \brief Motor controller interface class.
//! \ingroup ext_peripherals

// ******************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
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

//! \brief Brief description.
//! Details follow...
//! ...here.
class IMotorControl {
public:
    virtual ~IMotorControl() {}

    // Sets/clears the entry for the specified time, rounded to quarter hour.
    virtual void TurnOnCW(unsigned int const aDutyCycle = 100) const = 0;
    virtual void TurnOnCCW(unsigned int const aDutyCycle = 100) const = 0;
    virtual void TurnOff(void) const = 0;
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