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
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <memory>

#include "IRTCC.h"
#include "INVMem.h"
#include "ITemperature.h"
#include "SPISlaveCfg.h"
#include "inc/GPIO.h"

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


//! \brief DS3234 Real-time clock.
//! Implements INVMem interface.
class DS3234
    : public IRTCC
    , public INVMem
    , public ITemperature {

 public:
    DS3234(
        unsigned int const aBaseYear,
        unsigned long const aInterruptNumber,
        GPIO const &aInterruptPin,
        std::shared_ptr<CoreLink::ISPIMasterDev> aSPIMasterDev,
        GPIO const &aCSnPin
    );
    ~DS3234() = default;

    // RTCC Interface.
    void Init(void) override;
    void SetInterrupt(bool aEnable) override;
    void AckInterrupt(void) override;
    void SetImpure(void) override { mIsImpure = true; }

    // Polled API.
    void RdTime(Time &aTime) override;
    void RdDate(Date &aDate) override;
    void RdTimeAndDate(Time &aTime, Date &aDate) override;

    void WrTime(Time const &aTime) override;
    void WrDate(Date const &aDate) override;
    void WrTimeAndDate(Time const &aTime, Date const &aDate) override;
    void GetTimeAndDate(Time &aTime, Date &aDate) override;

    bool WrAlarm(Time const &aTime, Date const &aDate) override;
    bool WrAlarm(Time const &aTime, Weekday const &aWeekday) override;
    bool IsAlarmOn(void) override;
    void DisableAlarm(void) override;
    void ClrAlarmFlag(void) override;

    // INVMem Interface.
    unsigned int GetNVMemSize(void) const override { return mNVMemSize; }
    void RdFromNVMem(uint8_t * const aDataPtr, unsigned int aOffset, unsigned int aSize) override;
    void WrToNVMem(uint8_t const * const aDataPtr, unsigned int aOffset, unsigned int aSize) override;

    // ITemperature Interface.
    float GetTemperature(void) override;

private:
    enum class ALARM_ID : unsigned int {
        ALARM_ID_1 = 0,
        ALARM_ID_2,
    };

#ifdef _WIN32
    typedef enum class ALARM_ID alarm_id_t;
#else
    typedef enum ALARM_ID alarm_id_t;
#endif

    enum class ALARM_MODE : unsigned int {
        ONCE_PER_SEC = 0,
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
    typedef enum class ALARM_MODE alarm_mode_t;
#else
    typedef enum ALARM_MODE alarm_mode_t;
#endif

    typedef uint8_t volatile rtcc_reg_t;

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

    typedef struct L_TIME_STRUCT_TAG time2_t;
    typedef struct L_DATE_STRUCT_TAG date_t;

    struct L_ALARM_STRUCT_TAG {
        rtcc_reg_t mMinutes;
        rtcc_reg_t mHours;
        rtcc_reg_t mDayDate;
    };

    typedef struct L_ALARM_STRUCT_TAG rtcc_alarm_t;


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

    typedef struct L_ADDR_MAP_STRUCT_TAG rtcc_reg_map_t;

    enum HoursFields : uint8_t {
        H12_24_n = (0x1 << 6),
        PM_AM_n  = (0x1 << 5),
    };

    enum MonthFields : uint8_t {
        CENTURY = (0x1 << 7),
    };

    enum DayDateFields : uint8_t {
        DAY_DATE_n = (0x1 << 6),
        AnMx       = (0x1 << 7)
    };

    enum Addr : uint8_t {
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
    enum Ctrl : uint8_t {
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
    enum Status : uint8_t {
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
    void UpdateTime(Time &aTime);
    void UpdateDate(Date &aDate);
    bool IsImpure(void) const { return mIsImpure; }

    void FillTimeStruct(Time const &aTimeRef);
    void FillDateStruct(Date const &aDateRef);
    void FillAlarmStruct(
        rtcc_alarm_t &aAlarmRef,
        Time const   &aTimeRef,
        Date const   &aDateRef
    );
    void FillAlarmStruct(
        rtcc_alarm_t   &aAlarmRef,
        Time    const  &aTimeRef,
        Weekday const  &aWeekdayRef
    );
    void FillAlarmTimeStruct(rtcc_alarm_t &aAlarmRef, Time const &aTimeRef);
    void FillAlarmModeStruct(rtcc_alarm_t &aAlarmRef, alarm_mode_t aAlarmMode);
    void TxAlarmStruct(alarm_id_t aAlarmID);

    void SetAlarm(alarm_id_t aAlarmID);
    void ClrAlarmFlag(alarm_id_t aAlarmID);
    bool WrAlarm(
        alarm_id_t aAlarmID,
        Time const &aTime,
        Date const &aDate,
        enum ALARM_MODE aAlarmMode
    );


    uint8_t GetCtrl(void);
    uint8_t GetStatus(void);

    unsigned int mBaseYear = 0;
    unsigned int mCentury = 0;

    std::shared_ptr<CoreLink::ISPIMasterDev> mSPIMasterDev;
    CoreLink::SPISlaveCfg const mSPISlaveCfg;

    unsigned long mInterruptNumber = 0;
    GPIO const &mInterruptGPIO;

    rtcc_reg_map_t mRegMap = {0};

    bool mIsImpure = true;

    static unsigned int const mNVMemSize = 256;
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
