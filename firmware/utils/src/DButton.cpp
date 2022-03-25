// *****************************************************************************
//
// Project: Utilities.
//
// Module: Debounced button class.
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
#include <algorithm>

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
#include "DButton.h"

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

// Time(in ms) to register button as pressed or released.
unsigned int DButton::sRegisteringPressedDflt  = 10;
unsigned int DButton::sRegisteringReleasedDflt = 50;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

DButton::DButton(QP::QActive * const aContainerPtr,
		 unsigned long       aGPIOPort,
		 unsigned int        aGPIOPin) :
  QP::QHsm(Q_STATE_CAST(&DButton::Initial)),
  mContainerPtr(aContainerPtr),
  mDebounceTimeEvt(aContainerPtr, SIG_DEBOUNCE_TIMEOUT, 0U),
  mGPIOPort(aGPIOPort),
  mGPIOPin(aGPIOPin) {

  unsigned long lInt = BSPGPIOPortToInt(mGPIOPort);
  IntDisable(lInt);

  // Set specified GPIO as edge triggered input.
  // Don't enable interrupt just yet.
  GPIOPinTypeGPIOInput(mGPIOPort, mGPIOPin);
  GPIOIntTypeSet(mGPIOPort, mGPIOPin, GPIO_BOTH_EDGES);
  GPIOPadConfigSet(mGPIOPort,
		   mGPIOPin,
		   GPIO_STRENGTH_2MA,
		   GPIO_PIN_TYPE_STD_WPU);

  // Enable the interrupt of the selected GPIO.
  // [MG] COULD DELAY THE ENABLING OF THE INTERRUPT AT THE INITIAL STATE TRANSITION.
  GPIOPinIntEnable(mGPIOPort, mGPIOPin);
  GPIOPinIntClear(mGPIOPort, mGPIOPin);
  IntEnable(lInt);
}


unsigned int DButton::GetGPIOPinState(void) {

  // Input is grounded when button is pressed.
  unsigned long lGPIOPin = GPIOPinRead(mGPIOPort, mGPIOPin);
  unsigned int  lState = RELEASED;
  if (0 == (lGPIOPin & mGPIOPin)) {
    lState = PRESSED;
  }

  return lState;
}


void DButton::GenerateEvt(void) {

  // Determine state of the button.
  // Generate a new ButtonEvt event.
  // Publish event to the framework.
  //ButtonEvt *lButtonEvtPtr = Q_NEW(ButtonEvt, SIG_BUTTON_EVT);
  //lButtonEvtPtr->mState = GetGPIOPinState();

  //QP::QF::PUBLISH(lButtonEvtPtr, 0);
  // [MG] OR POST TO CONTAINER.
}


void DButton::GenerateEvt(QP::QActive &aAORef) {

  // Determine state of the button.
  // Generate a new ButtonEvt event.
  // Publish event to the specified Active Object.
  ButtonEvt *lButtonEvtPtr = Q_NEW(ButtonEvt, SIG_BUTTON_EVT);
  lButtonEvtPtr->mState = GetGPIOPinState();

  aAORef.POST(lButtonEvtPtr, 0);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

QP::QState DButton::Initial(DButton        * const me, //aMePtr,
			    QP::QEvt const * const aEvtPtr) {

  (void)aEvtPtr;

  return Q_TRAN(&DButton::Idle);
}


QP::QState DButton::Idle(DButton        * const me,
			 QP::QEvt const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_ENTRY_SIG:
    return Q_HANDLED();

  case SIG_BUTTON_EVT:
    if (ButtonEvt::PRESSED == (static_cast<ButtonEvt const *>(aEvtPtr))->mState) {
      unsigned int lDebounceDelay = (static_cast<ButtonEvt const *>(aEvtPtr))->mDebounceDelay;
      if (0 != lDebounceDelay) {
	me->mRegisteringReleased = lDebounceDelay;
	return Q_TRAN(&DButton::RegisteringPressed);
      } else {
	// No debounce time specified:
	// [MG] Check if the state can be modified and signal reposted to container.
	// [MG] ::DEBOUNCED SHOULD NOT BE INTERCEPTED HERE OR IT WILL BE REPOSTED FOREVER.
	//(static_cast<ButtonEvt const *>(aEvtPtr))->mState = ButtonEvt::DEBOUNCED;
	//me->mContainerPtr->POST(lButtonEvtPtr, me);
	return Q_HANDLED();
      }
    }
    break;
  }

  return Q_SUPER(&QP::QHsm::top);
}


QP::QState DButton::RegisteringPressed(DButton        * const me, //aMePtr,
				       QP::QEvt const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_ENTRY_SIG:
    // Get locally buffered delay.
    // [MG] SHOULD CONVERT TO TICKS.
    me->mDebounceTimeEvt.armX(sRegisteringPressedDflt);
    return Q_HANDLED();

  case SIG_DEBOUNCE_TIMEOUT: {
    unsigned int lState = me->GetGPIOPinState();
    if (PRESSED == lState) {
      return Q_TRAN(&DButton::RegisteringReleased);
    } else {
      return Q_TRAN(&DButton::Idle);
    }
  }
  }

  return Q_SUPER(&QP::QHsm::top);
}


QP::QState DButton::RegisteringReleased(DButton        * const me, //aMePtr,
					QP::QEvt const * const aEvtPtr) {

  switch (aEvtPtr->sig) {
  case Q_ENTRY_SIG: {
    // Get locally buffered delay.
    // [MG] SHOULD CONVERT TO TICKS.
    unsigned int lMinTime = std::min(me->mRegisteringReleased, sRegisteringReleasedDflt);
    me->mDebounceTimeEvt.armX(lMinTime);
    me->mRegisteringReleased -= lMinTime;
    return Q_HANDLED();
  }

  case Q_EXIT_SIG:
    me->mDebounceTimeEvt.disarm();
    return Q_HANDLED();

  case SIG_DEBOUNCE_TIMEOUT: {
    if (0 == me->mRegisteringReleased) {
      // Time to register as released has fully elapsed.
      // Check state of input pin if still pressed.
      unsigned int lState = me->GetGPIOPinState();
      if (PRESSED == lState) {
	ButtonEvt *lButtonEvtPtr = Q_NEW(ButtonEvt, SIG_BUTTON_EVT);
	lButtonEvtPtr->mState = ButtonEvt::DEBOUNCED;
	me->mContainerPtr->POST(lButtonEvtPtr, me);
      }
      return Q_TRAN(&DButton::Idle);
    } else {
      // Time not fully elapsed: trigger new timer event.
      unsigned int lMinTime = std::min(me->mRegisteringReleased, sRegisteringReleasedDflt);
      me->mDebounceTimeEvt.armX(lMinTime);
      me->mRegisteringReleased -= lMinTime;
    }
    return Q_HANDLED();
  }

  case SIG_BUTTON_EVT:
    if (ButtonEvt::PRESSED == (static_cast<ButtonEvt const *>(aEvtPtr))->mState) {
      // The button was pressed while waiting for registration time.
      // Cancel current release registration time and start over.
      return Q_TRAN(&DButton::RegisteringPressed);
    }
  }

  return Q_SUPER(&QP::QHsm::top);
}

//} // namespace BFH

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
