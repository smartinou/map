// *****************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar.
//
// *****************************************************************************

//! \file
//! \brief Feeding calendar class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <string.h>

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

CalendarRec::CalendarRec()
    : DBRec()
    , mRec{0} {

    // Ctor body left intentionally empty.
    ClrAllEntries();
}


CalendarRec::~CalendarRec() {
    // Dtor body left intentionally empty.
}


bool CalendarRec::IsSane(void) {
    // Check CRC.
    if (!IsCRCGood(reinterpret_cast<uint8_t *>(&mRec), sizeof(mRec))) {
        return false;
    }

    // Check magic value.
    if (('C' != mRec.mMagic[0])
          || ('A' != mRec.mMagic[1])
          || ('L' != mRec.mMagic[2])
    ) {
        return false;
    }

    return true;
}


void CalendarRec::ResetDflt(void) {

    // Set magic.
    mRec.mMagic[0] = 'C';
    mRec.mMagic[1] = 'A';
    mRec.mMagic[2] = 'L';

    // Set time entries in whole week.
    // 8:00 and 17:00.
    ClrAllEntries();
    SetTimeEntry(Time(8, 0, 0, true));
    SetTimeEntry(Time(17, 0, 0, true));

    mRec.mCRC = ComputeCRC(reinterpret_cast<uint8_t *>(&mRec), sizeof(mRec));

    SetIsDirty();
}


unsigned int CalendarRec::GetRecSize(void) const {
    return mRec.mCalendarArray.size();
}


void CalendarRec::Serialize(uint8_t * const aDataPtr) const {

    memcpy(aDataPtr, mRec.mCalendarArray.data(), mRec.mCalendarArray.size());
}


void CalendarRec::Deserialize(uint8_t const *aDataPtr) {

    memcpy(mRec.mCalendarArray.data(), aDataPtr, mRec.mCalendarArray.size());
}

//
// Start of child methods.
//

void CalendarRec::ClrAllEntries(void) {

    mRec.mCalendarArray.fill(0);
}


bool CalendarRec::IsEntrySet(Time const &aTime) {

    unsigned int lArrayIx = GetArrayIx(aTime);
    if (mRec.mCalendarArray.at(lArrayIx)) {
        return true;
    }

    return false;
}


void CalendarRec::SetEntry(Weekday const &aWeekday, Time const &aTime) {

    unsigned int lWeekdayUI = aWeekday.Get();
    SetEntry(lWeekdayUI, aTime);
}


void CalendarRec::SetEntry(unsigned int const aWeekday, Time const &aTime) {

    unsigned int lArrayIx        = GetArrayIx(aTime);
    unsigned int lWeekdayBitMask = (1 << aWeekday);

    mRec.mCalendarArray.at(lArrayIx) |= lWeekdayBitMask;

    // Check if all week is set. If so, set the "AllWeek" bit.
    if (~ALL_WEEK_BIT_MASK == (~ALL_WEEK_BIT_MASK & mRec.mCalendarArray.at(lArrayIx))) {
        mRec.mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
    }

    SetIsDirty();
}


void CalendarRec::SetTimeEntry(Time const &aTime) {

    unsigned int lArrayIx = GetArrayIx(aTime);
    mRec.mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
    SetIsDirty();
}


void CalendarRec::ClrEntry(Weekday const &aWeekday, Time const &aTime) {

    unsigned int lWeekdayUI = aWeekday.Get();
    ClrEntry(lWeekdayUI, aTime);
}


void CalendarRec::ClrEntry(unsigned int aWeekday, Time const &aTime) {

    unsigned int lArrayIx        = GetArrayIx(aTime);
    unsigned int lWeekdayBitMask = (0x1 << aWeekday);

    mRec.mCalendarArray.at(lArrayIx) &= ~lWeekdayBitMask;
    mRec.mCalendarArray.at(lArrayIx) &= ~ALL_WEEK_BIT_MASK;
    SetIsDirty();
}


bool CalendarRec::GetNextEntry(
    Weekday  const &aWeekday,
    Time     const &aTime,
    Weekday        &aNextWeekday,
    Time           &aNextTime
) {

    bool lIsEntryFound = false;

    // Select time slot next to current time.
    unsigned int lHourIx = GetArrayIx(aTime) + 1;
    lHourIx %= TIME_ENTRY_QTY;
    unsigned int lNextWeekdayMask = WeekdayToBitMask(aWeekday);
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
                lNextWeekdayMask = 1;
            }
        }
    }

    if (lIsEntryFound) {
        unsigned int lWeekdayUI = BitMaskToWeekday(lNextWeekdayMask);
        aNextWeekday.Set(lWeekdayUI);
    }

    return lIsEntryFound;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

unsigned int CalendarRec::GetArrayIx(Time const &aTime) {

    // Find minute bin index: from 0 to (SLOTS_PER_HOUR - 1).
    static const unsigned int lMinuteBinRange = 60 / SLOTS_PER_HOUR;
    unsigned int lHour  = aTime.GetHours();
    unsigned int lBinIx = aTime.GetMinutes() / lMinuteBinRange;

    // Manage pm vs am? Or is time in 24H format always?
    // Index is between 0 to (TIME_ENTRY_QTY - 1).
    return (lHour * SLOTS_PER_HOUR + lBinIx);
}


unsigned int CalendarRec::WeekdayToBitMask(Weekday const &aWeekday) {

    unsigned int lWeekdayUI = aWeekday.Get();
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