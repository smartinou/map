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
#include "gpio.h"
#include "interrupt.h"

// QP Library.
#include "qpcpp.h"

// This project.
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
               unsigned long aIntNbr,
               unsigned int  aID) :
  mGPIOPort(aGPIOPort),
  mGPIOPin(aGPIOPin),
  mIntNbr(aIntNbr),
  mID(aID) {

  DisableInt();

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
  //IntEnable(lInt);
}


unsigned int Button::GetGPIOPinState(void) {

  unsigned long lGPIOPin = GPIOPinRead(mGPIOPort, mGPIOPin);
  unsigned int  lState = RELEASED;
  if (lGPIOPin & mGPIOPin) {
    lState = PRESSED;
  }

  return lState;
}


void Button::DisableInt(void) {
  IntDisable(mIntNbr);
}


void Button::EnableInt(void) {
  IntEnable(mIntNbr);
}


void Button::ClrInt(void) {
  GPIOPinIntClear(mGPIOPort, mGPIOPin);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

//} // namespace BFH

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
