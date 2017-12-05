// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Feeding calendar class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2016-2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <string.h>

// Common Library.
#include "DBRec.h"
#include "Date.h"
#include "Time.h"

// This project.
#include "CalendarRec.h"

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

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

CalendarRec::CalendarRec() :
  DBRec() {

  // Ctor body left intentionally empty.
}


void CalendarRec::ClrAllEntries(void) {

  mRec.mCalendarArray.fill(0);
}


void CalendarRec::SetEntry(Weekday const &aWeekdayRef, Time const &aTimeRef) {

  unsigned int lWeekdayUI = aWeekdayRef.Get();
  SetEntry(lWeekdayUI, aTimeRef);
}


void CalendarRec::SetEntry(unsigned int const aWeekday, Time const &aTimeRef) {

  unsigned int lArrayIx        = GetArrayIx(aTimeRef);
  unsigned int lWeekdayBitMask = (1 << aWeekday);

  mRec.mCalendarArray.at(lArrayIx) |= lWeekdayBitMask;

  // Check if all week is set. If so, set the "AllWeek" bit.
  if (~ALL_WEEK_BIT_MASK == (~ALL_WEEK_BIT_MASK & mRec.mCalendarArray.at(lArrayIx))) {
    mRec.mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
  }

  mIsDirty = true;
}


void CalendarRec::SetTimeEntry(Time const &aTimeRef) {

  unsigned int lArrayIx = GetArrayIx(aTimeRef);
  mRec.mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
  mIsDirty = true;
}


void CalendarRec::ClrEntry(Weekday const &aWeekdayRef, Time const &aTimeRef) {

  unsigned int lWeekdayUI = aWeekdayRef.Get();
  ClrEntry(lWeekdayUI, aTimeRef);
}


void CalendarRec::ClrEntry(unsigned int aWeekday, Time const &aTimeRef) {

  unsigned int lArrayIx        = GetArrayIx(aTimeRef);
  unsigned int lWeekdayBitMask = (0x1 << aWeekday);

  mRec.mCalendarArray.at(lArrayIx) &= ~lWeekdayBitMask;
  mRec.mCalendarArray.at(lArrayIx) &= ~ALL_WEEK_BIT_MASK;
  mIsDirty = true;
}


bool CalendarRec::GetNextEntry(Weekday  const &aWeekdayRef,
                            Time     const &aTimeRef,
                            Weekday        &aNextWeekdayRef,
                            Time           &aNextTimeRef) {

  bool lIsEntryFound = false;

  // Select time slot next to current time.
  unsigned int lHourIx = GetArrayIx(aTimeRef) + 1;
  lHourIx %= TIME_ENTRY_QTY;
  unsigned int lNextWeekdayMask = WeekdayToBitMask(aWeekdayRef);
  for (unsigned int lTotArrayIx = 0; lTotArrayIx < (TIME_ENTRY_QTY * 7); lTotArrayIx++) {
    // Get weekday bit field of current time.
    // Find the next weekday closest to the current weekday.
    uint8_t lWeekdayField = mRec.mCalendarArray.at(lHourIx);

    if ((lWeekdayField & lNextWeekdayMask) ||
        (lWeekdayField & ALL_WEEK_BIT_MASK)) {
      // Time is at lHourIx.
      // Date is at lWeekdayIx.
      lIsEntryFound = true;
      break;
    }

    // Go to next time entry.
    lHourIx++;
    if (lHourIx >= TIME_ENTRY_QTY) {
      // Wrapped at the end of the day: mask next day.
      lHourIx = 0;
      lNextWeekdayMask <<= 1;
      if (lNextWeekdayMask > (0x1 << 7)) {
        lNextWeekdayMask = 0;
      }
    }
  }

  if (lIsEntryFound) {
    unsigned int lWeekdayUI = BitMaskToWeekday(lNextWeekdayMask);
    aNextWeekdayRef.Set(lWeekdayUI);
  }

  return lIsEntryFound;
}


bool CalendarRec::IsDirty(void) const {
  return mIsDirty;
}


bool CalendarRec::IsSane(void) const {

  // Check magic value.
  if (('C' != mRec.mMagic[0])
      || ('A' != mRec.mMagic[1])
      || ('L' != mRec.mMagic[2])
      || ('R' != mRec.mMagic[3])) {
    return false;
  }

  return true;
}


void CalendarRec::ResetDflt(void) {

  // Set time entries in whole week.
  // 8:00 and 17:00.
  ClrAllEntries();

  Time lTime;
  lTime.SetIs24H(true);
  lTime.SetHours(8);
  SetTimeEntry(lTime);
  lTime.SetHours(17);
  SetTimeEntry(lTime);

  mIsDirty = true;
}


unsigned int CalendarRec::GetRecSize(void) const {
  return mRec.mCalendarArray.size();
}


// Trivial serialization function.
void CalendarRec::Serialize(uint8_t * const aDataPtr) const {

  memcpy(aDataPtr, mRec.mCalendarArray.data(), mRec.mCalendarArray.size());
}


// Trivial serialization function.
void CalendarRec::Deserialize(uint8_t const *aDataPtr) {

  memcpy(mRec.mCalendarArray.data(), aDataPtr, mRec.mCalendarArray.size());
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

unsigned int CalendarRec::GetArrayIx(Time const &aTimeRef) {

  // Find minute bin index: from 0 to (SLOTS_PER_HOUR - 1).
  static const unsigned int lMinuteBinRange = 60 / SLOTS_PER_HOUR;
  unsigned int lHour  = aTimeRef.GetHours();
  unsigned int lBinIx = aTimeRef.GetMinutes() / lMinuteBinRange;

  // Manage pm vs am? Or is time in 24H format always?
  // Index is between 0 to (TIME_ENTRY_QTY - 1).
  return (lHour * SLOTS_PER_HOUR + lBinIx);
}


unsigned int CalendarRec::WeekdayToBitMask(Weekday const &aWeekdayRef) {

  unsigned int lWeekdayUI = aWeekdayRef.Get();
  unsigned int lBitMask   = (1 << lWeekdayUI);
  return lBitMask;
}


unsigned int CalendarRec::BitMaskToWeekday(unsigned int aBitMask) {

  unsigned int lWeekdayUI = 0;
  while (0 == (aBitMask & 0x1)) {
    aBitMask >>= 1;
    lWeekdayUI++;
  }

  return lWeekdayUI;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
