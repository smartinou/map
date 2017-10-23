// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Beast feeder manager QP Active Object.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group
//!
//! This is the main AO of the system.
//! It has control over the RTC and managed the feeding signals comming from
//! various sources like, RTC alarm, button, web cgi or CLI.
//!
//! The AO uses two different design pattern.
//! 1. Deferred events:
//!   The choice was made to defer feeding signal is the source is from the
//!   RTC alarm (because it's part of the feeding calendar).
//!   Any other feeding signal incoming while in the feeding substate is ignored.
//!
//! 2. Othogonal region.
//!   The AO uses composition to incorporate a button used for feeding.
//!   Signals directed to the feeding button are dispatched to the instance.
//!   Generated signals from the feeding button instance are sent to the 
//!   container object.

// *****************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
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
#include "hw_memmap.h"
#include "gpio.h"
#include "interrupt.h"
#include "uartstdio.h"

// QP Library.
#include "qpcpp.h"

// Common Library.
#include "Button.h"
#include "Date.h"
#include "DBRec.h"
#include "DS3234.h"
#include "SPI.h"
#include "SSD1329.h"
#include "Time.h"

// This project.
#include "BeastFeedHerMgr.h"
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

extern QP::QActive     *gMain_BeastFeedHerMgrAOPtr;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

BeastFeedHerMgr::BeastFeedHerMgr(CoreLink::SPIDev &aSPIDevRef) :
  QActive(Q_STATE_CAST(&BeastFeedHerMgr::Initial)),
  mTime(),
  mDate(),
  mTemperature(0.0),
  mDS3234Ptr(static_cast<DS3234 *>(0)),
  mRTCSPISlaveCfgPtr(static_cast<CoreLink::SPISlaveCfg *>(0)),
  mFeedDelayEvt(this, SIG_FEED_DELAY_TIMEOUT, 0U),
  mDebouncer(this) {

  // Create an SPI slave configuration for the DS3234 RTC.
  mRTCSPISlaveCfgPtr = new CoreLink::SPISlaveCfg();
  mRTCSPISlaveCfgPtr->SetProtocol(CoreLink::SPISlaveCfg::MOTO_1);
  mRTCSPISlaveCfgPtr->SetBitRate(4000000);
  mRTCSPISlaveCfgPtr->SetDataWidth(8);
  mRTCSPISlaveCfgPtr->SetCSnGPIO(GPIO_PORTA_BASE, GPIO_PIN_7);

  // Create & initialize the DS3234 RTC.
  // Configure the RTC to desired state:
  //   -Oscillator enabled.
  //   -Battery-backed square wave disabled.
  //   (-Rate select: 1KHz square wave.)
  //   -Interrupt Control enabled.
  //   -Alarm 1 enabled.
  //     -1Hz interrupt rate.
  mDS3234Ptr = new DS3234(aSPIDevRef, *mRTCSPISlaveCfgPtr);
  mDS3234Ptr->Init(DS3234::Ctrl::INTCn);

  // Initialize the QF queue for deferred feed requests.
  mFeedEvtQueue.init(mFeedEvtQueueSto, Q_DIM(mFeedEvtQueueSto));
}


