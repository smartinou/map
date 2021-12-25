// *****************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar DB.
//
// *****************************************************************************

//! \file
//! \brief Feeding calendar DB class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
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

char constexpr CalendarRec::sMagic[3];

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


bool CalendarRec::IsSane(void) const {
    // Check magic.
    if (!IsMagicGood(reinterpret_cast<BaseRec const * const>(&mRec.mBase), sMagic)) {
        return false;
    }

    // Check CRC.
    if (!IsCRCGood(reinterpret_cast<uint8_t const * const>(&mRec), sizeof(struct RecData))) {
        return false;
    }

    return true;
}


void CalendarRec::ResetDflt(void) {
    mRec.mBase.mMagic[0] = sMagic[0];
    mRec.mBase.mMagic[1] = sMagic[1];
    mRec.mBase.mMagic[2] = sMagic[2];

    // Set time entries in whole week.
    // 8:00 and 17:00.
    ClrAllEntries();
    SetTimeEntry(Time(8, 0, 0, true));
    SetTimeEntry(Time(17, 0, 0, true));

    SetIsDirty();
}


size_t CalendarRec::GetRecSize(void) const {
    return static_cast<size_t>(sizeof(struct BaseRec) + mRec.mCalendarArray.size());
}


void CalendarRec::Serialize(uint8_t * const aDataPtr) const {

    memcpy(aDataPtr, &mRec, GetRecSize());
}


void CalendarRec::Deserialize(uint8_t const * const aDataPtr) {

    memcpy(&mRec, aDataPtr, GetRecSize());
}


void CalendarRec::UpdateCRC() {
    mRec.mBase.mCRC = ComputeCRC(
        reinterpret_cast<uint8_t const * const>(&mRec),
        sizeof(struct RecData)
    );
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


void CalendarRec::ClrEntry(unsigned int const aWeekday, Time const &aTime) {

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
        Time lNextTime(lHourIx / SLOTS_PER_HOUR, (lHourIx % SLOTS_PER_HOUR) * (60 / SLOTS_PER_HOUR), 0, true);
        aNextTime = lNextTime;
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
