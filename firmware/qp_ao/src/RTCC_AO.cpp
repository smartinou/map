// *****************************************************************************
//
// Project: Active Object Library
//
// Module: RTCC QP Active Object.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group
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

// Standard Library.
#include <stddef.h>

// TI Library.
#include "hw_types.h"
#include "hw_ints.h"
#include "gpio.h"
#include "interrupt.h"
#include "uartstdio.h"

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
#include "BSP.h"

Q_DEFINE_THIS_FILE

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

#define DBG_CALENDAR
//#undef  DBG_CALENDAR

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
  mCalendar(),
  mIntNbr(0) {

  // Ctor body.
  mInstancePtr = this;
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

QP::QState RTCC_AO::Initial(RTCC_AO        * const me, //aMePtr,
                            QP::QEvt const * const aEvtPtr) {

  // FIXME: Determine what to return in case of failure of 'new'.
  int lResult = InitRTCC(me, aEvtPtr);

  // Subscribe to signals if any.
  //aMe->subscribe(<>_SIG);

  lResult = InitCalendar(me, aEvtPtr);
  lResult = InitInterrupt(me, aEvtPtr);

  (void)lResult;

  return Q_TRAN(&RTCC_AO::Running);
}


unsigned int RTCC_AO::InitRTCC(RTCC_AO         * const me, //aMePtr,
                               QP::QEvt  const * const aEvtPtr) {

  RTCCInitEvt const *lRTCCInitEvtPtr = static_cast<RTCCInitEvt const *>(aEvtPtr);

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


unsigned int RTCC_AO::InitCalendar(RTCC_AO         * const me, //aMePtr,
                                   QP::QEvt  const * const aEvtPtr) {

  // -Get current time & date, temperature.
  // -Read config from RTCC sram.
  // -Populate calendar.
  // -Set next alarm.
  me->mDS3234Ptr->RdTimeAndDate(me->mTime, me->mDate);
  me->mTemperature = me->mDS3234Ptr->GetTemperature();
#ifdef DBG_CALENDAR
  me->mCalendar.ClrAllEntries();
#else
  // Fill calendar from DB values:
  // Read whole DB from RTCC NV RAM.
  // Serialize Calendar record into Calendar member object.
#endif // DBG_CALENDAR
  SetNextCalendarEvt(me);

  return 0;
}


unsigned int RTCC_AO::InitInterrupt(RTCC_AO         * const me, //aMePtr,
                                    QP::QEvt  const * const aEvtPtr) {

  // Set interrupt pin and periodic alarm.
  RTCCInitEvt const *lRTCCInitEvtPtr = static_cast<RTCCInitEvt const *>(aEvtPtr);
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


QP::QState RTCC_AO::Running(RTCC_AO        * const me, //aMePtr,
                            QP::QEvt const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_ENTRY_SIG:
    IntEnable(me->mIntNbr);
    return Q_HANDLED();
    
  case SIG_RTCC_INTERRUPT: {
    // Read complete RTC, updating Time & Date at the same time.
    me->mDS3234Ptr->GetTimeAndDate(me->mTime, me->mDate);
    me->mTemperature = me->mDS3234Ptr->GetTemperature();
    me->mDS3234Ptr->ClrAlarmFlag(DS3234::ALARM_ID::ALARM_ID_1);
    UARTprintf("*");

    // Publish Tick Alarm Event.
    RTCCEvt *lTickAlarmEvtPtr = Q_NEW(RTCCEvt, SIG_RTCC_TIME_TICK_ALARM);
    lTickAlarmEvtPtr->mTime = me->mTime;
    lTickAlarmEvtPtr->mDate = me->mDate;
    QP::QF::PUBLISH(static_cast<QP::QEvt *>(lTickAlarmEvtPtr), me);
    
    if ((DS3234::AF2  & me->mDS3234Ptr->GetStatus()) &&
        (DS3234::AEI2 & me->mDS3234Ptr->GetCtrl())) {
      // Got a calendar event alarm: create event.
      // Set next calendar alarm event.
      UARTprintf("A");
      me->mDS3234Ptr->ClrAlarmFlag(DS3234::ALARM_ID::ALARM_ID_2);
      RTCCEvt *lCalendarEvtPtr = Q_NEW(RTCCEvt, SIG_RTCC_CALENDAR_EVENT_ALARM);
      lCalendarEvtPtr->mTime = me->mTime;
      lCalendarEvtPtr->mDate = me->mDate;
      QP::QF::PUBLISH(static_cast<QP::QEvt *>(lCalendarEvtPtr), me);

      SetNextCalendarEvt(me);
    }
    return Q_HANDLED();
  }

  case SIG_RTCC_ADD_CALENDAR_ENTRY: {
    RTCCSetEvt const *lSetEvtPtr = static_cast<RTCCSetEvt const *>(aEvtPtr);
    me->mCalendar.SetEntry(lSetEvtPtr->mWeekday, lSetEvtPtr->mTime);
    SetNextCalendarEvt(me);
    return Q_HANDLED();
  }

  case SIG_RTCC_DEL_CALENDAR_ENTRY: {
    RTCCSetEvt const *lSetEvtPtr = static_cast<RTCCSetEvt const *>(aEvtPtr);
    me->mCalendar.ClrEntry(lSetEvtPtr->mWeekday, lSetEvtPtr->mTime);
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

#ifdef DBG_CALENDAR
  lAlarmTime = me->mTime;
  unsigned int lMinutes = me->mTime.GetMinutes() + 2;
  unsigned int lHours   = me->mTime.GetHours();
  if (lMinutes > 59) {
    lMinutes %= 60;
    lHours++;
    if (lHours > 23) {
      lHours = 0;
    }
  }
  lAlarmTime.SetMinutes(lMinutes);
  lAlarmTime.SetHours(lHours);
  me->mCalendar.SetEntry(lCurrentWeekday, lAlarmTime);
#endif // DBG_CALENDAR

  bool lIsNextEntry = me->mCalendar.GetNextEntry(lCurrentWeekday,
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
