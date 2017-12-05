// *****************************************************************************
//
// Project: Active Object Library
//
// Module: RTCC QP Active Object.
//
// *****************************************************************************

//! \file
//! \brief RTCC QActive Object class.
//! \ingroup qp_ao
//!
//! This is the RTCC AO of the system.
//! It has control over the RTC and managed the feeding signals comming from
//! the RTC alarm.

// *****************************************************************************
//
//        Copyright (c) 2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************
#define RTCC_DBG
//#undef RTCC_DBG


// Standard Library.
#include <stddef.h>

// TI Library.
#include "hw_types.h"
#include "gpio.h"
#include "interrupt.h"
#ifdef RTCC_DBG
#include "uartstdio.h"
#endif // RTCC_DBG

// QP Library.
#include "qpcpp.h"

// Common Library.
#include "Date.h"
//#include "DBRec.h"
#include "DS3234.h"
#include "SPI.h"
#include "Time.h"

// This project.
#include "RTCC_AO.h"
#include "RTCC_Evt.h"
#include "BSP.h"

Q_DEFINE_THIS_FILE

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

#define RTCC_CALENDAR_DBG
//#undef  RTCC_CALENDAR_DBG

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

RTCC_AO *RTCC_AO::mInstancePtr = static_cast<RTCC_AO *>(0);

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

RTCC_AO::RTCC_AO() :
  QActive(Q_STATE_CAST(&RTCC_AO::Initial)),
  mTime(),
  mDate(),
  mTemperature(0.0),
  mRTCSPISlaveCfgPtr(static_cast<CoreLink::SPISlaveCfg *>(0)),
  mDS3234Ptr(static_cast<DS3234 *>(0)),
  mCalendarPtr(nullptr),
  mIntNbr(0) {

  // Ctor body.
  mInstancePtr = this;
}


void RTCC_AO::RdDBRec(DBRec * const aDBRecPtr) {

  if (mDS3234Ptr->HasNVMem()) {
    uint8_t      lSRAMData[256];
    unsigned int lRecSize = aDBRecPtr->GetRecSize();

    mDS3234Ptr->RdFromRAM(&lSRAMData[0], 0, lRecSize);
    aDBRecPtr->Deserialize(&lSRAMData[0]);
    if (!aDBRecPtr->IsSane()) {
      // Reset defaults and write back to NV mem.
      aDBRecPtr->ResetDflt();
      aDBRecPtr->Serialize(&lSRAMData[0]);
      mDS3234Ptr->WrToRAM(&lSRAMData[0], 0, lRecSize);
    }
  } else {
    aDBRecPtr->ResetDflt();
  }
}


void RTCC_AO::ISRCallback(void) {
  // Static event.
  static QP::QEvt const sRTCCAlarmIntEvt = { SIG_RTCC_INTERRUPT, 0U, 0U };
  mDS3234Ptr->ISRCallback();

  // Signal to AO that RTCC generated an interrupt.
  POST(&sRTCCAlarmIntEvt, 0);
}


RTCC_AO * const RTCC_AO::GetInstancePtr(void) {
  return mInstancePtr;
}


