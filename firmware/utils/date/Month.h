#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Month class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

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

//! \brief Brief description.
//! Details follow...
//! ...here.
class Month : public Limit {
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

  unsigned int Get(void) const override;
  Name ToName(void) const;

  static unsigned int NameToUI(Name aMonthName);
  static Name         UIToName(unsigned int aMonth);

 private:
  // Disable default and copy ctor.
  //Month();
  Month(const Month &) = delete;
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
