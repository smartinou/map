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
//! It manages the feeding signals comming from various sources like:
//! RTC alarm, button, web cgi or CLI.
//!
//! The AO uses two different design pattern.
//! 1. Deferred events:
//!   The choice was made to defer feeding signal is the source is from the
//!   RTC alarm (because it's part of the feeding calendar).
//!   Any other feeding signal incoming while in the feeding substate is ignored.

// *****************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************
#define BFH_MGR_DBG
//#undef BFH_MGR_DBG

// Standard Library.
#include <stddef.h>

// TI Library.

// QP Library.
#include "qpcpp.h"

// Common Library.
#include "Date.h"
#include "DBRec.h"
#include "FeedCfgRec.h"
#include "Logger.h"
#include "Time.h"

// This project.
#include "BFHMgr_AO.h"
#include "BFHMgr_Evt.h"
#include "BSP.h"
#include "RTCC_Evt.h"
#include "TB6612.h"

Q_DEFINE_THIS_FILE

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

BFHMgr_AO *BFHMgr_AO::mInstancePtr = static_cast<BFHMgr_AO *>(0);

static char const sLogCategory[] = "Feeder manager";

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

BFHMgr_AO &BFHMgr_AO::Instance(void) {

  if (static_cast<BFHMgr_AO *>(0) == mInstancePtr) {
    mInstancePtr = new BFHMgr_AO();
  }

  return *mInstancePtr;
}


QP::QActive &BFHMgr_AO::AOInstance(void) {

  if (static_cast<BFHMgr_AO *>(0) == mInstancePtr) {
    mInstancePtr = new BFHMgr_AO();
  }

  return static_cast<QP::QActive &>(*mInstancePtr);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

BFHMgr_AO::BFHMgr_AO() :
  QActive(Q_STATE_CAST(&BFHMgr_AO::Initial)),
  mFeedEvtQueue(),
  mFeedEvtQueueSto{nullptr},
  mFeedTimerEvt(this, SIG_FEED_MGR_TIMEOUT, 0U),
  mFeedCfgRecPtr(nullptr),
  mFeedTime(0),
  mMotorCtrlPtr(nullptr) {

  // Ctor body intentionally left empty.
}


QP::QState BFHMgr_AO::Initial(BFHMgr_AO      * const me,  //aMePtr,
                              QP::QEvt const * const e) { //aEvtPtr

  BFHInitEvt const * const lBFHInitEvtPtr = static_cast<BFHInitEvt const * const>(e);
  me->mFeedCfgRecPtr = lBFHInitEvtPtr->mFeedCfgRecPtr;

  // Store reference to a feeder unit, or a motor controller.
  me->mMotorCtrlPtr = new TB6612(*lBFHInitEvtPtr->mMotorCtrlIn1Ptr,
                                 *lBFHInitEvtPtr->mMotorCtrlIn2Ptr,
                                 *lBFHInitEvtPtr->mMotorCtrlPWMPtr);
  me->StopFeeding();

  // Initialize the QF queue for deferred feed requests.
  me->mFeedEvtQueue.init(me->mFeedEvtQueueSto, Q_DIM(me->mFeedEvtQueueSto));

  // Subscribe to signals if any.
  me->subscribe(SIG_RTCC_CALENDAR_EVENT_ALARM);

  // Set logging category.
  LOGGER.AddCategory(SIG_FEED_MGR_LOG, &sLogCategory[0]);

  // Object dictionary for BFHMgr_AO object.
  static BFHMgr_AO const * const sBFHMgrAOPtr = reinterpret_cast<BFHMgr_AO const * const>(me);
  QS_OBJ_DICTIONARY(sBFHMgrAOPtr);
  QS_OBJ_DICTIONARY(&sBFHMgrAOPtr->mFeedTimerEvt);
  QS_OBJ_DICTIONARY(&sBFHMgrAOPtr->mFeedEvtQueueSto[0]);

  // Function dictionaries for BFHMgr_AO state handlers.
  QS_FUN_DICTIONARY(&BFHMgr_AO::Initial);
  QS_FUN_DICTIONARY(&BFHMgr_AO::FeedingMgr);
  QS_FUN_DICTIONARY(&BFHMgr_AO::Waiting);
  QS_FUN_DICTIONARY(&BFHMgr_AO::TimedFeed);
  QS_FUN_DICTIONARY(&BFHMgr_AO::ManualFeed);
  QS_FUN_DICTIONARY(&BFHMgr_AO::WaitPeriod);
  QS_FUN_DICTIONARY(&BFHMgr_AO::TimeCappedFeed);

  // Locally consumed signals.
  QS_SIG_DICTIONARY(SIG_FEED_MGR_MANUAL_FEED_CMD,  sBFHMgrAOPtr);
  QS_SIG_DICTIONARY(SIG_FEED_MGR_TIMEOUT,          sBFHMgrAOPtr);
  QS_SIG_DICTIONARY(SIG_RTCC_CALENDAR_EVENT_ALARM, sBFHMgrAOPtr);
  QS_SIG_DICTIONARY(SIG_FEED_MGR_TIMED_FEED_CMD,   sBFHMgrAOPtr);

  // Published signals.

  return Q_TRAN(&BFHMgr_AO::FeedingMgr);
}


QP::QState BFHMgr_AO::FeedingMgr(BFHMgr_AO      * const me,  //aMePtr,
                                 QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    // Init Feedeer.
    return Q_HANDLED();

  case SIG_FEED_MGR_MANUAL_FEED_CMD: {
    // Cast event to know the state (on/off).
    BFHManualFeedCmdEvt const * const lEvtPtr = static_cast<BFHManualFeedCmdEvt const *>(e);
    if (lEvtPtr->mIsOn) {
      LOG_INFO(&sLogCategory[0], "Manual feed event (on).");
      return Q_TRAN(&BFHMgr_AO::ManualFeed);
    }
    // Off: intentional fallthrough.
    LOG_INFO(&sLogCategory[0], "Manual feed event (off).");
  }

  case Q_INIT_SIG:
    // Go into default nested state.
  case SIG_FEED_MGR_TIMEOUT:
    return Q_TRAN(&BFHMgr_AO::Waiting);

  case SIG_RTCC_CALENDAR_EVENT_ALARM:
    //RTCCEvt const *lEvtPtr = static_cast<RTCCEvt const *>(e);
    LOG_INFO(&sLogCategory[0], "Timed feed event from calendar entry.");
    me->mFeedTime = me->mFeedCfgRecPtr->GetTimedFeedPeriod();
    return Q_TRAN(&BFHMgr_AO::TimedFeed);

  case SIG_FEED_MGR_TIMED_FEED_CMD: {
    // FIXME: perform boundary check on value.
    BFHTimedFeedCmdEvt const * const lEvtPtr = reinterpret_cast<BFHTimedFeedCmdEvt const * const>(e);
    if (0 != lEvtPtr->mTime) {
      me->mFeedTime = lEvtPtr->mTime;
    } else {
      me->mFeedTime = me->mFeedCfgRecPtr->GetTimedFeedPeriod();
    }
    LOG_INFO(&sLogCategory[0], "Timed feed event from other source.");
    return Q_TRAN(&BFHMgr_AO::TimedFeed);
  }

  case Q_EXIT_SIG:
    return Q_HANDLED();

  case SIG_TERMINATE:
    // Exit application gracefully.
    // Could also transition to a "Final" state.
    // BSP_Exit();
    return Q_HANDLED();
  }

  return Q_SUPER(&QP::QHsm::top);
}