QP::QActive * const RTCC_AO::GetOpaqueAOInstancePtr(void) {
  return static_cast<QP::QActive *>(mInstancePtr);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

QP::QState RTCC_AO::Initial(RTCC_AO        * const me,  //aMePtr,
                            QP::QEvt const * const e) { //aEvtPtr

  // FIXME: Determine what to return in case of failure of 'new'.
  int lResult = InitRTCC(me, e);

  // Subscribe to signals if any.
  //aMe->subscribe(<>_SIG);

  lResult = InitCalendar(me, e);
  lResult = InitInterrupt(me, e);

  (void)lResult;

  return Q_TRAN(&RTCC_AO::Running);
}


unsigned int RTCC_AO::InitRTCC(RTCC_AO         * const me,  //aMePtr,
                               QP::QEvt  const * const e) { //aEvtPtr

  RTCCInitEvt const *lRTCCInitEvtPtr = static_cast<RTCCInitEvt const *>(e);

  // Create an SPI slave configuration for the DS3234 RTCC.
  // TODO: Verify if this can be changed into local var.
  me->mRTCSPISlaveCfgPtr = new CoreLink::SPISlaveCfg();
  me->mRTCSPISlaveCfgPtr->SetProtocol(CoreLink::SPISlaveCfg::MOTO_1);
  me->mRTCSPISlaveCfgPtr->SetBitRate(4000000);
  me->mRTCSPISlaveCfgPtr->SetDataWidth(8);
  me->mRTCSPISlaveCfgPtr->SetCSnGPIO(lRTCCInitEvtPtr->mCSnGPIOPort,
                                     lRTCCInitEvtPtr->mCSnGPIOPin);

  // Create & initialize the DS3234 RTCC.
  // Configure the RTCC to desired state:
  //   -Oscillator enabled.
  //   -Battery-backed square wave disabled.
  //   (-Rate select: 1KHz square wave.)
  //   -Interrupt Control enabled.
  //   -Alarm 1 enabled.
  //     -1Hz interrupt rate.
  me->mDS3234Ptr = new DS3234(lRTCCInitEvtPtr->mSPIDevRef,
                              *me->mRTCSPISlaveCfgPtr);
  me->mDS3234Ptr->Init(DS3234::Ctrl::INTCn);

  return 0;
}


unsigned int RTCC_AO::InitCalendar(RTCC_AO         * const me,  //aMePtr,
                                   QP::QEvt  const * const e) { //aEvtPtr

  // -Get current time & date, temperature.
  // -Read config from RTCC sram.
  // -Populate calendar.
  // -Set next alarm.
  me->mDS3234Ptr->RdTimeAndDate(me->mTime, me->mDate);
  me->mTemperature = me->mDS3234Ptr->GetTemperature();

  RTCCInitEvt const * const lInitEvtPtr = static_cast<RTCCInitEvt const * const>(e);
  if (nullptr == lInitEvtPtr->mCalendarPtr) {
    me->mCalendarPtr = new CalendarRec();
  } else {
    me->mCalendarPtr = lInitEvtPtr->mCalendarPtr;
  }

  // Set some defaults if required.
  if (!me->mCalendarPtr->IsSane()) {
    me->mCalendarPtr->ResetDflt();
  }

  // Check if a Calendar object was passed.
  // If not, create one locally,
#ifdef RTCC_CALENDAR_DBG
  me->mCalendarPtr->ClrAllEntries();
#endif // RTCC_CALENDAR_DBG

  SetNextCalendarEvt(me);
  return 0;
}


unsigned int RTCC_AO::InitInterrupt(RTCC_AO         * const me,  //aMePtr,
                                    QP::QEvt  const * const e) { //aEvtPtr

  // Set interrupt pin and periodic alarm.
  RTCCInitEvt const *lRTCCInitEvtPtr = static_cast<RTCCInitEvt const *>(e);
  me->mIntNbr = BSPGPIOPortToInt(lRTCCInitEvtPtr->mIRQGPIOPort);
  IntDisable(me->mIntNbr);

  GPIOPinTypeGPIOInput(lRTCCInitEvtPtr->mIRQGPIOPort,
                       lRTCCInitEvtPtr->mIRQGPIOPin);
  GPIOIntTypeSet(lRTCCInitEvtPtr->mIRQGPIOPort,
                 lRTCCInitEvtPtr->mIRQGPIOPin,
                 GPIO_FALLING_EDGE);
  GPIOPadConfigSet(lRTCCInitEvtPtr->mIRQGPIOPort,
                   lRTCCInitEvtPtr->mIRQGPIOPin,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);

  me->mDS3234Ptr->WrAlarm(DS3234::ALARM_ID::ALARM_ID_1,
                          me->mTime,
                          me->mDate,
                          DS3234::ALARM_MODE::ONCE_PER_SEC);

  GPIOPinIntEnable(lRTCCInitEvtPtr->mIRQGPIOPort, lRTCCInitEvtPtr->mIRQGPIOPin);
  // [MG] THIS CLEARS THE 1ST INTERRUPT. IT DOESN'T COME UP UNTIL FLAGS ARE CLEARED.
  //GPIOPinIntClear(lGPIOInitEvtPtr->mIRQGPIOPort, lGPIOInitEvtPtr->mIRQGPIOPin);
  return 0;
}


QP::QState RTCC_AO::Running(RTCC_AO        * const me,  //aMePtr,
                            QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    IntEnable(me->mIntNbr);
    return Q_HANDLED();

  case SIG_RTCC_INTERRUPT: {
    // Read complete RTCC, updating Time & Date at the same time.
    me->mDS3234Ptr->GetTimeAndDate(me->mTime, me->mDate);
    me->mTemperature = me->mDS3234Ptr->GetTemperature();
    me->mDS3234Ptr->ClrAlarmFlag(DS3234::ALARM_ID::ALARM_ID_1);
#ifdef RTCC_DBG
    if ((0 == me->mTime.GetMinutes()) && (0 == me->mTime.GetSeconds())) {
      UARTprintf("\nH\n");
    } else if (0 == me->mTime.GetSeconds()){
      UARTprintf("\nM\n");
    } else {
      UARTprintf(".");
    }
#endif // RTCC_DBG

    // Publish Tick Alarm Event.
    RTCCTimeDateEvt *lTickAlarmEvtPtr = Q_NEW(RTCCTimeDateEvt, SIG_RTCC_TIME_TICK_ALARM);
    lTickAlarmEvtPtr->mTime = me->mTime;
    lTickAlarmEvtPtr->mDate = me->mDate;
    QP::QF::PUBLISH(static_cast<QP::QEvt *>(lTickAlarmEvtPtr), me);

    if ((DS3234::AF2  & me->mDS3234Ptr->GetStatus()) &&
        (DS3234::AEI2 & me->mDS3234Ptr->GetCtrl())) {
      // Got a calendar event alarm: create event.
      // Set next calendar alarm event.
#ifdef RTCC_DBG
      UARTprintf("A");
#endif // RTCC_DBG
      me->mDS3234Ptr->ClrAlarmFlag(DS3234::ALARM_ID::ALARM_ID_2);
      RTCCTimeDateEvt *lCalendarEvtPtr = Q_NEW(RTCCTimeDateEvt, SIG_RTCC_CALENDAR_EVENT_ALARM);
      lCalendarEvtPtr->mTime = me->mTime;
      lCalendarEvtPtr->mDate = me->mDate;
      QP::QF::PUBLISH(static_cast<QP::QEvt *>(lCalendarEvtPtr), me);

      SetNextCalendarEvt(me);
    }
    return Q_HANDLED();
  }

  case SIG_RTCC_ADD_CALENDAR_ENTRY: {
    RTCCTimeDateEvt const *lSetEvtPtr = static_cast<RTCCTimeDateEvt const *>(e);
    me->mCalendarPtr->SetEntry(lSetEvtPtr->mDate.GetWeekday(),
                               lSetEvtPtr->mTime);
    SetNextCalendarEvt(me);
    return Q_HANDLED();
  }

  case SIG_RTCC_DEL_CALENDAR_ENTRY: {
    RTCCTimeDateEvt const *lSetEvtPtr = static_cast<RTCCTimeDateEvt const *>(e);
    me->mCalendarPtr->ClrEntry(lSetEvtPtr->mDate.GetWeekday(),
                               lSetEvtPtr->mTime);
    SetNextCalendarEvt(me);
    return Q_HANDLED();
  }

  case SIG_TERMINATE:
    // Exit application gracefully.
    // Could also transition to a "Final" state.
    // BSP_Exit();
    IntDisable(me->mIntNbr);
    return Q_HANDLED();
  }

  return Q_SUPER(&QP::QHsm::top);
}