void BeastFeedHerMgr::ISRCallback(void) {

  // Static event.
  static QP::QEvt const sRTCSQWIntEvt = { SIG_RTC_IRQ, 0U, 0U };
  mDS3234Ptr->ISRCallback();

  // Signal that RTC generated an interrupt.
  POST(&sRTCSQWIntEvt, 0);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

QP::QState BeastFeedHerMgr::Initial(BeastFeedHerMgr * const me, //aMePtr,
				    QP::QEvt  const * const aEvtPtr) {

  // Subscribe to signals if any.
  //aMe->subscribe(<>_SIG);

  // -Get current time & date, temperature.
  // -Read config from RTC sram.
  // -Populate calendar.
  // -Set next alarm.
  me->mDS3234Ptr->RdTimeAndDate(me->mTime, me->mDate);
  me->mTemperature = me->mDS3234Ptr->GetTemperature();
#ifdef DBG_CALENDAR
  me->mCalendar.ClrAllEntries();
#endif // DBG_CALENDAR
  SetNextFeedingTime(me);

  // Trigger initial transition of HSM object.
  me->mDebouncer.init();

  // Enable the external interrupt for the RTC.
  GPIOInitEvt const *lGPIOInitEvtPtr = static_cast<GPIOInitEvt const *>(aEvtPtr);
  unsigned long lInt = BSPGPIOPortToInt(lGPIOInitEvtPtr->mGPIOPort);
  IntDisable(lInt);

  GPIOPinTypeGPIOInput(lGPIOInitEvtPtr->mGPIOPort,
		       lGPIOInitEvtPtr->mGPIOPin);
  GPIOIntTypeSet(lGPIOInitEvtPtr->mGPIOPort,
		 lGPIOInitEvtPtr->mGPIOPin,
		 GPIO_FALLING_EDGE);
  GPIOPadConfigSet(lGPIOInitEvtPtr->mGPIOPort,
		   lGPIOInitEvtPtr->mGPIOPin,
		   GPIO_STRENGTH_2MA,
		   GPIO_PIN_TYPE_STD);

  //me->mDS3234Ptr->Init(DS3234::Ctrl::INTCn);
  me->mDS3234Ptr->WrAlarm(DS3234::ALARM_ID::ALARM_ID_1,
			  me->mTime,
			  me->mDate,
			  DS3234::ALARM_MODE::ONCE_PER_SEC);

  GPIOPinIntEnable(lGPIOInitEvtPtr->mGPIOPort, lGPIOInitEvtPtr->mGPIOPin);
  // [MG] THIS CLEARS THE 1ST INTERRUPT. IT DOESN'T COME UP UNTIL FLAGS ARE CLEARED.
  //GPIOPinIntClear(lGPIOInitEvtPtr->mGPIOPort, lGPIOInitEvtPtr->mGPIOPin);
  IntEnable(lInt);

  return Q_TRAN(&BeastFeedHerMgr::FeedingMgr);
}


QP::QState BeastFeedHerMgr::FeedingMgr(BeastFeedHerMgr * const me, //aMePtr,
				       QP::QEvt  const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_INIT_SIG:
    // Go into default nested state.
    return Q_TRAN(&BeastFeedHerMgr::Waiting);

  case SIG_RTC_IRQ:
    // Read complete RTC, updating Time & Date at the same time.
    me->mDS3234Ptr->GetTimeAndDate(me->mTime, me->mDate);
    me->mTemperature = me->mDS3234Ptr->GetTemperature();
    me->mDS3234Ptr->ClrAlarmFlag(DS3234::ALARM_ID::ALARM_ID_1);
    UARTprintf("*");

    if ((DS3234::AF2  & me->mDS3234Ptr->GetStatus()) &&
	(DS3234::AEI2 & me->mDS3234Ptr->GetCtrl())) {
      // Got a feed alarm: create feed command event.
      // Set the source to "RTC Interrupt".
      // Set next feeding alarm from calendar.
      UARTprintf("A");
      me->mDS3234Ptr->ClrAlarmFlag(DS3234::ALARM_ID::ALARM_ID_2);
      FeedCmdEvt *lFeedCmdEvtPtr = Q_NEW(FeedCmdEvt, SIG_FEED_CMD);
      lFeedCmdEvtPtr->mSrc = FeedCmdEvt::RTC_INT;
      gMain_BeastFeedHerMgrAOPtr->POST(lFeedCmdEvtPtr, me);
      me->POST(lFeedCmdEvtPtr, me);
      SetNextFeedingTime(me);
    }
    return Q_HANDLED();

  case SIG_BUTTON_EVT:
    if (ButtonEvt::DEBOUNCED == (static_cast<ButtonEvt const *>(aEvtPtr))->mState) {
      FeedCmdEvt *lFeedCmdEvtPtr = Q_NEW(FeedCmdEvt, SIG_FEED_CMD);
      lFeedCmdEvtPtr->mSrc = FeedCmdEvt::BUTTON;
      gMain_BeastFeedHerMgrAOPtr->POST(lFeedCmdEvtPtr, me);
      me->POST(lFeedCmdEvtPtr, me);
      return Q_HANDLED();
    }
    // Intentional fallthrough.
  case SIG_DEBOUNCE_TIMEOUT:
    me->mDebouncer.dispatch(aEvtPtr);
    return Q_HANDLED();

  case SIG_FEED_DELAY_TIMEOUT:
    return Q_TRAN(&BeastFeedHerMgr::Waiting);

  case SIG_FEED_CMD:
    // Log(aEvtPtr);
    return Q_TRAN(&BeastFeedHerMgr::FeedingTheBeast);

  case SIG_TERMINATE:
    // Exit application gracefully.
    // Could also transition to a "Final" state.
    // BSP_Exit();
    return Q_HANDLED();
  }

  return Q_SUPER(&QP::QHsm::top);
}


