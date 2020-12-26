// *****************************************************************************
//
// Project: Drivers.
//
// Module: DS3234 RTCC.
//
// *****************************************************************************

//! \file
//! \brief DS3234 RTCC.
//! \ingroup ext_peripherals

// *****************************************************************************
//
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stddef.h>
#include <stdio.h>

// TI Library.
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

#include "SPI.h"

#include "DS3234.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

#define L_RD_ADDR(m) \
    static_cast<uint8_t>(offsetof(rtcc_reg_map_t, m))

#define L_WR_ADDR(m) \
    static_cast<uint8_t>(offsetof(rtcc_reg_map_t, m) + WR_BASE_ADDR)

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

enum L_LIMITS {
    SECONDS_DEFAULT  = 0,
    SECONDS_MAX      = 59,
    MINUTES_DEFAULT  = 0,
    MINUTES_MAX      = 59,
    HOURS_DEFAULT    = 12,
    HOURS_12_MIN     = 1,
    HOURS_12_MAX     = 12,
    HOURS_12_DEFAULT = HOURS_12_MIN,
    HOURS_24_MIN     = 0,
    HOURS_24_MAX     = 23,
    HOURS_24_DEFAULT = HOURS_24_MIN,
    WEEKDAY_MIN      = 1,
    WEEKDAY_MAX      = 7,
    WEEKDAY_DEFAULT  = WEEKDAY_MIN,
    DAY_MIN          = 1,
    DAY_MAX          = 31,
    DAY_DEFAULT      = DAY_MIN,
    MONTH_MIN        = 1,
    MONTH_MAX        = 12,
    MONTH_DEFAULT    = MONTH_MIN,
    YEARS_MIN        = 0,
    YEARS_MAX        = 99,
    YEARS_DEFAULT    = YEARS_MIN
};

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// Ctor.
DS3234::DS3234(
    unsigned int const aBaseYear,
    unsigned long const aInterruptNumber,
    GPIO const &aInterruptPin,
    CoreLink::ISPIDev &aSPIDev,
    GPIO const &aCSnPin
)
    : mBaseYear(aBaseYear)
    , mSPIDev(aSPIDev)
    , mSPICfg(aCSnPin)
    , mInterruptNumber(aInterruptNumber)
    , mInterruptGPIO(aInterruptPin) {

    // Create an SPI slave to operate at maximum device speed.
    mSPICfg.SetProtocol(CoreLink::ISPISlaveCfg::PROTOCOL::MOTO_1);
    mSPICfg.SetBitRate(4000000);
    mSPICfg.SetDataWidth(8);
    //mSPICfg.SetCSnGPIO(aCSnPin.GetPort(), aCSnPin.GetPin());
}


DS3234::~DS3234() {

    // Dtor body intentionally left empty.
}


void DS3234::Init(void) {

    // Configure the RTC to desired state:
    //   -Oscillator enabled.
    //   -Battery-backed square wave disabled.
    //   -Interrupt enabled (square wave disabled).
    //   -Alarm 'N' disabled.
    mRegMap.mCtrl = Ctrl::INTCn;

    // Disable all and clear all flags in status regiser.
    mRegMap.mStatus = 0x00;
    mSPIDev.WrData(
        L_WR_ADDR(mCtrl),
        const_cast<uint8_t *>(&mRegMap.mCtrl),
        2 * sizeof(rtcc_reg_t),
        mSPICfg
    );
}

