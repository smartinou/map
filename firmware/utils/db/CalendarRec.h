#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar DB.
//
// *******************************************************************************

//! \file
//! \brief Feeding calendar DB class.
//! \ingroup utils_db

// ******************************************************************************
//
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard Libraries.
#include <array>
#include <bitset>

#include <time/Time.h>
#include <date/Weekday.h>

#include "DBRec.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************


//! \brief Feeding calendar database class.
class CalendarRec
    : public DBRec {
public:
    CalendarRec();
    ~CalendarRec();

    // DBRec interface.
    bool IsSane(void) const override;
    void ResetDflt(void) override;

    // Extended object's interface.

    // Sets/clears the entry for the specified time, rounded to quarter hour.
    void SetEntry(Weekday const &aWeekday, Time const &aTime);
    void SetEntry(unsigned int const aWeekday, Time const &aTime);
    void ClrEntry(Weekday const &aWeekday, Time const &aTime);
    void ClrEntry(unsigned int const aWeekday, Time const &aTime);
    void SetTimeEntry(Time const &aTime);
    void ClrAllEntries(void);

    bool IsEntrySet(Time const &aTime);

    // Gets the next set entry from current time.
    bool GetNextEntry(
        Weekday const &aWeekday,
        Time    const &aTime,
        Weekday       &aNextWeekday,
        Time          &aNextTime
    );

private:
    // DBRec interface.
    size_t GetRecSize(void) const override;
    void Serialize(uint8_t * const aData) const override;
    void Deserialize(uint8_t const * const aData) override;
    void UpdateCRC(void) override;

    unsigned int GetArrayIx(Time const &aTime);
    unsigned int WeekdayToBitMask(Weekday const &aWeekday);
    unsigned int BitMaskToWeekday(unsigned int aBitMask);

    enum CalendarDimEnumTag {
        HOUR_QTY          = 24,
        SLOTS_PER_HOUR    = 4,
        TIME_ENTRY_QTY    = HOUR_QTY * SLOTS_PER_HOUR,
        WEEKDAY_ENTRY_QTY = 7
    };

    enum BitMaskEnumTag { ALL_WEEK_BIT_MASK = (0x1 << 0) };

    struct RecData {
        BaseRec mBase;
        std::array<uint8_t, TIME_ENTRY_QTY> mCalendarArray;
    };

    struct RecData mRec;

    static char constexpr sMagic[3] = { 'C', 'A', 'L' };
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
