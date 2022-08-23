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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <cstdint>
#include <cstdio>

// TI Library.
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

#include "SPIMasterDev.h"

#include "DS3234Chrono.h"

using namespace std::chrono_literals;

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

static constexpr auto sSPIClk{4000000};
static constexpr auto sSPIBits{8};

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

namespace WithChrono {


// Ctor.
DS3234::DS3234(
    int const aBaseYear,
    unsigned long const aInterruptNumber,
    PortPin const &aInterruptPin,
    std::shared_ptr<CoreLink::ISPIMasterDev> const aSPIMasterDev,
    PortPin const &aCSnPin
) noexcept
    : mBaseYear{aBaseYear}
    , mSPIMasterDev{std::move(aSPIMasterDev)}
    , mSPISlaveCfg{
        GPIO{aCSnPin.mPort, aCSnPin.mPin},
        CoreLink::SPISlaveCfg::PROTOCOL::MOTO_1,
        sSPIClk,
        sSPIBits
    }
    , mInterruptNumber{aInterruptNumber}
    , mInterruptGPIO{aInterruptPin}
{
    mSPISlaveCfg.InitCSnGPIO();
}


void DS3234::Init() {

    // Configure the RTC to desired state:
    //   -Oscillator enabled.
    //   -Battery-backed square wave disabled.
    //   -Interrupt enabled (square wave disabled).
    //   -Alarm 'N' disabled.
    mRegMap.mCtrl = std::byte{Ctrl::INTCn};

    // Disable all and clear all flags in status register.
    mRegMap.mStatus = std::byte{0x00};
    mSPIMasterDev->WrData(
        ToWrAddr(sCtrlAddr),
        reinterpret_cast<uint8_t *>(&mRegMap.mCtrl),
        2 * sizeof(rtcc_reg_t),
        mSPISlaveCfg
    );

    // Force set time to 24H mode if set to 12H.
    time_t lTime;
    mSPIMasterDev->RdData(
        sTimeAddr,
        reinterpret_cast<uint8_t *>(&lTime),
        sizeof(time_t),
        mSPISlaveCfg
    );

    if (std::to_integer<bool>(lTime.mHours & std::byte{DayDateFields::DAY_DATE_n})) {
        lTime.mHours &= ~std::byte{DayDateFields::DAY_DATE_n};
        mSPIMasterDev->WrData(
            ToWrAddr(sTimeAddr),
            reinterpret_cast<uint8_t *>(&lTime),
            sizeof(time_t),
            mSPISlaveCfg
        );
    }
}


void DS3234::SetInterrupt(bool const aEnable) {

    MAP_IntDisable(mInterruptNumber);
    if (aEnable) {
        MAP_GPIOPinTypeGPIOInput(mInterruptGPIO.mPort, mInterruptGPIO.mPin);
        MAP_GPIOIntTypeSet(
            mInterruptGPIO.mPort,
            mInterruptGPIO.mPin,
            GPIO_FALLING_EDGE
        );
        MAP_GPIOPadConfigSet(
            mInterruptGPIO.mPort,
            mInterruptGPIO.mPin,
            GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD
        );
#ifdef USE_TIVAWARE
        MAP_GPIOIntEnable(mInterruptGPIO.mPort, mInterruptGPIO.mPin);
#elif defined (USE_STELLARISWARE)
        MAP_GPIOPinIntEnable(mInterruptGPIO.mPort, mInterruptGPIO.mPin);
#endif
        // [MG] THIS CLEARS THE 1ST INTERRUPT.
        // IT DOESN'T COME UP UNTIL FLAGS ARE CLEARED.
        //MAP_GPIOPinIntClear(mInterrupt->GetPort(), lGPIOInitEvtPtr->mIRQGPIOPin);

        MAP_IntEnable(mInterruptNumber);

        // Alarm1 is dedicated to periodic interrupt.
        static constexpr Time lTime{};
        static constexpr Date lDate{};
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


void DS3234::AckInterrupt() {
    // It is assumed that all interrupts where serviced at this point.
    // Clear all flags.
    ClrAlarmFlag(ALARM_ID::ALARM_ID_1);
    ClrAlarmFlag(ALARM_ID::ALARM_ID_2);
}


void DS3234::UpdateCachedVal() {

    if (IsImpure()) {
        // Read the whole RTC.
        mSPIMasterDev->RdData(
            0,
            reinterpret_cast<uint8_t *>(&mRegMap),
            sizeof(rtcc_reg_map_t),
            mSPISlaveCfg
        );
        mIsImpure = false;
    }
}


auto DS3234::RdTime() -> DS3234::Time {

    // Read the Time portion of the RTC.
    // Update time ref.
    mSPIMasterDev->RdData(
        sTimeAddr,
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time_t),
        mSPISlaveCfg
    );
    return UpdateTime();
}


auto DS3234::RdDate() -> DS3234::Date {

    // Read the date portion out of RTC.
    // Update date ref.
    mSPIMasterDev->RdData(
        sDateAddr,
        reinterpret_cast<uint8_t *>(&mRegMap.mDate),
        sizeof(date_t),
        mSPISlaveCfg
    );
    return UpdateDate();
}


auto DS3234::RdTimeAndDate() -> DS3234::TimeAndDate {

    // Read the whole RTC into register map structure.
    // Update time ref.
    // Update date ref.
    mSPIMasterDev->RdData(
        sTimeAddr,
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time_t) + sizeof(date_t),
        mSPISlaveCfg
    );

    auto const lTime = UpdateTime();
    auto const lDate = UpdateDate();
    return {lTime, lDate};
}


void DS3234::WrTime(DS3234::Time const &aTime) {

    // Fill time structure to write to RTC.
    // Send only the time portion of the structure to RTC.
    FillTimeStruct(aTime);
    mSPIMasterDev->WrData(
        ToWrAddr(sTimeAddr),
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time_t),
        mSPISlaveCfg
    );
}