// TODO: CONSIDER REMOVING.
#if 0
void DS3234::IsValid(void) {

    // Check for valid time.
    bool lIsTimeInvalid = false;
    unsigned int lSeconds = mRegMap.mTime.mSeconds;
    if (SECONDS_MAX < lSeconds) {
        mRegMap.mTime.mSeconds = BinaryToBCD(SECONDS_DEFAULT);
        lIsTimeInvalid = true;
    }

    unsigned int lMinutes = mRegMap.mTime.mMinutes;
    if (MINUTES_MAX < lMinutes) {
        mRegMap.mTime.mMinutes = BinaryToBCD(MINUTES_DEFAULT);
        lIsTimeInvalid = true;
    }

    // Here should make a distinction between 12H vs 24H mode.
    unsigned int lHours = mRegMap.mTime.mHours;
    if (mRegMap.mTime.mHours & HoursFields::H12_24_n) {
        if ((lHours < HOURS_12_MIN) || (HOURS_12_MAX < lHours)) {
        mRegMap.mTime.mHours = BinaryToBCD(HOURS_12_DEFAULT);
        lIsTimeInvalid = true;
        }
    } else {
        if ((lHours < HOURS_24_MIN) || (HOURS_24_MAX < lHours)) {
        mRegMap.mTime.mHours = BinaryToBCD(HOURS_24_DEFAULT);
        lIsTimeInvalid = true;
        }
    }

    // Check for valid date.
    // Limit this to allowed maximum value.
    // Not attempting to relate with month.
    bool lIsDateInvalid = false;
    unsigned int lWeekday = BCDToBinary(mRegMap.mDate.mWeekday);
    if ((lWeekday < WEEKDAY_MIN ) || (WEEKDAY_MAX < lWeekday)) {
        mRegMap.mDate.mWeekday = BinaryToBCD(WEEKDAY_DEFAULT);
        lIsDateInvalid = true;
    }

    unsigned int lDay = BCDToBinary(mRegMap.mDate.mDate);
    if ((lDay < DAY_MIN) || (DAY_MAX < lDay)) {
        mRegMap.mDate.mDate = BinaryToBCD(DAY_DEFAULT);
        lIsDateInvalid = true;
    }

    unsigned int lMonth = BCDToBinary(mRegMap.mDate.mMonth & MonthFields::CENTURY);
    if((lMonth < MONTH_MIN) || (MONTH_MAX < lMonth)) {
        mRegMap.mDate.mMonth = BinaryToBCD(MONTH_DEFAULT);
        lIsDateInvalid = true;
    }

    unsigned int lYear = BCDToBinary(mRegMap.mDate.mYear);
    if (YEARS_MAX < lYear) {
        mRegMap.mDate.mYear = BinaryToBCD(YEARS_DEFAULT);
        lIsDateInvalid = true;
    }

    if (lIsTimeInvalid) {

        if (lIsDateInvalid) {
            // Write back both new valid time & date.
        } else {
        // Write back new valid time.
        }
    } else if (lIsDateInvalid) {
        // Write back new valid date.
    }
}
#endif


void DS3234::SetInterrupt(bool aEnable) {

    MAP_IntDisable(mInterruptNumber);
    if (aEnable) {
        MAP_GPIOPinTypeGPIOInput(mInterruptGPIO.GetPort(), mInterruptGPIO.GetPin());
        MAP_GPIOIntTypeSet(
            mInterruptGPIO.GetPort(),
            mInterruptGPIO.GetPin(),
            GPIO_FALLING_EDGE
        );
        MAP_GPIOPadConfigSet(
            mInterruptGPIO.GetPort(),
            mInterruptGPIO.GetPin(),
            GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD
        );
#ifdef USE_TIVAWARE
        MAP_GPIOIntEnable(mInterruptGPIO.GetPort(), mInterruptGPIO.GetPin());
#elif defined (USE_STELLARISWARE)
        MAP_GPIOPinIntEnable(mInterruptGPIO.GetPort(), mInterruptGPIO.GetPin());
#endif
        // [MG] THIS CLEARS THE 1ST INTERRUPT. IT DOESN'T COME UP UNTIL FLAGS ARE CLEARED.
        //MAP_GPIOPinIntClear(mInterrupt->GetPort(), lGPIOInitEvtPtr->mIRQGPIOPin);

        MAP_IntEnable(mInterruptNumber);

        // Alarm1 is dedicated to periodic interrupt.
        Time lTime;
        Date lDate;
        WrAlarm(
            ALARM_ID::ALARM_ID_1,
            lTime,
            lDate,
            // This is also where we can throttle down the frequency of clock updates.
            // Instead of using square wave, the alarm is triggered periodically.
            ALARM_MODE::ONCE_PER_SEC
        );
    }
}


void DS3234::AckInterrupt(void) {
    // It is assumed that all interrupts where serviced at this point.
    // Clear all flags.
    ClrAlarmFlag(ALARM_ID::ALARM_ID_1);
    ClrAlarmFlag(ALARM_ID::ALARM_ID_2);
}


