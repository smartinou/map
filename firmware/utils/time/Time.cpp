// *****************************************************************************
//
// Project: Utilities.
//
// Module: Time class.
//
// *****************************************************************************

//! \file
//! \brief Class used for representation of time.
//! \ingroup utils

// *****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stdio.h>

#include "Time.h"

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

// TimeHelper functions.
char const *TimeHelper::ToStr(Time const &aTime, char * const aInStr) {

    if (aTime.Is24H()) {
        snprintf(aInStr, 5 + 1, "%02d:%02d", aTime.GetHours(), aTime.GetMinutes());
    } else if (aTime.IsPM()) {
        snprintf(aInStr, 8 + 1, "%02d:%02d PM", aTime.GetHours(), aTime.GetMinutes());
    } else {
        snprintf(aInStr, 8 + 1, "%02d:%02d AM", aTime.GetHours(), aTime.GetMinutes());
    }

    return aInStr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
