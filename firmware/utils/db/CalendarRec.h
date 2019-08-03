#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2016-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard Libraries.
#include <array>
#include <bitset>

#include "DBRec.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declarations.
class Time;
class Weekday;


//! \brief Brief description.
//! Details follow...
//! ...here.
class CalendarRec
    : public DBRec {
public:
    CalendarRec();
    ~CalendarRec();

    // DBRec.
    bool IsSane(void) override;
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
    enum CalendarDimEnumTag {
        HOUR_QTY          = 24,
        SLOTS_PER_HOUR    = 4,
        TIME_ENTRY_QTY    = HOUR_QTY * SLOTS_PER_HOUR,
        WEEKDAY_ENTRY_QTY = 7
    };

    enum BitMaskEnumTag { ALL_WEEK_BIT_MASK = (0x1 << 0) };

    unsigned int GetRecSize(void) const override;
    void Serialize(uint8_t * const aData) const override;
    void Deserialize(uint8_t const * const aData) override;

    unsigned int GetArrayIx(Time const &aTime);
    unsigned int WeekdayToBitMask(Weekday const &aWeekday);
    unsigned int BitMaskToWeekday(unsigned int aBitMask);

    struct RecStructTag {
        uint8_t mCRC;
        char    mMagic[3];
        std::array<uint8_t, TIME_ENTRY_QTY> mCalendarArray;
    };

    struct RecStructTag mRec;
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
