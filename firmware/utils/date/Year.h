#pragma once
// *******************************************************************************
//
// Project: Utils\Date.
//
// Module: Year class.
//
// *******************************************************************************

//! \file
//! \brief Simple year class.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
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

//! \brief Year class.
class Year
  : public Limit {
 public:
  explicit Year(unsigned int aVal = 2000)
    : Limit(2000, 2100, aVal) {}
  ~Year() {}
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
