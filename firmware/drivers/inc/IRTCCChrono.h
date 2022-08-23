#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: RTCC.
//
// *******************************************************************************

//! \file
//! \brief RTCC interface class.
//! \ingroup ext_peripherals

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

#include <chrono>
#include <string>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

namespace WithChrono {


//! \brief RTCC interface.
class IRTCCChrono {
public:
    virtual ~IRTCCChrono();

    using Time = std::chrono::hh_mm_ss<std::chrono::seconds>;
    using Date = std::chrono::year_month_day;
    using Weekday = std::chrono::weekday;
    struct Entry_s {
        Time mTime{};
        Date mDate{};
    };
    using TimeAndDate = struct Entry_s;

    virtual void Init() = 0;
    virtual void SetInterrupt(bool aEnable) = 0;
    virtual void AckInterrupt() = 0;
    virtual void SetImpure() = 0;

    virtual auto RdTime() -> Time = 0;
    virtual auto RdDate() -> Date = 0;
    virtual auto RdTimeAndDate() -> TimeAndDate = 0;

    virtual void WrTime(Time const &aTime) = 0;
    virtual void WrDate(Date const &aDate) = 0;
    virtual void WrTimeAndDate(Time const &aTime, Date const &aDate) = 0;
    virtual auto GetCachedTimeAndDate() -> TimeAndDate = 0;

    virtual void WrAlarm(Time const &aTime, Date const &aDate) = 0;
    virtual void WrAlarm(Time const &aTime, Weekday const &aWeekday) = 0;
    virtual auto IsAlarmOn() -> bool = 0;
    virtual void DisableAlarm() = 0;
    virtual void ClrAlarmFlag() = 0;
};


} // namespace WithChrono

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
