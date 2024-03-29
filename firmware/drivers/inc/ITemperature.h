#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: Temperature.
//
// *******************************************************************************

//! \file
//! \brief Non-volatile memory interface class.
//! \ingroup ext_peripherals

// ******************************************************************************
//
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
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


//! \brief Temperature interface.
class ITemperature {
public:
    virtual ~ITemperature() {}

    virtual float GetTemperature(void) = 0;
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