void DS3234::UpdateCachedVal(void) {

    if (IsImpure()) {
        // Read the whole RTC.
        mSPIDev.RdData(
            0,
            reinterpret_cast<uint8_t *>(&mRegMap),
            sizeof(rtcc_reg_map_t),
            mSPICfg
        );
        mIsImpure = false;
    }
}


void DS3234::RdTime(Time &aTime) {

    // Read the Time portion of the RTC.
    // Update time ref.
    mSPIDev.RdData(
        L_RD_ADDR(mTime),
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time2_t),
        mSPICfg
    );
    UpdateTime(aTime);
}


void DS3234::RdDate(Date &aDate) {

    // Read the date portion out of RTC.
    // Update date ref.
    mSPIDev.RdData(
        L_RD_ADDR(mDate),
        reinterpret_cast<uint8_t *>(&mRegMap.mDate),
        sizeof(date_t),
        mSPICfg
    );
    UpdateDate(aDate);
}


void DS3234::RdTimeAndDate(Time &aTime, Date &aDate) {

    // Read the whole RTC into register map structure.
    // Update time ref.
    // Update date ref.
    mSPIDev.RdData(
        L_RD_ADDR(mTime),
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time2_t) + sizeof(date_t),
        mSPICfg
    );

    UpdateTime(aTime);
    UpdateDate(aDate);
}


void DS3234::WrTime(Time const &aTime) {

    // Fill time structure to write to RTC.
    // Send only the time portion of the structure to RTC.
    FillTimeStruct(aTime);
    mSPIDev.WrData(
        L_WR_ADDR(mTime),
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time2_t),
        mSPICfg
    );
}


void DS3234::WrDate(Date const &aDate) {

    // Fill date structure to write to RTC.
    // Send only the date portion of the structure to RTC.
    FillDateStruct(aDate);
    mSPIDev.WrData(
        L_WR_ADDR(mDate),
        reinterpret_cast<uint8_t *>(&mRegMap.mDate),
        sizeof(date_t),
        mSPICfg
    );
}


void DS3234::WrTimeAndDate(Time const &aTime, Date const &aDate) {

    // Fill time structure to write to RTC.
    // Fill date structure to write to RTC.
    // Send time and date portion of the structure to RTC.
    FillTimeStruct(aTime);
    FillDateStruct(aDate);
    mSPIDev.WrData(
        L_WR_ADDR(mTime),
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time2_t) + sizeof(date_t),
        mSPICfg
    );
}


void DS3234::GetTimeAndDate(Time &aTime, Date &aDate) {

    // Update date ref.
    UpdateCachedVal();
    UpdateTime(aTime);
    UpdateDate(aDate);
}


float DS3234::GetTemperature(void) {

    // Return temperature field.
    // Convert temperature MSB and LSB to float.
    UpdateCachedVal();
    float lTempFloat = 0.25f * (mRegMap.mTempLSB >> 6);
    lTempFloat += static_cast<float>(mRegMap.mTempMSB);
    return lTempFloat;
}


bool DS3234::WrAlarm(Time const &aTime, Date const &aDate) {

    // Public alarm is triggered on time match.
    // Alarm1 is dedicated to periodic time/date interrupt.
    return WrAlarm(ALARM_ID::ALARM_ID_2, aTime, aDate, ALARM_MODE::WHEN_DATE_HOURS_MINS_SECS_MATCH);
}


bool DS3234::WrAlarm(Time const &aTime, Weekday const &aWeekdayRef) {

    // Hard-coded to use alarm2, but provides template for use with any alarm.
    alarm_id_t constexpr aAlarmID = ALARM_ID::ALARM_ID_2;
    alarm_mode_t constexpr aAlarmMode = ALARM_MODE::WHEN_DAY_HOURS_MINS_SECS_MATCH;
    // Fill alarm structure to write to RTC.
    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1:
        mRegMap.mAlarm1Seconds = BinaryToBCD(aTime.GetSeconds());
        FillAlarmStruct(mRegMap.mAlarm1, aTime, aWeekdayRef);
        FillAlarmModeStruct(mRegMap.mAlarm1, aAlarmMode);
    break;

    case ALARM_ID::ALARM_ID_2:
        FillAlarmStruct(mRegMap.mAlarm2, aTime, aWeekdayRef);
        FillAlarmModeStruct(mRegMap.mAlarm2, aAlarmMode);
    break;

    default:
        return false;
    }

    // Send alarm and control portion of the structure to the RTC.
    SetAlarm(aAlarmID);
    TxAlarmStruct(aAlarmID);
    return true;
}