QP::QState BFHMgr_AO::Waiting(BFHMgr_AO      * const me,  //aMePtr,
                              QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->recall(&me->mFeedEvtQueue);
    return Q_HANDLED();
  }

  return Q_SUPER(&BFHMgr_AO::FeedingMgr);
}


QP::QState BFHMgr_AO::TimedFeed(BFHMgr_AO      * const me,  //aMePtr,
                                QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->mFeedTimerEvt.armX(me->mFeedTime * BSP_TICKS_PER_SEC);
    me->StartFeeding();
    return Q_HANDLED();

  case SIG_RTCC_CALENDAR_EVENT_ALARM:
    // Cast event, log(Time, Date);
    me->defer(&me->mFeedEvtQueue, e);
    return Q_HANDLED();

  case SIG_FEED_MGR_MANUAL_FEED_CMD:
    // DoNothing();
    return Q_HANDLED();

  case Q_EXIT_SIG:
    me->StopFeeding();
    me->mFeedTimerEvt.disarm();
    return Q_HANDLED();
  }

  return Q_SUPER(&BFHMgr_AO::FeedingMgr);
}


QP::QState BFHMgr_AO::ManualFeed(BFHMgr_AO      * const me,  //aMePtr,
                                 QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    return Q_HANDLED();

  case Q_INIT_SIG:
    // Go into default nested state.
    return Q_TRAN(&BFHMgr_AO::WaitPeriod);

  case SIG_RTCC_CALENDAR_EVENT_ALARM:
    // Cast event, log(Time, Date);
    me->defer(&me->mFeedEvtQueue, e);
    return Q_HANDLED();

  case SIG_FEED_MGR_MANUAL_FEED_CMD: {
    // Cast event to know the state (on/off).
    BFHManualFeedCmdEvt const * const lEvtPtr = static_cast<BFHManualFeedCmdEvt const * const>(e);
    if (lEvtPtr->mIsOn) {
      // On: DoNothing();
      return Q_HANDLED();
    }
    break;
  }

  case Q_EXIT_SIG:
    me->StopFeeding();
    me->mFeedTimerEvt.disarm();
    return Q_HANDLED();
  }

  return Q_SUPER(&BFHMgr_AO::FeedingMgr);
}


QP::QState BFHMgr_AO::WaitPeriod(BFHMgr_AO      * const me,  //aMePtr,
                                 QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->mFeedTimerEvt.armX(TIME_CAPPED_DEBOUNCE * BSP_TICKS_PER_SEC);
    return Q_HANDLED();

  case SIG_FEED_MGR_TIMEOUT:
    return Q_TRAN(&BFHMgr_AO::TimeCappedFeed);

  case Q_EXIT_SIG:
    return Q_HANDLED();
  }

  return Q_SUPER(&BFHMgr_AO::ManualFeed);
}


QP::QState BFHMgr_AO::TimeCappedFeed(BFHMgr_AO      * const me,  //aMePtr,
                                     QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->mFeedTimerEvt.armX(TIME_CAPPED_TIMEOUT * BSP_TICKS_PER_SEC);
    me->StartFeeding();
    return Q_HANDLED();

  case Q_EXIT_SIG:
    return Q_HANDLED();
  }

  return Q_SUPER(&BFHMgr_AO::ManualFeed);
}


void BFHMgr_AO::StartFeeding(void) const {
  // Turn on feeding mechanism.
  mMotorCtrlPtr->TurnOnCW();
}


void BFHMgr_AO::StopFeeding(void) const {
  // Turn off feeding mechanism.
  mMotorCtrlPtr->TurnOff();
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
