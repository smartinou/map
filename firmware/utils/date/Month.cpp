// *****************************************************************************
//
// Project: Utils\Date.
//
// Module: Month class.
//
// *****************************************************************************

//! \file
//! \brief Simple month class.
//! \ingroup utils

// *****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "Month.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

char const * MonthHelper::ToStr(Month const &aMonth) {
  return MonthHelper::ToStr(aMonth.Get());
}


char const * MonthHelper::ToStr(unsigned int const aMonth) {
  switch (aMonth) {
    case  1: return "Jan";
    case  2: return "Feb";
    case  3: return "Mar";
    case  4: return "Apr";
    case  5: return "May";
    case  6: return "June";
    case  7: return "July";
    case  8: return "Aug";
    case  9: return "Sept";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
  }

  return nullptr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