bool DS3234::IsAlarmOn(void) {

    // Hard-coded to use alarm2, but provides template for use with any alarm.
    alarm_id_t constexpr aAlarmID = ALARM_ID::ALARM_ID_2;
    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1:
        if ((DS3234::AF1  & GetStatus()) && (DS3234::AEI1 & GetCtrl())) {
            return true;
        }
    return false;

    case ALARM_ID::ALARM_ID_2:
        if ((DS3234::AF2  & GetStatus()) && (DS3234::AEI2 & GetCtrl())) {
            return true;
        }
    // Intentional fallthrough.
    default:
        return false;
    }

    return false;
}


void DS3234::DisableAlarm(void) {

    // Hard-coded to use alarm2, but provides template for use with any alarm.
    alarm_id_t constexpr aAlarmID = ALARM_ID::ALARM_ID_2;
    // Only clear interrupt.
    // We don't care if the Alarm fields are set,
    // as long as the interrupt is not generated.
    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1: mRegMap.mCtrl &= ~AEI1; break;
    case ALARM_ID::ALARM_ID_2: mRegMap.mCtrl &= ~AEI2; break;
    }

    mSPIDev.WrData(
        L_WR_ADDR(mCtrl),
        const_cast<uint8_t const *>(&mRegMap.mCtrl),
        sizeof(rtcc_reg_t),
        mSPICfg
    );
}


void DS3234::ClrAlarmFlag(void) {

    ClrAlarmFlag(ALARM_ID::ALARM_ID_2);
}


void DS3234::RdFromNVMem(
    uint8_t * const aDataPtr,
    unsigned int const aOffset,
    unsigned int aSize
) {

    // Write SRAM address register.
    // Cap size.
    // Loop into data register.
    mSPIDev.WrData(
        L_WR_ADDR(mSRAMAddr),
        reinterpret_cast<uint8_t *>(aOffset),
        sizeof(rtcc_reg_t),
        mSPICfg
    );

    unsigned int lMaxSize = mNVMemSize - aOffset;
    if (aSize > lMaxSize) {
        aSize = lMaxSize;
    }

    mSPIDev.RdData(
        L_RD_ADDR(mSRAMData),
        reinterpret_cast<uint8_t *>(aDataPtr),
        aSize * sizeof(rtcc_reg_t),
        mSPICfg
    );
}


void DS3234::WrToNVMem(
    uint8_t const * const aDataPtr,
    unsigned int const aOffset,
    unsigned int aSize
) {

    // Write SRAM address register.
    // Cap size.
    // Loop into data register.
    mSPIDev.WrData(
        L_WR_ADDR(mSRAMAddr),
        reinterpret_cast<uint8_t *>(aOffset),
        sizeof(rtcc_reg_t),
        mSPICfg
    );

    unsigned int lMaxSize = mNVMemSize - aOffset;
    if (aSize > lMaxSize) {
        aSize = lMaxSize;
    }

    mSPIDev.WrData(
        L_WR_ADDR(mSRAMData),
        reinterpret_cast<uint8_t const *>(aDataPtr),
        aSize * sizeof(rtcc_reg_t),
        mSPICfg
    );
}


// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void DS3234::UpdateTime(Time &aTime) {

    // Apply proper mask and convert where needed.
    unsigned int lSeconds = BCDToBinary(mRegMap.mTime.mSeconds);
    unsigned int lMinutes = BCDToBinary(mRegMap.mTime.mMinutes);
    bool lIs24H = true;
    bool lIsPM  = false;
    unsigned int lHours = 0;
    if (mRegMap.mTime.mHours & HoursFields::H12_24_n) {
        lIs24H = false;
        if (mRegMap.mTime.mHours & HoursFields::PM_AM_n) {
            lIsPM = true;
        }
        lHours = BCDToBinary(mRegMap.mTime.mHours & ~(HoursFields::H12_24_n | HoursFields::PM_AM_n));
    } else {
        lHours = BCDToBinary(mRegMap.mTime.mHours & ~HoursFields::H12_24_n);
    }

    // "Return" via copy ctor.
    Time lTime(lHours, lMinutes, lSeconds, lIs24H, lIsPM);
    aTime = lTime;
}


