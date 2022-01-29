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

//! \brief Month class.
class Month final
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
    constexpr explicit Month(const unsigned int aMonth) noexcept
        : Limit(
            Month::NameToUI(Name::January),
            Month::NameToUI(Name::December),
            aMonth) {}
    constexpr explicit Month(const Name aMonthName = Name::January) noexcept
        : Limit(
                Month::NameToUI(Name::January),
                Month::NameToUI(Name::December),
                NameToUI(aMonthName)) {}
    ~Month() = default;

    constexpr Name ToName(void) const {
        unsigned int lMonthUI = Limit::Get();
        return UIToName(lMonthUI);
    }

    static constexpr unsigned int NameToUI(const Name aMonthName) {
        unsigned int lMonthUI = static_cast<unsigned int>(aMonthName);
        return lMonthUI;
    }

    static constexpr Name UIToName(const unsigned int aMonth) {
        return static_cast<Name>(aMonth);
    }
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
    char const * ToStr(unsigned int const aMonth);
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
