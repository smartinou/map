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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

//! \brief Weekday class.
class Weekday final
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
    constexpr explicit Weekday(unsigned const int aVal) noexcept
        : Limit(
            Weekday::NameToUI(Name::Min),
            Weekday::NameToUI(Name::Max),
            aVal) {}
    constexpr explicit Weekday(Name const aWeekdayName = Name::Sunday) noexcept
        : Limit(
            Weekday::NameToUI(Name::Min),
            Weekday::NameToUI(Name::Max),
            Weekday::NameToUI(aWeekdayName)) {}
    ~Weekday() = default;

    constexpr Name ToName(void) const noexcept {
        auto const lWeekdayUI = Get();
        return UIToName(lWeekdayUI);
    }

    static constexpr unsigned int NameToUI(Name const aWeekdayName) noexcept {
        return static_cast<unsigned int>(aWeekdayName);
    }

    static constexpr Name UIToName(unsigned const int aWeekday) noexcept {
        return static_cast<Name>(aWeekday);
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