void DS3234::UpdateDate(Date &aDate) {

    unsigned int lWeekday = BCDToBinary(mRegMap.mDate.mWeekday);
    unsigned int lDay     = BCDToBinary(mRegMap.mDate.mDate);
    unsigned int lMonth   = BCDToBinary(mRegMap.mDate.mMonth & ~MonthFields::CENTURY);
    unsigned int lYear    = BCDToBinary(mRegMap.mDate.mYear);
    if (mRegMap.mDate.mMonth & MonthFields::CENTURY) {
        mCentury += 100;
    }

    Month::Name lMonthName = Month::UIToName(lMonth);
    Weekday::Name lWeekdayName = Weekday::UIToName(lWeekday);

    // "Return" via copy ctor.
    Date lDate(lYear + mBaseYear + mCentury, lMonthName, lDay, lWeekdayName);
    aDate = lDate;
}


void DS3234::FillTimeStruct(Time const &aTime) {

    unsigned int lSeconds = BinaryToBCD(aTime.GetSeconds());
    unsigned int lMinutes = BinaryToBCD(aTime.GetMinutes());
    unsigned int lHours   = BinaryToBCD(aTime.GetHours());
    if (!aTime.Is24H()) {
        lHours |= HoursFields::H12_24_n;
        if (aTime.IsPM()) {
            lHours |= HoursFields::PM_AM_n;
        }
    }

    mRegMap.mTime.mSeconds = lSeconds;
    mRegMap.mTime.mMinutes = lMinutes;
    mRegMap.mTime.mHours   = lHours;
}


void DS3234::FillDateStruct(Date const &aDate) {

    unsigned int lWeekday = aDate.GetWeekday();
    unsigned int lDate    = BinaryToBCD(aDate.GetDate());
    unsigned int lMonth   = BinaryToBCD(aDate.GetMonth());
    unsigned int lYear    = BinaryToBCD(
        aDate.GetYear()
        - mBaseYear
        - mCentury
    );

    mRegMap.mDate.mWeekday = lWeekday;
    mRegMap.mDate.mDate    = lDate;
    mRegMap.mDate.mMonth   = lMonth;
    mRegMap.mDate.mYear    = lYear;
}


void DS3234::FillAlarmStruct(
    rtcc_alarm_t &aAlarmRef,
    Time const   &aTime,
    Date const   &aDate
) {

    FillAlarmTimeStruct(aAlarmRef, aTime);
    aAlarmRef.mDayDate = BinaryToBCD(aDate.GetDate());
}


void DS3234::FillAlarmStruct(
    rtcc_alarm_t  &aAlarmRef,
    Time    const &aTime,
    Weekday const &aWeekdayRef
) {

    FillAlarmTimeStruct(aAlarmRef, aTime);
    aAlarmRef.mDayDate = Weekday::NameToUI(aWeekdayRef.ToName());
}


void DS3234::FillAlarmTimeStruct(rtcc_alarm_t &aAlarmRef, Time const &aTime) {

    unsigned int lMinutes = BinaryToBCD(aTime.GetMinutes());
    unsigned int lHours   = BinaryToBCD(aTime.GetHours());
    if (!aTime.Is24H()) {
        lHours |= HoursFields::H12_24_n;
        if (aTime.IsPM()) {
            lHours |= HoursFields::PM_AM_n;
        }
    }

    aAlarmRef.mHours   = lHours;
    aAlarmRef.mMinutes = lMinutes;
}


