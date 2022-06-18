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
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <array>
#include <string>

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
    constexpr explicit Month(unsigned int const aMonth = 1) noexcept
        : Limit(
            Month::NameToUI(Name::January),
            Month::NameToUI(Name::December),
            aMonth) {}
    constexpr explicit Month(Name const aMonthName = Name::January) noexcept
        : Month(NameToUI(aMonthName)) {}
    ~Month() = default;

    constexpr Name ToName(void) const noexcept {
        auto const lMonthUI = Limit::Get();
        return UIToName(lMonthUI);
    }

    constexpr std::string_view ToStr(void) const noexcept {
        using namespace std::string_view_literals;
        constexpr std::array<std::string_view, 12> lMonths = {
            "Jan"sv,
            "Feb"sv,
            "Mar"sv,
            "Apr"sv,
            "May"sv,
            "June"sv,
            "July"sv,
            "Aug"sv,
            "Sep"sv,
            "Oct"sv,
            "Nov"sv,
            "Dec"sv
        };

        return lMonths.at(Get());
    }

    static constexpr unsigned int NameToUI(Name const aMonthName) noexcept {
        return static_cast<unsigned int>(aMonthName);
    }

    static constexpr Name UIToName(unsigned int const aMonth) noexcept {
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

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
