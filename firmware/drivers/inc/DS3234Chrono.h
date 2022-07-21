#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: DS3234 RTCC.
//
// *******************************************************************************

//! \file
//! \brief DS3234 RTCC.
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

#include <cstddef>
#include <memory>

#include "IRTCCChrono.h"
#include "INVMem.h"
#include "ITemperature.h"
#include "SPISlaveCfg.h"
#include "inc/PortPin.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declaration.
namespace CoreLink {
    class ISPIMasterDev;
}


namespace WithChrono {


//! \brief DS3234 Real-time clock.
//! Implements INVMem interface.
class DS3234
    : public IRTCCChrono
    , public INVMem
    , public ITemperature {

 public:
    DS3234(
        int const aBaseYear,
        unsigned long const aInterruptNumber,
        PortPin const &aInterruptPin,
        std::shared_ptr<CoreLink::ISPIMasterDev> const aSPIMasterDev,
        PortPin const &aCSnPin
    );

    // RTCC Interface.
    void Init(void) override;
    void SetInterrupt(bool const aEnable) override;
    void AckInterrupt(void) override;
    void SetImpure(void) override { mIsImpure = true; }

    // Polled API.
    //void RdTime(Time &aTime) override;
    //void RdDate(Date &aDate) override;
    //void RdTimeAndDate(Time &aTime, Date &aDate) override;
    Time RdTime(void) override;
    Date RdDate(void) override;
    TimeAndDate RdTimeAndDate(void) override;

    //void WrTime(Time const &aTime) override;
    //void WrDate(Date const &aDate) override;
    //void WrTimeAndDate(Time const &aTime, Date const &aDate) override;
    //void GetTimeAndDate(Time &aTime, Date &aDate) override;
    void WrTime(Time const &aTime) override;
    void WrDate(Date const &aDate) override;
    //void WrTimeAndDate(TimeAndDate const &aTimeAndDate) override;
    void WrTimeAndDate(Time const &aTime, Date const &aDate) override;
    // TBD QUOI FAIRE AVEC GetTimeAndDate().
    TimeAndDate GetCachedTimeAndDate(void) override;

    //bool WrAlarm(Time const &aTime, Date const &aDate) override;
    //bool WrAlarm(Time const &aTime, Weekday const &aWeekday) override;
    void WrAlarm(Time const &aTime, Date const &aDate) override;
    void WrAlarm(Time const &aTime, Weekday const &aWeekday) override;
    bool IsAlarmOn(void) override;
    void DisableAlarm(void) override;
    void ClrAlarmFlag(void) override;

    // INVMem Interface.
    std::size_t GetNVMemSize(void) const override { return mNVMemSize; }
    void RdFromNVMem(
        uint8_t * const aDataPtr,
        std::size_t const aOffset,
        std::size_t const aSize
    ) override;
    void WrToNVMem(
        uint8_t const * const aDataPtr,
        std::size_t const aOffset,
        std::size_t const aSize
    ) override;

    // ITemperature Interface.
    float GetTemperature(void) override;

private:
    enum class ALARM_ID {
        ALARM_ID_1,
        ALARM_ID_2,
    };

#ifdef _WIN32
    using alarm_id_t = enum class ALARM_ID;
#else
    using alarm_id_t = enum ALARM_ID;
#endif

    enum class ALARM_MODE {
        ONCE_PER_SEC,
        WHEN_SECS_MATCH,
        ONCE_PER_MINUTE = WHEN_SECS_MATCH,
        WHEN_MINS_SECS_MATCH,
        WHEN_MINS_MATCH = WHEN_MINS_SECS_MATCH,
        WHEN_HOURS_MINS_SECS_MATCH,
        WHEN_HOURS_MINS_MATCH = WHEN_HOURS_MINS_SECS_MATCH,
        WHEN_DAY_HOURS_MINS_SECS_MATCH,
        WHEN_DAY_HOURS_MINS_MATCH = WHEN_DAY_HOURS_MINS_SECS_MATCH,
        WHEN_DATE_HOURS_MINS_SECS_MATCH,
        WHEN_DATE_HOURS_MINS_MATCH = WHEN_DATE_HOURS_MINS_SECS_MATCH
    };
#ifdef _WIN32
    using alarm_mode_t = enum class ALARM_MODE;
#else
    using alarm_mode_t = enum ALARM_MODE;
#endif

    using rtcc_reg_t = std::byte;

    struct L_TIME_STRUCT_TAG {
        rtcc_reg_t mSeconds;
        rtcc_reg_t mMinutes;
        rtcc_reg_t mHours;
    };


    struct L_DATE_STRUCT_TAG {
        rtcc_reg_t mWeekday;
        rtcc_reg_t mDate;
        rtcc_reg_t mMonth;
        rtcc_reg_t mYear;
    };

    using time2_t = struct L_TIME_STRUCT_TAG;
    using date_t = struct L_DATE_STRUCT_TAG;

    struct L_ALARM_STRUCT_TAG {
        rtcc_reg_t mMinutes;
        rtcc_reg_t mHours;
        rtcc_reg_t mDayDate_n;
    };

    using rtcc_alarm_t = struct L_ALARM_STRUCT_TAG;


    struct L_ADDR_MAP_STRUCT_TAG {
        time2_t      mTime;
        date_t       mDate;
        rtcc_reg_t   mAlarm1Seconds;
        rtcc_alarm_t mAlarm1;
        rtcc_alarm_t mAlarm2;
        rtcc_reg_t   mCtrl;
        rtcc_reg_t   mStatus;
        rtcc_reg_t   mAgingOffset;
        rtcc_reg_t   mTempMSB;
        rtcc_reg_t   mTempLSB;
        rtcc_reg_t   mDisableTemp;
        rtcc_reg_t   mReserved[4];
        rtcc_reg_t   mSRAMAddr;
        rtcc_reg_t   mSRAMData;
    };

    using rtcc_reg_map_t = struct L_ADDR_MAP_STRUCT_TAG;

    enum class HoursFields {
        H12_24_n = (0x1 << 6),
        PM_AM_n  = (0x1 << 5),
    };

    enum class MonthFields {
        CENTURY = (0x1 << 7),
    };

    enum class DayDateFields {
        DAY_DATE_n = (0x1 << 6),
        AnMx       = (0x1 << 7)
    };

    enum Addr : unsigned int {
        WR_BASE_ADDR = 0x80
    };

    // public:
    /* -------------------------------------------------------------------------
     Control Register : 0x0E/8Eh
     Name    Value       Description
     ----    ---------   -------------------------------------------------------
     EOSCn   x--- ----   Enable Oscillator.
     BBSQW   -x-- ----   Battery-Backed Square-Wave Enable.
     CONV    --x- ----   Convert Temperature.
     RS2     ---x ----   Rate selection bit 1.
     RS1     ---- x---   Rate selection bit 2.
     INTCn   ---- -x--   Interrupt Control.
     AEI2    ---- --x-   Alarm 2 Interrupt Enable.
     AEI1    ---- ---x   Alarm 1 Interrupt Enable.
     ------------------------------------------------------------------------- */
    enum class Ctrl {
        AEI1  = (0x1 << 0),
        AEI2  = (0x1 << 1),
        INTCn = (0x1 << 2),
        RS1   = (0x1 << 3),
        RS2   = (0x1 << 4),
        CONV  = (0x1 << 5),
        BBSQW = (0x1 << 6),
        EOSCn = (0x1 << 7)
    };

    /* -------------------------------------------------------------------------
     Control/Status Register : 0x0F/8Fh
     Name    Value       Description
     ----    ---------   -------------------------------------------------------
     OSF     x--- ----   Oscillator Stop Flag.
     BB32K   -x-- ----   Battery-Backed 32KHz Output.
     CRATE1  --x- ----   Conversion Rate 1.
     CRATE0  ---x ----   Conversion Rate 2.
     EN32K   ---- x---   Enable 32KHz Output.
     BSY     ---- -x--   Busy.
     AF2     ---- --x-   Alarm 2 Flag.
     AF1     ---- ---x   Alarm 1 Flag.
     ------------------------------------------------------------------------- */
    enum class Status {
        AF1    = (0x1 << 0),
        AF2    = (0x1 << 1),
        BSY    = (0x1 << 2),
        EN32K  = (0x1 << 3),
        CRATE0 = (0x1 << 4),
        CRATE1 = (0x1 << 5),
        BB32K  = (0x1 << 6),
        ESF    = (0x1 << 7)
    };

private:
    void UpdateCachedVal(void);
    Time UpdateTime(void);
    Date UpdateDate(void) const;
    bool IsImpure(void) const { return mIsImpure; }

    void FillTimeStruct(Time const &aTime);
    void FillDateStruct(Date const &aDate);
    rtcc_alarm_t FillAlarmStruct(Time const &aTime, Date const &aDate);
    rtcc_alarm_t FillAlarmStruct(Time const &aTime, Weekday const &aWeekday);
    rtcc_alarm_t FillAlarmModeStruct(rtcc_alarm_t const &aAlarmStruct, alarm_mode_t const aAlarmMode);
    void TxAlarmStruct(alarm_id_t const aAlarmID);

    void SetAlarm(alarm_id_t const aAlarmID);
    void ClrAlarmFlag(alarm_id_t const aAlarmID);
    void WrAlarm(
        alarm_id_t const aAlarmID,
        Time const &aTime,
        Date const &aDate,
        enum ALARM_MODE const aAlarmMode
    );

    rtcc_reg_t GetCtrl(void);
    rtcc_reg_t GetStatus(void);

    int const mBaseYear{2000};

    std::shared_ptr<CoreLink::ISPIMasterDev> const mSPIMasterDev;
    CoreLink::SPISlaveCfg const mSPISlaveCfg;

    unsigned long const mInterruptNumber;
    PortPin const mInterruptGPIO;

    rtcc_reg_map_t mRegMap{std::byte{0}};

    bool mIsImpure{true};

    static unsigned int const mNVMemSize{256};
};

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

namespace DS3234Helper {
constexpr std::byte BinaryToBCD(uint8_t const aByte) {
    auto const lLowNibble = aByte % 10;
    auto const lHighNibble = aByte / 10;

    return std::byte(lHighNibble << 4 | lLowNibble);
}

constexpr uint8_t BCDToBinary(std::byte const &aBCD) {
    auto const lBCD = std::to_integer<uint8_t>(aBCD);
    auto const lUnits = lBCD & 0x0F;
    auto const lDecimals = (lBCD & 0xF0) * 10;

    return lDecimals + lUnits;
}


} // namespace DS3234Helper

} // namespace WithChrono

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
