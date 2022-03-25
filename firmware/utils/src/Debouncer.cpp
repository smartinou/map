// *****************************************************************************
//
// Project: Utilities.
//
// Module: Debouncer QP Active Object.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

// QP Library.
#include "qpcpp.h"

// Common Library.

// This project.
#include "BSP.h"
#include "Debouncer.h"

Q_DEFINE_THIS_FILE

//namespace BFH {

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

Debouncer::Debouncer(QP::QActive * const aContainerPtr) :
  QP::QHsm(Q_STATE_CAST(&Debouncer::Initial)),
  mContainerPtr(aContainerPtr),
  mDebounceTimeEvt(aContainerPtr, SIG_DEBOUNCE_TIMEOUT, 0U) {

  // Intentionally empty constructor.
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

QP::QState Debouncer::Initial(Debouncer      * const me, //aMePtr,
			      QP::QEvt const * const aEvtPtr) {

  (void)aEvtPtr;

  return Q_TRAN(&Debouncer::Ready);
}


QP::QState Debouncer::Ready(Debouncer      * const me,
			    QP::QEvt const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_ENTRY_SIG:
    return Q_HANDLED();

  case SIG_BUTTON_EVT:
    if ((ButtonEvt::PRESSED == (static_cast<ButtonEvt const *>(aEvtPtr))->mState) &&
	(0 != (static_cast<ButtonEvt const *>(aEvtPtr))->mDebounceDelay)) {
      return Q_TRAN(&Debouncer::Pressed);
    }
    break;
  }

  return Q_SUPER(&QP::QHsm::top);
  //return Q_TRAN(&QP::QHsm::top);
}


QP::QState Debouncer::Pressed(Debouncer      * const me, //aMePtr,
			      QP::QEvt const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_ENTRY_SIG:
    // Get locally buffered delay.
    me->mDebounceTimeEvt.armX(20); //BSP_TICKS_PER_SEC * 2);
    return Q_HANDLED();

  case Q_EXIT_SIG:
    me->mDebounceTimeEvt.disarm();
    return Q_HANDLED();

  case SIG_DEBOUNCE_TIMEOUT: {
    ButtonEvt *lButtonEvtPtr = Q_NEW(ButtonEvt, SIG_BUTTON_EVT);
    lButtonEvtPtr->mState = ButtonEvt::DEBOUNCED;
    me->mContainerPtr->POST(lButtonEvtPtr, me);
    return Q_TRAN(&Debouncer::Ready);
  }

  case SIG_BUTTON_EVT:
    if (ButtonEvt::RELEASED == (static_cast<ButtonEvt const *>(aEvtPtr))->mState) {
      return Q_TRAN(&Debouncer::Ready);
    }
  }

  return Q_SUPER(&QP::QHsm::top);
  //return Q_TRAN(&QP::QHsm::top);
}


//} // namespace BFH

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
