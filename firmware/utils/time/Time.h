#pragma once
// *******************************************************************************
//
// Project: Utilities\Time.
//
// Module: Time class.
//
// *******************************************************************************

//! \file
//! \brief Class used for representation of time.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <string>
#include <time.h>

#include "Hour.h"
#include "Minute.h"
#include "Second.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Time class, as an aggregate of other classes.
// This is a simple class, meant to represent time snapshots,
// from an RTCC for example.
// It will not properly overflow and keep time if two objects
// are added together.
class Time final {
public:
    constexpr explicit Time(
        Hour const &aHours,
        Minute const &aMinutes,
        Second const &aSeconds,
        bool const aIs24H = true,
        bool const aIsPM  = false
    ) noexcept
        : mHours(aHours)
        , mMinutes(aMinutes)
        , mSeconds(aSeconds)
        , mIs24H(aIs24H)
        , mIsPM(aIsPM) {}

    constexpr explicit Time(
        unsigned const int aHours = 0,
        unsigned const int aMinutes = 0,
        unsigned const int aSeconds = 0,
        bool const aIs24H = true,
        bool const aIsPM  = false
    ) noexcept
        : mHours(aHours)
        , mMinutes(aMinutes)
        , mSeconds(aSeconds)
        , mIs24H(aIs24H)
        , mIsPM(aIsPM) {}

    constexpr explicit Time(struct tm const * const aTime) noexcept
        : mHours(aTime->tm_hour)
        , mMinutes(aTime->tm_min)
        , mSeconds(aTime->tm_sec)
        , mIs24H(true)
        , mIsPM(false) {}
    ~Time() = default;

    constexpr unsigned int GetHours(void) const noexcept {return mHours.Get();}
    constexpr unsigned int GetMinutes(void) const noexcept {return mMinutes.Get();}
    constexpr unsigned int GetSeconds(void) const noexcept {return mSeconds.Get();}

    constexpr bool Is24H(void) const noexcept {return mIs24H;}
    constexpr bool IsPM(void)  const noexcept {return mIsPM;}

    std::string ToStr(void) const;

private:
    Hour mHours;
    Minute mMinutes;
    Second mSeconds;
    bool mIs24H;
    bool mIsPM;
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
