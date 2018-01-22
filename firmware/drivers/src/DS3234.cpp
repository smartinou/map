// *****************************************************************************
//
// Project: Component drivers.
//
// Module: DS3234 RTCC.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// *****************************************************************************
//
//        Copyright (c) 2015-2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stddef.h>

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
DS3234::DS3234(unsigned int aBaseYear,
               SPIDev      &aSPIDevRef,
               SPISlaveCfg &aSPICfgRef)
  : RTCC(aBaseYear)
  , mSPIDevRef(aSPIDevRef)
  , mSPICfgRef(aSPICfgRef)
  , mRegMap{0} {

  // Ctor body intentionally left empty.
}


DS3234::~DS3234() {

  // Dtor body intentionally left empty.
}


void DS3234::Init(uint8_t aCtrlReg) {

  // Configure the RTC to desired state:
  //   -Oscillator enabled.
  //   -Battery-backed square wave disabled.
  //   -Rate select: 1KHz square wave.
  //   -Interrupt Control disabled (square wave enabled).
  //   -Alarm 'N' disabled.
  mRegMap.mCtrl  = 0x00;
  mRegMap.mCtrl |= aCtrlReg;

  // Disable all and clear all flags in status regiser.
  mRegMap.mStatus = 0x00;
  mSPIDevRef.WrData(L_WR_ADDR(mCtrl),
                    const_cast<uint8_t *>(&mRegMap.mCtrl),
                    2 * sizeof(rtcc_reg_t),
                    mSPICfgRef);
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
  if (mRegMap.mTime.mHours & Hours::H12_24_n) {
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

  unsigned int lMonth = BCDToBinary(mRegMap.mDate.mMonth & Month::CENTURY);
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


void DS3234::UpdateCachedVal(void) {

  if (IsImpure()) {
    // Read the whole RTC.
    mSPIDevRef.RdData(0,
                      reinterpret_cast<uint8_t *>(&mRegMap),
                      sizeof(rtcc_reg_map_t),
                      mSPICfgRef);
    mIsImpure = false;
  }
}


void DS3234::RdTime(Time &aTimeRef) {

  // Read the Time portion of the RTC.
  // Update time ref.
  mSPIDevRef.RdData(L_RD_ADDR(mTime),
                    reinterpret_cast<uint8_t *>(&mRegMap.mTime),
                    sizeof(time2_t),
                    mSPICfgRef);
  UpdateTime(aTimeRef);
}


void DS3234::RdDate(Date &aDateRef) {

  // Read the date portion out of RTC.
  // Update date ref.
  mSPIDevRef.RdData(L_RD_ADDR(mDate),
                    reinterpret_cast<uint8_t *>(&mRegMap.mDate),
                    sizeof(date_t),
                    mSPICfgRef);
  UpdateDate(aDateRef);
}


void DS3234::RdTimeAndDate(Time &aTimeRef, Date &aDateRef) {

  // Read the whole RTC into register map structure.
  // Update time ref.
  // Update date ref.
  mSPIDevRef.RdData(L_RD_ADDR(mTime),
                    reinterpret_cast<uint8_t *>(&mRegMap.mTime),
                    sizeof(time2_t) + sizeof(date_t),
                    mSPICfgRef);

  UpdateTime(aTimeRef);
  UpdateDate(aDateRef);
}


void DS3234::WrTime(Time const &aTimeRef) {

  // Fill time structure to write to RTC.
  // Send only the time portion of the structure to RTC.
  FillTimeStruct(aTimeRef);
  mSPIDevRef.WrData(L_WR_ADDR(mTime),
                    reinterpret_cast<uint8_t *>(&mRegMap.mTime),
                    sizeof(time2_t),
                    mSPICfgRef);
}


void DS3234::WrDate(Date const &aDateRef) {

  // Fill date structure to write to RTC.
  // Send only the date portion of the structure to RTC.
  FillDateStruct(aDateRef);
  mSPIDevRef.WrData(L_WR_ADDR(mDate),
                    reinterpret_cast<uint8_t *>(&mRegMap.mDate),
                    sizeof(date_t),
                    mSPICfgRef);
}


void DS3234::WrTimeAndDate(Time const &aTimeRef, Date const &aDateRef) {

  // Fill time structure to write to RTC.
  // Fill date structure to write to RTC.
  // Send time and date portion of the structure to RTC.
  FillTimeStruct(aTimeRef);
  FillDateStruct(aDateRef);
  mSPIDevRef.WrData(L_WR_ADDR(mTime),
                    reinterpret_cast<uint8_t *>(&mRegMap.mTime),
                    sizeof(time2_t) + sizeof(date_t),
                    mSPICfgRef);
}


void DS3234::WrAlarm(enum ALARM_ID   aAlarmID,
                     Time    const  &aTimeRef,
                     Weekday const  &aWeekdayRef,
                     enum ALARM_MODE aAlarmMode) {

  // Fill alarm structure to write to RTC.
  switch (aAlarmID) {
  case ALARM_ID::ALARM_ID_1:
    mRegMap.mAlarm1Seconds = BinaryToBCD(aTimeRef.GetSeconds());
    FillAlarmStruct(mRegMap.mAlarm1, aTimeRef, aWeekdayRef);
    FillAlarmModeStruct(mRegMap.mAlarm1, aAlarmMode);
    mRegMap.mCtrl |= AEI1;
    break;

  case ALARM_ID::ALARM_ID_2:
    FillAlarmStruct(mRegMap.mAlarm2, aTimeRef, aWeekdayRef);
    FillAlarmModeStruct(mRegMap.mAlarm2, aAlarmMode);
    mRegMap.mCtrl |= AEI2;
    break;
  }

  // Send alarm and control portion of the structure to the RTC.
  TxAlarmStruct(aAlarmID);
}


void DS3234::WrAlarm(enum ALARM_ID   aAlarmID,
                     Time const     &aTimeRef,
                     Date const     &aDateRef,
                     enum ALARM_MODE aAlarmMode) {

  // Fill alarm structure to write to RTC.
  switch (aAlarmID) {
  case ALARM_ID::ALARM_ID_1:
    mRegMap.mAlarm1Seconds = BinaryToBCD(aTimeRef.GetSeconds());
    FillAlarmStruct(mRegMap.mAlarm1, aTimeRef, aDateRef);
    FillAlarmModeStruct(mRegMap.mAlarm1, aAlarmMode);
    break;

  case ALARM_ID::ALARM_ID_2:
    FillAlarmStruct(mRegMap.mAlarm2, aTimeRef, aDateRef);
    FillAlarmModeStruct(mRegMap.mAlarm2, aAlarmMode);
    break;
  }

  // Send alarm and control portion of the structure to the RTC.
  SetAlarm(aAlarmID);
  TxAlarmStruct(aAlarmID);
}


void DS3234::SetAlarm(enum ALARM_ID aAlarmID) {

  switch (aAlarmID) {
  case ALARM_ID::ALARM_ID_1: mRegMap.mCtrl |= AEI1; break;
  case ALARM_ID::ALARM_ID_2: mRegMap.mCtrl |= AEI2; break;
  }
}


void DS3234::DisableAlarm(enum ALARM_ID aAlarmID) {

  // Only clear interrupt.
  // We don't care if the Alarm fields are set,
  // as long as the interrupt is not generated.
  switch (aAlarmID) {
  case ALARM_ID::ALARM_ID_1: mRegMap.mCtrl &= ~AEI1; break;
  case ALARM_ID::ALARM_ID_2: mRegMap.mCtrl &= ~AEI2; break;
  }

  mSPIDevRef.WrData(L_WR_ADDR(mCtrl),
                    const_cast<uint8_t const *>(&mRegMap.mCtrl),
                    sizeof(rtcc_reg_t),
                    mSPICfgRef);
}


void DS3234::ClrAlarmFlag(enum ALARM_ID aAlarmID) {

  // Only clear interrupt.
  // We don't care if the Alarm fields are set,
  // as long as the interrupt is not generated.
  switch (aAlarmID) {
  case ALARM_ID::ALARM_ID_1: mRegMap.mStatus &= ~AF1; break;
  case ALARM_ID::ALARM_ID_2: mRegMap.mStatus &= ~AF2; break;
  }

  mSPIDevRef.WrData(L_WR_ADDR(mStatus),
                    const_cast<uint8_t const *>(&mRegMap.mStatus),
                    sizeof(rtcc_reg_t),
                    mSPICfgRef);
}


bool DS3234::HasNVMem(void) const {
  return true;
}


void DS3234::RdFromRAM(uint8_t * const aDataPtr,
                       unsigned int    aOffset,
                       unsigned int    aSize) {

  // Write SRAM address register.
  // Cap size.
  // Loop into data register.
  mSPIDevRef.WrData(L_WR_ADDR(mSRAMAddr),
                    reinterpret_cast<uint8_t *>(aOffset),
                    sizeof(rtcc_reg_t),
                    mSPICfgRef);

  unsigned int lMaxSize = 256 - aOffset;
  if (aSize > lMaxSize) {
    aSize = lMaxSize;
  }

  mSPIDevRef.RdData(L_RD_ADDR(mSRAMData),
                    reinterpret_cast<uint8_t *>(aDataPtr),
                    aSize * sizeof(rtcc_reg_t),
                    mSPICfgRef);
}


void DS3234::WrToRAM(uint8_t const * const aDataPtr,
                     unsigned int          aOffset,
                     unsigned int          aSize) {

  // Write SRAM address register.
  // Cap size.
  // Loop into data register.
  mSPIDevRef.WrData(L_WR_ADDR(mSRAMAddr),
                    reinterpret_cast<uint8_t *>(aOffset),
                    sizeof(rtcc_reg_t),
                    mSPICfgRef);

  unsigned int lMaxSize = 256 - aOffset;
  if (aSize > lMaxSize) {
    aSize = lMaxSize;
  }

  mSPIDevRef.WrData(L_WR_ADDR(mSRAMData),
                    reinterpret_cast<uint8_t const *>(aDataPtr),
                    aSize * sizeof(rtcc_reg_t),
                    mSPICfgRef);
}


void DS3234::GetTimeAndDate(Time &aTimeRef, Date &aDateRef) {

  // Update date ref.
  UpdateCachedVal();
  UpdateTime(aTimeRef);
  UpdateDate(aDateRef);
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


float DS3234::GetTemperature(void) {

  // Return temperature field.
  // Convert temperature MSB and LSB to float.
  UpdateCachedVal();
  float lTempFloat = 0.25 * (mRegMap.mTempLSB >> 6);
  lTempFloat += static_cast<float>(mRegMap.mTempMSB);
  return lTempFloat;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void DS3234::UpdateTime(Time &aTimeRef) {

  // Apply proper mask and convert where needed.
  unsigned int lSeconds = BCDToBinary(mRegMap.mTime.mSeconds);
  unsigned int lMinutes = BCDToBinary(mRegMap.mTime.mMinutes);
  bool lIs24H = true;
  bool lIsPM  = false;
  unsigned int lHours = 0;
  if (mRegMap.mTime.mHours & Hours::H12_24_n) {
    lIs24H = false;
    if (mRegMap.mTime.mHours & Hours::PM_AM_n) {
      lIsPM = true;
    }
    lHours = BCDToBinary(mRegMap.mTime.mHours & ~(Hours::H12_24_n | Hours::PM_AM_n));
  } else {
    lHours = BCDToBinary(mRegMap.mTime.mHours & ~Hours::H12_24_n);
  }

  aTimeRef.SetSeconds(lSeconds);
  aTimeRef.SetMinutes(lMinutes);
  aTimeRef.SetHours(lHours);
  aTimeRef.SetIs24H(lIs24H);
  aTimeRef.SetIsPM(lIsPM);
}


void DS3234::UpdateDate(Date &aDateRef) {

  unsigned int lWeekday = BCDToBinary(mRegMap.mDate.mWeekday);
  unsigned int lDate    = BCDToBinary(mRegMap.mDate.mDate);
  unsigned int lMonth   = BCDToBinary(mRegMap.mDate.mMonth & ~Month::CENTURY);
  unsigned int lYear    = BCDToBinary(mRegMap.mDate.mYear);
  if (mRegMap.mDate.mMonth & Month::CENTURY) {
    mCentury += 100;
  }

  aDateRef.SetWeekday(lWeekday);
  aDateRef.SetDate(lDate);
  aDateRef.SetMonth(lMonth);
  aDateRef.SetYear(lYear + mBaseYear + mCentury);
}


void DS3234::FillTimeStruct(Time const &aTimeRef) {

  unsigned int lSeconds = BinaryToBCD(aTimeRef.GetSeconds());
  unsigned int lMinutes = BinaryToBCD(aTimeRef.GetMinutes());
  unsigned int lHours   = BinaryToBCD(aTimeRef.GetHours());
  if (!aTimeRef.Is24H()) {
    lHours |= Hours::H12_24_n;
    if (aTimeRef.IsPM()) {
      lHours |= Hours::PM_AM_n;
    }
  }

  mRegMap.mTime.mSeconds = lSeconds;
  mRegMap.mTime.mMinutes = lMinutes;
  mRegMap.mTime.mHours   = lHours;
}


void DS3234::FillDateStruct(Date const &aDateRef) {

  unsigned int lWeekday = aDateRef.GetWeekday();
  unsigned int lDate    = BinaryToBCD(aDateRef.GetDate());
  unsigned int lMonth   = BinaryToBCD(aDateRef.GetMonth());
  unsigned int lYear    = BinaryToBCD(aDateRef.GetYear()
                                      - mBaseYear
                                      - mCentury);

  mRegMap.mDate.mWeekday = lWeekday;
  mRegMap.mDate.mDate    = lDate;
  mRegMap.mDate.mMonth   = lMonth;
  mRegMap.mDate.mYear    = lYear;
}


void DS3234::FillAlarmStruct(rtcc_alarm_t &aAlarmRef,
                             Time const   &aTimeRef,
                             Date const   &aDateRef) {

  FillAlarmTimeStruct(aAlarmRef, aTimeRef);
  aAlarmRef.mDayDate = BinaryToBCD(aDateRef.GetDate());
}


void DS3234::FillAlarmStruct(rtcc_alarm_t   &aAlarmRef,
                             Time    const  &aTimeRef,
                             Weekday const  &aWeekdayRef) {

  FillAlarmTimeStruct(aAlarmRef, aTimeRef);
  aAlarmRef.mDayDate = Weekday::NameToUI(aWeekdayRef.GetName());
}


void DS3234::FillAlarmTimeStruct(rtcc_alarm_t &aAlarmRef,
                                 Time const   &aTimeRef) {

  unsigned int lMinutes = BinaryToBCD(aTimeRef.GetMinutes());
  unsigned int lHours   = BinaryToBCD(aTimeRef.GetHours());
  if (!aTimeRef.Is24H()) {
    lHours |= Hours::H12_24_n;
    if (aTimeRef.IsPM()) {
      lHours |= Hours::PM_AM_n;
    }
  }

  aAlarmRef.mHours   = lHours;
  aAlarmRef.mMinutes = lMinutes;
}


void DS3234::FillAlarmModeStruct(rtcc_alarm_t    &aAlarmRef,
                                 enum ALARM_MODE  aAlarmMode) {

  // Assumes the AxMy bit was clear on previous operation.
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


void DS3234::TxAlarmStruct(enum ALARM_ID aAlarmID) {

  switch (aAlarmID) {
  case ALARM_ID::ALARM_ID_1:
    mSPIDevRef.WrData(L_WR_ADDR(mAlarm1Seconds),
                      const_cast<uint8_t const *>(&mRegMap.mAlarm1Seconds),
                      sizeof(rtcc_reg_t) + sizeof(rtcc_alarm_t),
                      mSPICfgRef);

    // Control register needs to be written after alarm 1 registers.
    mSPIDevRef.WrData(L_WR_ADDR(mCtrl),
                      const_cast<uint8_t const *>(&mRegMap.mCtrl),
                      sizeof(rtcc_reg_t),
                      mSPICfgRef);
    break;

  case ALARM_ID::ALARM_ID_2:
    // Control register follows alarm 2 registers.
    mSPIDevRef.WrData(L_WR_ADDR(mAlarm2),
                      reinterpret_cast<uint8_t const *>(&mRegMap.mAlarm2),
                      sizeof(rtcc_alarm_t) + sizeof(rtcc_reg_t),
                      mSPICfgRef);
    break;
  }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