void DS3234::FillAlarmModeStruct(rtcc_alarm_t &aAlarmRef, alarm_mode_t const aAlarmMode) {

    // Assumes the AxMy bit was cleared on previous operation.
    // This should be performed by BinaryToBCD().
    switch (aAlarmMode) {
    case ALARM_MODE::ONCE_PER_SEC:
        mRegMap.mAlarm1Seconds |= AnMx;
        // Intentional Fallthrough.

    case ALARM_MODE::WHEN_SECS_MATCH:
        aAlarmRef.mMinutes |= AnMx;
        // Intentional Fallthrough.

    case ALARM_MODE::WHEN_MINS_SECS_MATCH:
        aAlarmRef.mHours |= AnMx;
        // Intentional Fallthrough.

    case ALARM_MODE::WHEN_HOURS_MINS_SECS_MATCH:
        aAlarmRef.mDayDate |= AnMx;
        // Don't care about DY/DATEn bit.
    break;

    case ALARM_MODE::WHEN_DAY_HOURS_MINS_SECS_MATCH:
        aAlarmRef.mDayDate |= DAY_DATE_n;
    break;

    case ALARM_MODE::WHEN_DATE_HOURS_MINS_SECS_MATCH:
    default:
        // Nothing to set.
    break;
    }
}


void DS3234::TxAlarmStruct(alarm_id_t const aAlarmID) {

    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1:
        mSPIDev.WrData(
            L_WR_ADDR(mAlarm1Seconds),
            const_cast<uint8_t const *>(&mRegMap.mAlarm1Seconds),
            sizeof(rtcc_reg_t) + sizeof(rtcc_alarm_t),
            mSPICfg
        );

        // Control register needs to be written after alarm 1 registers.
        mSPIDev.WrData(
            L_WR_ADDR(mCtrl),
            const_cast<uint8_t const *>(&mRegMap.mCtrl),
            sizeof(rtcc_reg_t),
            mSPICfg
        );
    break;

    case ALARM_ID::ALARM_ID_2:
        // Control register follows alarm 2 registers.
        mSPIDev.WrData(
            L_WR_ADDR(mAlarm2),
            reinterpret_cast<uint8_t const *>(&mRegMap.mAlarm2),
            sizeof(rtcc_alarm_t) + sizeof(rtcc_reg_t),
            mSPICfg
        );
    break;
    }
}


void DS3234::SetAlarm(alarm_id_t const aAlarmID) {

    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1: mRegMap.mCtrl |= AEI1; break;
    case ALARM_ID::ALARM_ID_2: mRegMap.mCtrl |= AEI2; break;
    }
}


void DS3234::ClrAlarmFlag(alarm_id_t const aAlarmID) {

    // Only clear interrupt.
    // We don't care if the Alarm fields are set,
    // as long as the interrupt is not generated.
    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1: mRegMap.mStatus &= ~AF1; break;
    case ALARM_ID::ALARM_ID_2: mRegMap.mStatus &= ~AF2; break;
    }

    mSPIDev.WrData(
        L_WR_ADDR(mStatus),
        const_cast<uint8_t const *>(&mRegMap.mStatus),
        sizeof(rtcc_reg_t),
        mSPICfg
    );
}


bool DS3234::WrAlarm(
    alarm_id_t const aAlarmID,
    Time const &aTime,
    Date const &aDate,
    enum ALARM_MODE const aAlarmMode
) {

    // Fill alarm structure to write to RTC.
    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1:
        mRegMap.mAlarm1Seconds = BinaryToBCD(aTime.GetSeconds());
        FillAlarmStruct(mRegMap.mAlarm1, aTime, aDate);
        FillAlarmModeStruct(mRegMap.mAlarm1, aAlarmMode);
        break;

    case ALARM_ID::ALARM_ID_2:
        FillAlarmStruct(mRegMap.mAlarm2, aTime, aDate);
        FillAlarmModeStruct(mRegMap.mAlarm2, aAlarmMode);
        break;

    default:
        return false;
    }

    // Send alarm and control portion of the structure to the RTC.
    SetAlarm(aAlarmID);
    TxAlarmStruct(aAlarmID);
    return true;
}


uint8_t DS3234::GetCtrl(void) {

    // Return status field.
    UpdateCachedVal();
    return mRegMap.mCtrl;
}


uint8_t DS3234::GetStatus(void) {

    // Return status field.
    UpdateCachedVal();
    return mRegMap.mStatus;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
