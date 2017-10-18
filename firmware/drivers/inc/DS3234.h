#ifndef DS3234_H_
#define DS3234_H_
// *******************************************************************************
//
// Project: Larger project scope.
//
// Module: Module in the larger project scope.
//
// *******************************************************************************

//! \file
//! \brief RTC base class.
//! \ingroup ext_peripherals


// ******************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "RTC.h"
#include "SPI.h"

using namespace CoreLink;

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// [MG] CONSIDER MOVING THIS INTO THE CLASS ITSELF.
typedef uint8_t volatile rtc_reg_t;

struct L_TIME_STRUCT_TAG {
  rtc_reg_t mSeconds;
  rtc_reg_t mMinutes;
  rtc_reg_t mHours;
};


struct L_DATE_STRUCT_TAG {
  rtc_reg_t mWeekday;
  rtc_reg_t mDate;
  rtc_reg_t mMonth;
  rtc_reg_t mYear;
};

typedef struct L_TIME_STRUCT_TAG time2_t;
typedef struct L_DATE_STRUCT_TAG date_t;

struct L_ALARM_STRUCT_TAG {
  rtc_reg_t mMinutes;
  rtc_reg_t mHours;
  rtc_reg_t mDayDate;
};

typedef struct L_ALARM_STRUCT_TAG rtc_alarm_t;


struct L_ADDR_MAP_STRUCT_TAG {
  time2_t      mTime;
  date_t      mDate;
  rtc_reg_t   mAlarm1Seconds;
  rtc_alarm_t mAlarm1;
  rtc_alarm_t mAlarm2;
  rtc_reg_t   mCtrl;
  rtc_reg_t   mStatus;
  rtc_reg_t   mAgingOffset;
  rtc_reg_t   mTempMSB;
  rtc_reg_t   mTempLSB;
  rtc_reg_t   mDisableTemp;
  rtc_reg_t   mReserved[4];
  rtc_reg_t   mSRAMAddr;
  rtc_reg_t   mSRAMDData;
};

typedef struct L_ADDR_MAP_STRUCT_TAG rtc_reg_map_t;

//class SPI;


//! \brief Brief description.
//! Details follow...
//! ...here.
class DS3234 : public RTC {

 public:
  enum class ALARM_ID {
    ALARM_ID_1 = 0,
    ALARM_ID_2,
  };

  enum class ALARM_MODE {
#if 0
    DY   = (0x1 << 4),
    A1M4 = (0x1 << 3),
    A1M3 = (0x1 << 2),
    A1M2 = (0x1 << 1),
    A1M1 = (0x1 << 0),
    ONCE_PER_SEC = (A1M4 | A1M3 | A1M2 | A1M1),
    WHEN_SECS_MATCH = (A1M4 | A1M3 | A1M2),
    WHEN_MINS_SECS_MATCH = (A1M4 | A1M3),
    WHEN_HOURS_MINS_SECS_MATCH = (A1M4),
    WHEN_DAY_HOURS_MINS_SECS_MATCH  = (DY),
    WHEN_DATE_HOURS_MINS_SECS_MATCH = (0x0),
#else

      ONCE_PER_SEC = 0,
      WHEN_SECS_MATCH,
      ONCE_PER_MINUTE = WHEN_SECS_MATCH,
      WHEN_MINS_SECS_MATCH,
      WHEN_MINS_MATCH = WHEN_MINS_SECS_MATCH,
      WHEN_HOURS_MINS_SECS_MATCH,
      WHEN_HOURS_MINS_MATCH = WHEN_HOURS_MINS_SECS_MATCH,
      WHEN_DAY_HOURS_MINS_SECS_MATCH,
      WHEN_DAY_HOURS_MINS_MATCH = WHEN_DAY_HOURS_MINS_SECS_MATCH,
      WHEN_DATE_HOURS_MINS_SECS_MATCH,
      WHEN_DATE_HOURS_MINS_MATCH = WHEN_DATE_HOURS_MINS_SECS_MATCH

#endif
  };


  DS3234(SPIDev      &aSPIDevRef,
	 SPISlaveCfg &aSPICfgRef);
  ~DS3234();

  void Init(uint8_t aCtrlRef);

  // Polled API.
  void RdTime(Time &aTimeRef);
  void RdDate(Date &aDateRef);
  void RdTimeAndDate(Time &aTimeRef, Date &aDateRef);
  void RdFromRAM(uint8_t     *aDataPtr,
		 unsigned int aOffset,
		 unsigned int aSize);

