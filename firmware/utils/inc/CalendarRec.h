#ifndef CALENDAR_REC_H_
#define CALENDAR_REC_H_
// *******************************************************************************
//
// Project: Beast Feed'Her.
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

class Time;
class Weekday;

//! \brief Brief description.
//! Details follow...
//! ...here.
class CalendarRec : public DBRec {
 public:
  CalendarRec();
  ~CalendarRec() {}

  // Sets/clears the entry for the specified time, rounded to quarter hour.
  void SetEntry(Weekday const &aWeekdayRef, Time const &aTimeRef);
  void SetEntry(unsigned int const aWeekday, Time const &aTimeRef);
  void ClrEntry(Weekday const &aWeekdayRef, Time const &aTimeRef);
  void ClrEntry(unsigned int const aWeekday, Time const &aTimeRef);
  void SetTimeEntry(Time const &aTimeRef);
  void ClrAllEntries(void);

  bool IsEntrySet(Time const &aTimeRef);

  // Gets the next set entry from current time.
  bool GetNextEntry(Weekday const &aWeekdayRef,
                    Time    const &aTimeRef,
                    Weekday       &aNextWeekdayRef,
                    Time          &aNextTimeRef);

  bool IsSane(void);
  bool IsDirty(void) const;
  void ResetDflt(void);

  // Simple Serialize/Deserialize methods.
  unsigned int GetRecSize(void) const;
  void Serialize(  uint8_t       * const aDataPtr) const;
  void Deserialize(uint8_t const * const aDataPtr);

 private:
  enum CalendarDimEnumTag {
    HOUR_QTY          = 24,
    SLOTS_PER_HOUR    = 4,
    TIME_ENTRY_QTY    = HOUR_QTY * SLOTS_PER_HOUR,
    WEEKDAY_ENTRY_QTY = 7
  };

  enum BitMaskEnumTag { ALL_WEEK_BIT_MASK = (0x1 << 0) };

  unsigned int GetArrayIx(Time const &aTimeRef);
  unsigned int WeekdayToBitMask(Weekday const &aWeekdayRef);
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
#endif // CALENDAR_REC_H_
