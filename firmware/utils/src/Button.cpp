// *****************************************************************************
//
// Project: Utilities.
//
// Module: Button class.
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

// TI Library.
#include "hw_types.h"
#include "gpio.h"
#include "interrupt.h"

// This project.
#include "GPIOs.h"
#include "Button.h"

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

Button::Button(unsigned long const aGPIOPort,
               unsigned int  const aGPIOPin,
               unsigned long const aIntNbr,
               unsigned int  const aID)
  : GPIOs(aGPIOPort, aGPIOPin)
  , mIntNbr(aIntNbr)
  , mID(aID) {

  DisableInt();

  // Set specified GPIO as edge triggered input.
  // Don't enable interrupt just yet.
  GPIOPinTypeGPIOInput(mPort, mPin);
  GPIOIntTypeSet(mPort, mPin, GPIO_BOTH_EDGES);
  GPIOPadConfigSet(mPort,
                   mPin,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD_WPU);

  // Enable the interrupt of the selected GPIO.
  // Don't enable the interrupt globally yet.
  GPIOPinIntEnable(mPort, mPin);
  GPIOPinIntClear(mPort, mPin);
}


Button::Button(GPIOs         const &aGPIO,
               unsigned long const aIntNbr,
               unsigned int  const aID)
  : Button(aGPIO.GetPort(),
           aGPIO.GetPin(),
           aIntNbr,
           aID) {
  // Ctor body left intentionally empty.
}


unsigned int Button::GetGPIOPinState(void) const {

  unsigned long lGPIOPin = GPIOPinRead(mPort, mPin);
  unsigned int  lState = RELEASED;
  if (lGPIOPin & mPin) {
    lState = PRESSED;
  }

  return lState;
}


void Button::DisableInt(void) const {
  IntDisable(mIntNbr);
}


void Button::EnableInt(void) const {
  IntEnable(mIntNbr);
}


void Button::ClrInt(void) const {
  GPIOPinIntClear(mPort, mPin);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