QP::QState BeastFeedHerMgr::Waiting(BeastFeedHerMgr * const me, //aMePtr,
				    QP::QEvt  const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_ENTRY_SIG:
    me->recall(&me->mFeedEvtQueue);
    return Q_HANDLED();
  }

  return Q_SUPER(&BeastFeedHerMgr::FeedingMgr);
}


QP::QState BeastFeedHerMgr::FeedingTheBeast(BeastFeedHerMgr * const me, //aMePtr,
					    QP::QEvt  const * const e) {//aEvtPtr) {

  switch (e->sig) {//aEvtPtr->sig) {
  case Q_ENTRY_SIG:
    me->mFeedDelayEvt.armX(30);
    me->StartFeeding();
    return Q_HANDLED();

  case Q_EXIT_SIG:
    me->StopFeeding();
    me->mFeedDelayEvt.disarm();
    return Q_HANDLED();

  case SIG_FEED_CMD:
    // If source is RTC interrupt (alarm) , defer to queue.
    // In all other cases, we dismiss the event.
    //if (FeedCmdEvt::RTC_INT == Q_EVT_CAST(FeedCmdEvt)->mSrc) {
    if (FeedCmdEvt::RTC_INT == (static_cast<FeedCmdEvt const *>(e))->mSrc) {
      me->defer(&me->mFeedEvtQueue, e); //aEvtPtr);
    }
    return Q_HANDLED();
  }

  return Q_SUPER(&BeastFeedHerMgr::FeedingMgr);
}


void BeastFeedHerMgr::StartFeeding(void) const {
  // Turn on feeding mechanism.
}


void BeastFeedHerMgr::StopFeeding(void) const {
  // Turn off feeding mechanism.
}


void BeastFeedHerMgr::SetNextFeedingTime(BeastFeedHerMgr * const aMePtr) {

  Weekday lCurrentWeekday(aMePtr->mDate.GetWeekdayName());
  Time    lAlarmTime;
  Weekday lAlarmWeekday;
#ifdef DBG_CALENDAR
  lAlarmTime = aMePtr->mTime;
  unsigned int lMinutes = aMePtr->mTime.GetMinutes() + 2;
  unsigned int lHours = aMePtr->mTime.GetHours();
  if (lMinutes > 59) {
    lMinutes %= 60;
    lHours++;
    if (lHours > 23) {
      lHours = 0;
    }
  }
  lAlarmTime.SetMinutes(lMinutes);
  lAlarmTime.SetHours(lHours);
  aMePtr->mCalendar.SetEntry(lCurrentWeekday, lAlarmTime);
#endif // DBG_CALENDAR
  bool    lIsNextEntry = aMePtr->mCalendar.GetNextEntry(lCurrentWeekday,
							aMePtr->mTime,
							lAlarmWeekday,
							lAlarmTime);
  lIsNextEntry = true;
  if (lIsNextEntry) {
    // Entry found: use alarm 2 for feeding alarm.
    aMePtr->mDS3234Ptr->WrAlarm(DS3234::ALARM_ID::ALARM_ID_2,
				lAlarmTime,
				lAlarmWeekday);
  } else {
    // No next entry found:
    // clear alarm so it does not generate an interrupt.
    aMePtr->mDS3234Ptr->DisableAlarm(DS3234::ALARM_ID::ALARM_ID_2);
  }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