void RTCC_AO::SetNextCalendarEvt(RTCC_AO * const me) {

  Weekday lCurrentWeekday(me->mDate.GetWeekdayName());
  Time    lAlarmTime;
  Weekday lAlarmWeekday;

#ifdef RTCC_CALENDAR_DBG
  lAlarmTime = me->mTime;
  unsigned int lMinutes = me->mTime.GetMinutes() + 15;
  unsigned int lHours   = me->mTime.GetHours();
  if (lMinutes >= 60) {
    lMinutes %= 60;
    lHours++;
    if (lHours >= 24) {
      lHours = 0;
    }
  }
  lAlarmTime.SetMinutes(lMinutes);
  lAlarmTime.SetHours(lHours);
  me->mCalendarPtr->SetEntry(lCurrentWeekday, lAlarmTime);
#endif // RTCC_CALENDAR_DBG

  bool lIsNextEntry = me->mCalendarPtr->GetNextEntry(lCurrentWeekday,
                                                     me->mTime,
                                                     lAlarmWeekday,
                                                     lAlarmTime);
  lIsNextEntry = true;
  if (lIsNextEntry) {
    // Entry found: use alarm 2 for feeding alarm.
    me->mDS3234Ptr->WrAlarm(DS3234::ALARM_ID::ALARM_ID_2,
                            lAlarmTime,
                            lAlarmWeekday);
  } else {
    // No next entry found:
    // clear alarm so it does not generate an interrupt.
    me->mDS3234Ptr->DisableAlarm(DS3234::ALARM_ID::ALARM_ID_2);
  }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