void DS3234::WrDate(DS3234::Date const &aDate) {

    // Fill date structure to write to RTC.
    // Send only the date portion of the structure to RTC.
    FillDateStruct(aDate);
    mSPIMasterDev->WrData(
        ToWrAddr(sDateAddr),
        reinterpret_cast<uint8_t *>(&mRegMap.mDate),
        sizeof(date_t),
        mSPISlaveCfg
    );
}


void DS3234::WrTimeAndDate(DS3234::Time const &aTime, DS3234::Date const &aDate) {

    // Fill time structure to write to RTC.
    // Fill date structure to write to RTC.
    // Send time and date portion of the structure to RTC.
    FillTimeStruct(aTime);
    FillDateStruct(aDate);
    mSPIMasterDev->WrData(
        ToWrAddr(sTimeAddr),
        reinterpret_cast<uint8_t *>(&mRegMap.mTime),
        sizeof(time_t) + sizeof(date_t),
        mSPISlaveCfg
    );
}


auto DS3234::GetCachedTimeAndDate() -> DS3234::TimeAndDate {

    // Update date ref.
    UpdateCachedVal();
    auto const lTime = UpdateTime();
    auto const lDate = UpdateDate();
    return {lTime, lDate};
}


auto DS3234::GetTemperature() -> float {

    // Return temperature field.
    // Convert temperature MSB and LSB to float.
    UpdateCachedVal();
    float const lTempFloat = (0.25F * static_cast<float>(std::to_integer<uint8_t>(mRegMap.mTempLSB >> 6)))
        + static_cast<float>(mRegMap.mTempMSB);
    return lTempFloat;
}


void DS3234::WrAlarm(DS3234::Time const &aTime, DS3234::Date const &aDate) {

    // Public alarm is triggered on time match.
    // Alarm1 is dedicated to periodic time/date interrupt.
    return WrAlarm(
        ALARM_ID::ALARM_ID_2,
        aTime,
        aDate,
        ALARM_MODE::WHEN_DATE_HOURS_MINS_SECS_MATCH
    );
}


void DS3234::WrAlarm(DS3234::Time const &aTime, DS3234::Weekday const &aWeekday) {

    // Hard-coded to use alarm2, but provides template for use with any alarm.
    alarm_mode_t constexpr sAlarmMode{ALARM_MODE::WHEN_DAY_HOURS_MINS_SECS_MATCH};
    // Fill alarm structure to write to RTC.
    mRegMap.mAlarm2 = FillAlarmStruct(aTime, aWeekday);
    mRegMap.mAlarm2 = FillAlarmModeStruct(mRegMap.mAlarm2, sAlarmMode);

    // Send alarm and control portion of the structure to the RTC.
    alarm_id_t constexpr sAlarmID{ALARM_ID::ALARM_ID_2};
    SetAlarm(sAlarmID);
    TxAlarmStruct(sAlarmID);
}


