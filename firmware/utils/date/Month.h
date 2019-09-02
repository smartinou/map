#pragma once
// *******************************************************************************
//
// Project: Utils\Date.
//
// Module: Month class.
//
// *******************************************************************************

//! \file
//! \brief Simple month class.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
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

//! \brief Month class.
class Month
  : public Limit {
 public:
  enum class Name : unsigned int {
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December,
    Min = January,
    Max = December
  };

 public:
  explicit Month(unsigned int aMonth);
  explicit Month(Name aMonthName = Name::January);
  ~Month() {}

  Name ToName(void) const;

  static unsigned int NameToUI(Name aMonthName);
  static Name         UIToName(unsigned int aMonth);
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

namespace MonthHelper {
  char const * ToStr(Month const &aMonth);
  char const * ToStr(unsigned int aMonth);
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