  void WrTime(Time const &aTimeRef);
  void WrDate(Date const &aDateRef);
  void WrTimeAndDate(Time const &aTimeRef, Date const &aDateRef);
  void WrAlarm(enum ALARM_ID   aAlarmID,
	       Time const     &aTimeRef,
	       Date const     &aDateRef,
	       enum ALARM_MODE aAlarmMode = ALARM_MODE::WHEN_DATE_HOURS_MINS_SECS_MATCH);
  void WrAlarm(enum ALARM_ID   aAlarmID,
	       Time    const  &aTimeRef,
	       Weekday const  &aWeekdayRef,
	       enum ALARM_MODE aAlarmMode = ALARM_MODE::WHEN_DAY_HOURS_MINS_SECS_MATCH);
  void DisableAlarm(enum ALARM_ID aAlarmID);
  void ClrAlarmFlag(enum ALARM_ID aAlarmID);

  void WrToRAM(uint8_t const *aDataPtr,
	       unsigned int   aOffset,
	       unsigned int   aSize);

  // Interrupt-based/cached API.
  void    GetTimeAndDate(Time &aTimeRef, Date &aDateRef);
  uint8_t GetCtrl(void);
  uint8_t GetStatus(void);
  float   GetTemperature(void);

 protected:
  void UpdateCachedVal(void);

 private:
  enum Hours : uint8_t {
    H12_24_n = (0x1 << 6),
    PM_AM_n  = (0x1 << 5),
  };

  enum Month : uint8_t {
    CENTURY = (0x1 << 7),
  };

  enum DayDate : uint8_t {
    DAY_DATE_n = (0x1 << 6),
    AnMx       = (0x1 << 7)
  };

  enum Addr : uint8_t {
    WR_BASE_ADDR = 0x80
  };

 public:
  /* -------------------------------------------------------------------------
   Control Register : 0x0E/8Eh
   Name    Value       Description
   ----    ---------   -------------------------------------------------------
   EOSCn   x--- ----   Enable Oscillator.
   BBSQW   -x-- ----   Battery-Backed Square-Wave Enable.
   CONV    --x- ----   Convert Temperature.
   RS2     ---x ----   Rate selection bit 1.
   RS1     ---- x---   Rate selection bit 2.
   INTCn   ---- -x--   Interrupt Control.
   AEI2    ---- --x-   Alarm 2 Interrupt Enable.
   AEI1    ---- ---x   Alarm 1 Interrupt Enable.
   ------------------------------------------------------------------------- */
  enum Ctrl : uint8_t {
    AEI1  = (0x1 << 0),
    AEI2  = (0x1 << 1),
    INTCn = (0x1 << 2),
    RS1   = (0x1 << 3),
    RS2   = (0x1 << 4),
    CONV  = (0x1 << 5),
    BBSQW = (0x1 << 6),
    EOSCn = (0x1 << 7)
  };

  /* -------------------------------------------------------------------------
   Control/Status Register : 0x0F/8Fh
   Name    Value       Description
   ----    ---------   -------------------------------------------------------
   OSF     x--- ----   Oscillator Stop Flag.
   BB32K   -x-- ----   Battery-Backed 32KHz Output.
   CRATE1  --x- ----   Conversion Rate 1.
   CRATE0  ---x ----   Conversion Rate 2.
   EN32K   ---- x---   Enable 32KHz Output.
   BSY     ---- -x--   Busy.
   AF2     ---- --x-   Alarm 2 Flag.
   AF1     ---- ---x   Alarm 1 Flag.
   ------------------------------------------------------------------------- */
  enum Status : uint8_t {
    AF1    = (0x1 << 0),
    AF2    = (0x1 << 1),
    BSY    = (0x1 << 2),
    EN32K  = (0x1 << 3),
    CRATE0 = (0x1 << 4),
    CRATE1 = (0x1 << 5),
    BB32K  = (0x1 << 6),
    ESF    = (0x1 << 7)
  };

 private:
  void UpdateTime(Time &aTimeRef);
  void UpdateDate(Date &aDateRef);

  void FillTimeStruct(Time const &aTimeRef);
  void FillDateStruct(Date const &aDateRef);
  void FillAlarmStruct(rtc_alarm_t &aAlarmRef,
		       Time const  &aTimeRef,
		       Date const  &aDateRef);
  void FillAlarmStruct(rtc_alarm_t   &aAlarmRef,
		       Time    const &aTimeRef,
		       Weekday const &aWeekdayRef);
  void FillAlarmTimeStruct(rtc_alarm_t &aAlarmRef,
			   Time const  &aTimeRef);
  void FillAlarmModeStruct(rtc_alarm_t    &aAlarmRef,
			   enum ALARM_MODE aAlarmMode);
  void TxAlarmStruct(enum ALARM_ID aAlarmID);

  void SetAlarm(enum ALARM_ID aAlarmID);

  CoreLink::SPIDev      &mSPIDevRef;
  CoreLink::SPISlaveCfg &mSPICfgRef;

  rtc_reg_map_t mRegMap;
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
#endif // DS3234_H_