auto DS3234::IsAlarmOn() -> bool {

    return (std::to_integer<bool>(std::byte{Status::AF2} & GetStatus())
        && std::to_integer<bool>(std::byte{Ctrl::AEI2} & GetCtrl()));
}


void DS3234::DisableAlarm() {

    mRegMap.mCtrl &= ~std::byte{Ctrl::AEI2};
    mSPIMasterDev->WrData(
        ToWrAddr(sCtrlAddr),
        reinterpret_cast<uint8_t const *>(&mRegMap.mCtrl),
        sizeof(rtcc_reg_t),
        mSPISlaveCfg
    );
}


void DS3234::ClrAlarmFlag() {

    ClrAlarmFlag(ALARM_ID::ALARM_ID_2);
}


void DS3234::RdFromNVMem(
    uint8_t * const aDataPtr,
    std::size_t const aOffset,
    std::size_t const aSize
) {

    // Write SRAM address register.
    // Cap size.
    // Loop into data register.
    mSPIMasterDev->WrData(
        ToWrAddr(sSRAMAddrAddr),
        reinterpret_cast<uint8_t *>(aOffset),
        sizeof(rtcc_reg_t),
        mSPISlaveCfg
    );

    std::size_t const lMaxSize = mNVMemSize - aOffset;
    std::size_t lSize = aSize;
    if (lSize > lMaxSize) {
        lSize = lMaxSize;
    }

    mSPIMasterDev->RdData(
        sSRAMDataAddr,
        reinterpret_cast<uint8_t *>(aDataPtr),
        lSize * sizeof(rtcc_reg_t),
        mSPISlaveCfg
    );
}


