// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Display manager QP Active Object.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <stddef.h>

// TI Library.
#include "hw_memmap.h"
#include "hw_types.h"
#include "gpio.h"

// QP Library.
#include "qpcpp.h"

// Drivers Library.
#include "SSD1329.h"

// This project.
#include "BSP.h"
#include "DisplayMgr_AO.h"
#include "DisplayMgr_Evt.h"
#include "Signals.h"

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

DisplayMgr_AO *DisplayMgr_AO::mInstancePtr = nullptr;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

DisplayMgr_AO &DisplayMgr_AO::Instance(void) {
  return *mInstancePtr;
}


QP::QActive &DisplayMgr_AO::AOInstance(void) {
  return static_cast<QP::QActive &>(*mInstancePtr);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

DisplayMgr_AO::DisplayMgr_AO(SSD1329 &aSSD1329Ptr)
  : QActive(Q_STATE_CAST(&DisplayMgr_AO::Initial))
  , mDisplayTimerEvt(this, SIG_DISPLAY_TIMEOUT, 0U)
  , mDisplay(aSSD1329Ptr)
  , mIsDisplayOn(false)
  , mDisplayTime(0) {
  // Ctor body.
  mInstancePtr = this;
}


QP::QState DisplayMgr_AO::Initial(DisplayMgr_AO  * const me,  //aMePtr,
                                  QP::QEvt const * const e) { //aEvtPtr

  DisplayMgrInitEvt const * const lInitEvtPtr = static_cast<DisplayMgrInitEvt const * const>(e);

  me->mDisplayTime = lInitEvtPtr->mDisplayTime;

  // Subscribe to signals if any.
  me->subscribe(SIG_DISPLAY_TEXT);

  // Object dictionary for DisplayMgr_AO object.
  static DisplayMgr_AO const * const sDisplayMgrAOPtr = reinterpret_cast<DisplayMgr_AO const * const>(me);
  QS_OBJ_DICTIONARY(sDisplayMgrAOPtr);
  QS_OBJ_DICTIONARY(&sDisplayMgrAOPtr->mDisplayTimerEvt);

  // Function dictionaries for DisplayMgr_AO state handlers.
  QS_FUN_DICTIONARY(&DisplayMgr_AO::Running);

  // Locally consumed signals.
  QS_SIG_DICTIONARY(SIG_DISPLAY_TEXT,    sDisplayMgrAOPtr);
  QS_SIG_DICTIONARY(SIG_DISPLAY_TIMEOUT, sDisplayMgrAOPtr);
  QS_SIG_DICTIONARY(SIG_DISPLAY_REFRESH, sDisplayMgrAOPtr);

  return Q_TRAN(&DisplayMgr_AO::Running);
}


QP::QState DisplayMgr_AO::Running(DisplayMgr_AO  * const me,  //aMePtr,
                                  QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->DisplayInit(me);
    // Intentional fallthrough.
  case Q_INIT_SIG:
    return Q_HANDLED();

  case SIG_DISPLAY_TEXT: {
    DisplayText(me, e);
    return Q_HANDLED();
  }
  case SIG_DISPLAY_TIMEOUT:
    DisplayOff(me);
    return Q_HANDLED();

  case SIG_DISPLAY_REFRESH:
    DisplayOn(me);
    return Q_HANDLED();

  case Q_EXIT_SIG:
    me->mDisplayTimerEvt.disarm();
    DisplayOff(me);
  case TERMINATE_SIG:
    return Q_HANDLED();
  }

  return Q_SUPER(&QP::QHsm::top);
}


void DisplayMgr_AO::DisplayInit(DisplayMgr_AO * const me) {
  me->mDisplay.Init();
  me->mIsDisplayOn = false;
}


void DisplayMgr_AO::DisplayText(DisplayMgr_AO  * const me,
                                QP::QEvt const * const e) {

  DisplayTextEvt const * const lTextEvtPtr = static_cast<DisplayTextEvt const * const>(e);
  me->DisplayOn(me);
  me->mDisplay.DrawStr(&lTextEvtPtr->mStr[0],
                           lTextEvtPtr->mPosX,
                           lTextEvtPtr->mPosY,
                           lTextEvtPtr->mGreyLvl);
}


void DisplayMgr_AO::DisplayOn(DisplayMgr_AO * const me) {

  me->mDisplayTimerEvt.rearm(me->mDisplayTime * BSP::TICKS_PER_SEC);
  if (!me->mIsDisplayOn) {
    me->mIsDisplayOn = true;
    me->mDisplay.DisplayOn();
  }
}


void DisplayMgr_AO::DisplayOff(DisplayMgr_AO * const me) {
  if (me->mIsDisplayOn) {
    me->mIsDisplayOn = false;
    me->mDisplay.DisplayOff();
  }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
