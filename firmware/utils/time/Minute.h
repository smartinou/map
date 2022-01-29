#pragma once
// *******************************************************************************
//
// Project: Utils\Time.
//
// Module: Minute class.
//
// *******************************************************************************

//! \file
//! \brief Simple minute class.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "inc/Limit.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Minute class.
class Minute final
    : public Limit {
public:
    constexpr explicit Minute(const unsigned int aVal = 0) noexcept
        : Limit(0, 59, aVal) {}
    ~Minute() = default;
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