void DS3234::WrToNVMem(
    uint8_t const * const aDataPtr,
    std::size_t const aOffset,
    std::size_t const aSize
) {

    // Write SRAM address register.
    // Cap size.
    // Loop into data register.
    mSPIMasterDev->WrData(
        ToWrAddr(sSRAMAddrAddr),
        reinterpret_cast<uint8_t *>(aOffset),
        sizeof(rtcc_reg_t),
        mSPISlaveCfg
    );

    std::size_t const lMaxSize = mNVMemSize - aOffset;
    std::size_t lSize = aSize;
    if (lSize > lMaxSize) {
        lSize = lMaxSize;
    }

    mSPIMasterDev->WrData(
        ToWrAddr(sSRAMDataAddr),
        reinterpret_cast<uint8_t const *>(aDataPtr),
        lSize * sizeof(rtcc_reg_t),
        mSPISlaveCfg
    );
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

auto DS3234::UpdateTime() const noexcept -> DS3234::Time {

    // Apply proper mask and convert where needed.
    auto const lSeconds = DS3234Helper::BCDToBinary(mRegMap.mTime.mSeconds);
    auto const lMinutes = DS3234Helper::BCDToBinary(mRegMap.mTime.mMinutes);

    auto lHours{0};
    if (std::to_integer<bool>(mRegMap.mTime.mHours & std::byte{HoursFields::H12_24_n})) {
        // Should not take this branch: time is stored in 24H mode.
        auto const lHoursReg = mRegMap.mTime.mHours
            & ~(std::byte{HoursFields::H12_24_n} | std::byte{HoursFields::PM_AM_n});
        lHours = DS3234Helper::BCDToBinary(lHoursReg);
        if (std::to_integer<bool>(mRegMap.mTime.mHours & std::byte{HoursFields::PM_AM_n})) {
            lHours += 12;
        }
    } else {
        lHours = DS3234Helper::BCDToBinary(mRegMap.mTime.mHours);
    }

    // "Return" via copy ctor.
    return Time{
        std::chrono::hours(lHours) 
        + std::chrono::minutes(lMinutes)
        + std::chrono::seconds(lSeconds)
    };
}


auto DS3234::UpdateDate() const noexcept -> DS3234::Date {

    //auto const lWeekday = BCDToBinary(mRegMap.mDate.mWeekday);
    std::chrono::day const lDay {DS3234Helper::BCDToBinary(mRegMap.mDate.mDate)};
    std::chrono::month const lMonth {DS3234Helper::BCDToBinary(mRegMap.mDate.mMonth & ~std::byte{MonthFields::CENTURY})};
    std::chrono::year const lYear {DS3234Helper::BCDToBinary(mRegMap.mDate.mYear) + mBaseYear};

    // Should check for a valid date here. If not, construct one.
    return {lYear, lMonth, lDay};
}


void DS3234::FillTimeStruct(DS3234::Time const &aTime) noexcept {

    mRegMap.mTime.mSeconds = DS3234Helper::BinaryToBCD(aTime.seconds().count());
    mRegMap.mTime.mMinutes = DS3234Helper::BinaryToBCD(aTime.minutes().count());
    // Time is always stored as 24H.
    mRegMap.mTime.mHours = DS3234Helper::BinaryToBCD(aTime.hours().count());
}


void DS3234::FillDateStruct(DS3234::Date const &aDate) noexcept {

    std::chrono::weekday const lWeekday{std::chrono::sys_days{aDate}};
    mRegMap.mDate.mWeekday = std::byte{static_cast<unsigned char>(lWeekday.iso_encoding())};

    mRegMap.mDate.mDate = DS3234Helper::BinaryToBCD(static_cast<unsigned>(aDate.day()));
    mRegMap.mDate.mMonth = DS3234Helper::BinaryToBCD(static_cast<unsigned>(aDate.month()));

    auto const lYears = std::chrono::years(int(aDate.year()) - int(std::chrono::year{mBaseYear}));
    mRegMap.mDate.mYear = DS3234Helper::BinaryToBCD(lYears.count());
}


auto DS3234::FillAlarmStruct(
    DS3234::Time const &aTime,
    DS3234::Date const &aDate
) noexcept -> DS3234::rtcc_alarm_t
{
    auto const lHours = DS3234Helper::BinaryToBCD(aTime.hours().count());
    auto const lMinutes = DS3234Helper::BinaryToBCD(aTime.minutes().count());
    auto const lDay = DS3234Helper::BinaryToBCD(static_cast<unsigned>(aDate.day()));

    return {
        lHours,
        lMinutes,
        lDay | std::byte{DayDateFields::DAY_DATE_n}
    };
}


auto DS3234::FillAlarmStruct(
    DS3234::Time const &aTime,
    DS3234::Weekday const &aWeekday
) noexcept -> DS3234::rtcc_alarm_t
{
    auto const lHours = DS3234Helper::BinaryToBCD(aTime.hours().count());
    auto const lMinutes = DS3234Helper::BinaryToBCD(aTime.minutes().count());
    auto const lWeekday = static_cast<unsigned char>(aWeekday.iso_encoding());

    return {lHours, lMinutes, std::byte{lWeekday}};
}


auto DS3234::FillAlarmModeStruct(
    rtcc_alarm_t const &aAlarmStruct,
    alarm_mode_t const aAlarmMode
) -> DS3234::rtcc_alarm_t
{
    rtcc_alarm_t lAlarmStruct{aAlarmStruct};

    // Assumes the AxMy bit was cleared on previous operation.
    // This should be performed by BinaryToBCD().
    switch (aAlarmMode) {
    case ALARM_MODE::ONCE_PER_SEC:
        //mRegMap.mAlarm1Seconds |= std::byte{DayDateFields::AnMx};
        [[fallthrough]];

    case ALARM_MODE::WHEN_SECS_MATCH:
        lAlarmStruct.mMinutes |= std::byte{DayDateFields::AnMx};
        [[fallthrough]];

    case ALARM_MODE::WHEN_MINS_SECS_MATCH:
        lAlarmStruct.mHours |= std::byte{DayDateFields::AnMx};
        [[fallthrough]];

    case ALARM_MODE::WHEN_HOURS_MINS_SECS_MATCH:
        lAlarmStruct.mDayDate_n |= std::byte{DayDateFields::AnMx};
        // Don't care about DY/DATEn bit.
        break;

    case ALARM_MODE::WHEN_DAY_HOURS_MINS_SECS_MATCH:
        lAlarmStruct.mDayDate_n |= std::byte{DayDateFields::DAY_DATE_n};
        break;

    case ALARM_MODE::WHEN_DATE_HOURS_MINS_SECS_MATCH:
    [[fallthrough]];
    default:
        // Nothing to set.
        break;
    }

    return lAlarmStruct;
}


void DS3234::TxAlarmStruct(alarm_id_t const aAlarmID) {

    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1:
        mSPIMasterDev->WrData(
            ToWrAddr(sAlarm1Addr),
            reinterpret_cast<uint8_t const *>(&mRegMap.mAlarm1Seconds),
            sizeof(rtcc_reg_t) + sizeof(rtcc_alarm_t),
            mSPISlaveCfg
        );

        // Control register needs to be written after alarm 1 registers.
        mSPIMasterDev->WrData(
            ToWrAddr(sCtrlAddr),
            reinterpret_cast<uint8_t const *>(&mRegMap.mCtrl),
            sizeof(rtcc_reg_t),
            mSPISlaveCfg
        );
        break;

    case ALARM_ID::ALARM_ID_2:
        // Control register follows alarm 2 registers.
        mSPIMasterDev->WrData(
            ToWrAddr(sAlarm2Addr),
            reinterpret_cast<uint8_t const *>(&mRegMap.mAlarm2),
            sizeof(rtcc_alarm_t) + sizeof(rtcc_reg_t),
            mSPISlaveCfg
        );
        break;
    }
}


