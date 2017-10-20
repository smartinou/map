// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Button class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

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

// QP Library.
#include "qpcpp.h"

// Common Library.

// This project.
#include "BSP.h"
#include "Button.h"

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

Button::Button(unsigned long aGPIOPort,
	       unsigned int  aGPIOPin,
	       unsigned int  aID) :
  mGPIOPort(aGPIOPort),
  mGPIOPin(aGPIOPin),
  mID(aID) {

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
  GPIOPinIntEnable(mGPIOPort, mGPIOPin);
  GPIOPinIntClear(mGPIOPort, mGPIOPin);
  IntEnable(lInt);
}


unsigned int Button::GetGPIOPinState(void) {

  unsigned long lGPIOPin = GPIOPinRead(mGPIOPort, mGPIOPin);
  unsigned int  lState = RELEASED;
  if (lGPIOPin & mGPIOPin) {
    lState = PRESSED;
  }

  return lState;
}


void Button::GenerateEvt(void) {

  // Determine state of the button.
  // Generate a new ButtonEvt event.
  // Publish event to the framework.
  //ButtonEvt *lButtonEvtPtr = Q_NEW(ButtonEvt, SIG_BUTTON_EVT);
  //lButtonEvtPtr->mState = GetGPIOPinState();

  //QP::QF::PUBLISH(lButtonEvtPtr, 0);
}


void Button::GenerateEvt(QP::QActive &aAORef) {

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

//} // namespace BFH

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
