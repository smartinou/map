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
    constexpr explicit Weekday(const unsigned int aVal) noexcept
        : Limit(
            Weekday::NameToUI(Name::Min),
            Weekday::NameToUI(Name::Max),
            aVal) {}
    constexpr explicit Weekday(const Name aWeekdayName = Name::Sunday) noexcept
        : Limit(
            Weekday::NameToUI(Name::Min),
            Weekday::NameToUI(Name::Max),
            Weekday::NameToUI(aWeekdayName)) {}
    ~Weekday() = default;

    constexpr Name ToName(void) const {
        unsigned int lWeekdayUI = Get();
        return UIToName(lWeekdayUI);
    }

    static constexpr unsigned int NameToUI(const Name aWeekdayName) {
        unsigned int lWeekdayUI = static_cast<unsigned int>(aWeekdayName);
        return lWeekdayUI;
    }

    static constexpr Name UIToName(const unsigned int aWeekday) {
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