void DS3234::SetAlarm(alarm_id_t const aAlarmID) {

    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1: mRegMap.mCtrl |= std::byte{Ctrl::AEI1}; break;
    case ALARM_ID::ALARM_ID_2: mRegMap.mCtrl |= std::byte{Ctrl::AEI2}; break;
    }
}


void DS3234::ClrAlarmFlag(alarm_id_t const aAlarmID) {

    // Only clear interrupt.
    // We don't care if the Alarm fields are set,
    // as long as the interrupt is not generated.
    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1: mRegMap.mStatus &= ~std::byte{Status::AF1}; break;
    case ALARM_ID::ALARM_ID_2: mRegMap.mStatus &= ~std::byte{Status::AF2}; break;
    }

    mSPIMasterDev->WrData(
        ToWrAddr(sStatusAddr),
        reinterpret_cast<uint8_t const *>(&mRegMap.mStatus),
        sizeof(rtcc_reg_t),
        mSPISlaveCfg
    );
}


void DS3234::WrAlarm(
    alarm_id_t const aAlarmID,
    DS3234::Time const &aTime,
    DS3234::Date const &aDate,
    enum ALARM_MODE const aAlarmMode
) {

    // Fill alarm structure to write to RTC.
    switch (aAlarmID) {
    case ALARM_ID::ALARM_ID_1: {
        mRegMap.mAlarm1Seconds = DS3234Helper::BinaryToBCD(aTime.seconds().count())
            | ((aAlarmMode == ALARM_MODE::ONCE_PER_SEC) ? std::byte{DayDateFields::AnMx} : std::byte{});
        mRegMap.mAlarm1 = FillAlarmStruct(aTime, aDate);
        mRegMap.mAlarm1 = FillAlarmModeStruct(mRegMap.mAlarm1, aAlarmMode);
        break;
    }

    case ALARM_ID::ALARM_ID_2:
        mRegMap.mAlarm2 = FillAlarmStruct(aTime, aDate);
        mRegMap.mAlarm2 = FillAlarmModeStruct(mRegMap.mAlarm2, aAlarmMode);
        break;
    }

    // Send alarm and control portion of the structure to the RTC.
    SetAlarm(aAlarmID);
    TxAlarmStruct(aAlarmID);
}


auto DS3234::GetCtrl() -> DS3234::rtcc_reg_t {

    // Return status field.
    UpdateCachedVal();
    return mRegMap.mCtrl;
}


auto DS3234::GetStatus() -> DS3234::rtcc_reg_t {

    // Return status field.
    UpdateCachedVal();
    return mRegMap.mStatus;
}

#if 0
namespace DS3234Helper {

std::byte BinaryToBCD(uint8_t const aByte) {

    auto const lLowNibble = aByte % 10;
    auto const lHighNibble = aByte / 10;

    return std::byte(lHighNibble << 4 | lLowNibble);
}


uint8_t BCDToBinary(std::byte const &aBCD) {

    auto const lBCD = std::to_integer<uint8_t>(aBCD);
    auto const lUnits = lBCD & 0x0F;
    auto const lDecimals = (lBCD & 0xF0) * 10;

    return lDecimals + lUnits;
}

} // namespace DS3234Helper
#endif

} // namespace WithChrono

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
