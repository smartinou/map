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
//        Copyright (c) 2016, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <string.h>

// TI Library.

// QP Library.

// Common Library.
#include "DBRec.h"
#include "Date.h"
#include "Time.h"

// This project.
#include "Calendar.h"

//Q_DEFINE_THIS_FILE

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

Calendar::Calendar() :
  DBRec() {

  // Prepare calendar storage for deserialization.
  ClrAllEntries();
}


void Calendar::ClrAllEntries(void) {

  mCalendarArray.fill(0);
}


void Calendar::SetEntry(Weekday const &aWeekdayRef, Time const &aTimeRef) {

  unsigned int lArrayIx        = GetArrayIx(aTimeRef);
  unsigned int lWeekdayBitMask = WeekdayToBitMask(aWeekdayRef);

  mCalendarArray.at(lArrayIx) |= lWeekdayBitMask;

  // Check if all week is set. If so, set the "AllWeek" bit.
  if (~ALL_WEEK_BIT_MASK == (~ALL_WEEK_BIT_MASK & mCalendarArray.at(lArrayIx))) {
    mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
  }
}


void Calendar::SetTimeEntry(Time const &aTimeRef) {

  unsigned int lArrayIx = GetArrayIx(aTimeRef);
  mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
}


void Calendar::ClrEntry(Weekday const &aWeekdayRef, Time const &aTimeRef) {

  unsigned int lArrayIx        = GetArrayIx(aTimeRef);
  unsigned int lWeekdayBitMask = WeekdayToBitMask(aWeekdayRef);

  mCalendarArray.at(lArrayIx) &= ~lWeekdayBitMask;
  mCalendarArray.at(lArrayIx) &= ~ALL_WEEK_BIT_MASK;
}


bool Calendar::GetNextEntry(Weekday  const &aWeekdayRef,
			    Time     const &aTimeRef,
		            Weekday        &aNextWeekdayRef,
			    Time           &aNextTimeRef) {

  bool lIsEntryFound = false;

  // Select time slot next to current time.
  unsigned int lHourIx = GetArrayIx(aTimeRef) + 1;
  unsigned int lNextWeekdayMask = WeekdayToBitMask(aWeekdayRef);
  for (unsigned int lTotArrayIx = 0; lTotArrayIx < (TIME_ENTRY_QTY * 7); lTotArrayIx++) {
    // Get weekday bit field of current time.
    // Find the next weekday closest to the current weekday.
    uint8_t lWeekdayField = mCalendarArray.at(lHourIx);

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


// Trivial serialization function.
void Calendar::Serialize(uint8_t * const aDataPtr) {

  memcpy(aDataPtr, mCalendarArray.data(), mCalendarArray.size());
}


// Trivial serialization function.
void Calendar::Deserialize(uint8_t const *aDataPtr) {

  memcpy(mCalendarArray.data(), aDataPtr, mCalendarArray.size());
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

unsigned int Calendar::GetArrayIx(Time const &aTimeRef) {

  static const unsigned int lMinuteBinRange = 60 / SLOTS_PER_HOUR;
  unsigned int lHour  = aTimeRef.GetHours();
  unsigned int lBinIx = aTimeRef.GetMinutes() / lMinuteBinRange;

  // Manage pm vs am? Or is time in 24H format always?
  return (lHour * SLOTS_PER_HOUR + lBinIx);
}


unsigned int Calendar::WeekdayToBitMask(Weekday const &aWeekdayRef) {

  unsigned int lWeekdayUI = aWeekdayRef.Get();
  unsigned int lBitMask   = (1 << lWeekdayUI);
  return lBitMask;
}


unsigned int Calendar::BitMaskToWeekday(unsigned int aBitMask) {

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
