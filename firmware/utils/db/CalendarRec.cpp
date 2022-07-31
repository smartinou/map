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
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// This project.
#include "CalendarRec.h"

// Standard Library.
#include <cstring>

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

auto CalendarRec::IsSane() const noexcept -> bool {

    bool const lIsMagicGood = IsMagicGood(mRec.mBase, sMagic);
    if (lIsMagicGood) {
        return IsCRCGood(
            {
                reinterpret_cast<uint8_t const * const>(&mRec),
                sizeof(struct RecData)
            }
        );
    }

    return false;
}


void CalendarRec::ResetDflt() noexcept {
    mRec.mBase.mMagic = sMagic;

    // Set time entries in whole week.
    // 8:00 and 17:00.
    ClrAllEntries();
    {
        static constexpr Time sDefaultTime{8, 0, 0, true};
        SetTimeEntry(sDefaultTime);
    }
    {
        static constexpr Time sDefaultTime{17, 0, 0, true};
        SetTimeEntry(sDefaultTime);
    }

    SetIsDirty();
}


//
// Start of child methods.
//

void CalendarRec::ClrAllEntries() noexcept {

    mRec.mCalendarArray.fill(0);
}


auto CalendarRec::IsEntrySet(Time const &aTime) noexcept -> bool {

    auto const lArrayIx = GetArrayIx(aTime);
    return static_cast<bool>(mRec.mCalendarArray.at(lArrayIx));
}


void CalendarRec::SetEntry(Weekday const &aWeekday, Time const &aTime) noexcept {

    auto const lWeekdayUI = aWeekday.Get();
    SetEntry(lWeekdayUI, aTime);
}


void CalendarRec::SetEntry(unsigned int const aWeekday, Time const &aTime) noexcept {

    auto const lArrayIx = GetArrayIx(aTime);
    auto const lWeekdayBitMask{static_cast<unsigned int>(1 << aWeekday)};

    mRec.mCalendarArray.at(lArrayIx) |= lWeekdayBitMask;

    // Check if all week is set. If so, set the "AllWeek" bit.
    if (~ALL_WEEK_BIT_MASK == (~ALL_WEEK_BIT_MASK & mRec.mCalendarArray.at(lArrayIx))) {
        mRec.mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
    }

    SetIsDirty();
}


void CalendarRec::SetTimeEntry(Time const &aTime) noexcept {

    auto const lArrayIx = GetArrayIx(aTime);
    mRec.mCalendarArray.at(lArrayIx) |= ALL_WEEK_BIT_MASK;
    SetIsDirty();
}


void CalendarRec::ClrEntry(Weekday const &aWeekday, Time const &aTime) noexcept {

    auto const lWeekdayUI = aWeekday.Get();
    ClrEntry(lWeekdayUI, aTime);
}


void CalendarRec::ClrEntry(unsigned int const aWeekday, Time const &aTime) noexcept {

    auto const lArrayIx = GetArrayIx(aTime);
    auto const lWeekdayBitMask = (0x1 << aWeekday);

    mRec.mCalendarArray.at(lArrayIx) &= ~lWeekdayBitMask;
    mRec.mCalendarArray.at(lArrayIx) &= ~ALL_WEEK_BIT_MASK;
    SetIsDirty();
}


auto CalendarRec::GetNextEntry(
    Weekday const &aWeekday,
    Time const &aTime
) const noexcept -> std::optional<std::pair<Time, Weekday>> {

    bool lIsEntryFound{false};

    // Select time slot next to current time.
    auto lHourIx = GetArrayIx(aTime) + 1;
    lHourIx %= sTimeEntryQty;
    auto lNextWeekdayMask = WeekdayToBitMask(aWeekday);
    for (auto lTotArrayIx{0};
        lTotArrayIx < (sTimeEntryQty * sWeekdayEntryQty);
        ++lTotArrayIx)
    {
        // Get weekday bit field of current time.
        // Find the next weekday closest to the current weekday.
        uint8_t const lWeekdayField = mRec.mCalendarArray.at(lHourIx);

        if ((lWeekdayField & lNextWeekdayMask) ||
            (lWeekdayField & ALL_WEEK_BIT_MASK)) {
            // Time is at lHourIx.
            // Date is at lWeekdayIx.
            lIsEntryFound = true;
            break;
        }

        // Go to next time entry.
        lHourIx++;
        if (lHourIx >= sTimeEntryQty) {
            // Wrapped at the end of the day: mask next day.
            lHourIx = 0;
            lNextWeekdayMask <<= 1;
            if (lNextWeekdayMask > (0x1 << sWeekdayEntryQty)) {
                lNextWeekdayMask = 1;
            }
        }
    }

    if (lIsEntryFound) {
        Time const lNextTime(
            lHourIx / sSlotsPerHours,
            (lHourIx % sSlotsPerHours) * (60 / sSlotsPerHours),
            0,
            true
        );
        auto const lWeekdayUI = BitMaskToWeekday(lNextWeekdayMask);
        auto const lNextWeekday(lWeekdayUI);
        auto const lNextEntry = std::make_pair(lNextTime, lNextWeekday);
        return std::optional<std::pair<Time, Weekday>>{lNextEntry};
    }

    return std::nullopt;
}


auto CalendarRec::GetNextEntry(
    CalendarRec::TimeAndDate const &aEntry
) const noexcept -> std::optional<CalendarRec::TimeAndDate>
{
    if (auto const lNextEntry = GetNextEntry(aEntry.mWeekday, aEntry.mTime)) {
        TimeAndDate const lReturn = {
            lNextEntry.value().first, lNextEntry.value().second
        };
        return std::optional<TimeAndDate>{lReturn};
    }

    return std::nullopt;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

auto CalendarRec::GetRecSize() const noexcept -> size_t {
    return static_cast<size_t>(sizeof(struct BaseRec) + mRec.mCalendarArray.size());
}


void CalendarRec::Serialize(uint8_t * const aDataPtr) const {

    std::memcpy(aDataPtr, &mRec, GetRecSize());
}


void CalendarRec::Deserialize(uint8_t const * const aDataPtr) {

    std::memcpy(&mRec, aDataPtr, GetRecSize());
}


void CalendarRec::UpdateCRC() noexcept {
    mRec.mBase.mCRC = ComputeCRC(
        {
            reinterpret_cast<uint8_t const * const>(mRec.mBase.mMagic.data()),
            sizeof(struct RecData) - 1
        }
    );
}


auto CalendarRec::GetArrayIx(Time const &aTime) noexcept -> unsigned int {

    // Find minute bin index: from 0 to (SLOTS_PER_HOUR - 1).
    static constexpr unsigned int lMinuteBinRange{60 / sSlotsPerHours};
    unsigned int const lHour  = aTime.GetHours();
    unsigned int const lBinIx = aTime.GetMinutes() / lMinuteBinRange;

    // Manage pm vs am? Or is time in 24H format always?
    // Index is between 0 to (sTimeEntryQty - 1).
    return (lHour * sSlotsPerHours + lBinIx);
}


auto CalendarRec::WeekdayToBitMask(Weekday const &aWeekday) noexcept -> unsigned int {

    auto const lWeekdayUI = aWeekday.Get();
    auto const lBitMask{static_cast<unsigned int>(1 << lWeekdayUI)};
    return lBitMask;
}


auto CalendarRec::BitMaskToWeekday(unsigned int aBitMask) noexcept -> unsigned int {

    unsigned int lWeekdayUI{0};
    while (0 == (aBitMask & 0x1)) {
        aBitMask >>= 1;
        lWeekdayUI++;
    }

    return lWeekdayUI;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
