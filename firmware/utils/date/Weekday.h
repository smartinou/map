#pragma once
// *******************************************************************************
//
// Project: Utils\Date.
//
// Module: Weekday class.
//
// *******************************************************************************

//! \file
//! \brief Simple weekday class.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "Limit.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Weekday class.
class Weekday
  : public Limit {
 public:
  enum class Name : unsigned int {
    Sunday    = 1,
    Monday    = 2,
    Tuesday   = 3,
    Wednesday = 4,
    Thursday  = 5,
    Friday    = 6,
    Saturday  = 7,
    Min       = Sunday,
    Max       = Saturday,
  };

 public:
  explicit Weekday(unsigned int aVal);
  explicit Weekday(Name aWeekdayName = Name::Sunday);
  ~Weekday() {}

  unsigned int Get(void) const;
  Name ToName(void) const;

  static unsigned int NameToUI(Name aWeekdayName);
  static Name         UIToName(unsigned int aWeekday);
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
