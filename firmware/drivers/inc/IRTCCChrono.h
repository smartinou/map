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
    using Time = std::chrono::hh_mm_ss<std::chrono::seconds>;
    using Date = std::chrono::year_month_day;
    using Weekday = std::chrono::weekday;
    struct Entry_s {
        Time mTime{};
        Date mDate{};
    };
    using TimeAndDate = struct Entry_s;

    virtual void Init(void) = 0;
    virtual void SetInterrupt(bool const aEnable) = 0;
    virtual void AckInterrupt(void) = 0;
    virtual void SetImpure(void) = 0;

    virtual Time RdTime(void) = 0;
    virtual Date RdDate(void) = 0;
    virtual TimeAndDate RdTimeAndDate(void) = 0;

    virtual void WrTime(Time const &aTimeRef) = 0;
    virtual void WrDate(Date const &aDateRef) = 0;
    virtual void WrTimeAndDate(Time const &aTimeRef, Date const &aDateRef) = 0;
    virtual TimeAndDate GetCachedTimeAndDate(void) = 0;

    virtual void WrAlarm(Time const &aTimeRef, Date const &aDateRef) = 0;
    virtual void WrAlarm(Time const &aTimeRef, Weekday const &aWeekdayRef) = 0;
    virtual bool IsAlarmOn(void) = 0;
    virtual void DisableAlarm(void) = 0;
    virtual void ClrAlarmFlag(void) = 0;
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
