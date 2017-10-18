// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Motor controller class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2016, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.

// TI Library.
// TI Library.
#include "hw_types.h"
//#include "hw_ints.h"
#include "hw_memmap.h"
#include "gpio.h"
#include "pwm.h"
#include "sysctl.h"

// QP Library.

// Common Library.

// This project.
#include "TB6612.h"

//Q_DEFINE_THIS_FILE

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

TB6612::TB6612(unsigned long aIn1GPIOPort,
	       unsigned int  aIn1GPIOPin,
	       unsigned long aIn2GPIOPort,
	       unsigned int  aIn2GPIOPin,
	       unsigned long aPWMGPIOPort,
	       unsigned int  aPWMGPIOPin):
  mIn1GPIOPort(aIn1GPIOPort),
  mIn2GPIOPort(aIn2GPIOPort),
  mPWMGPIOPort(aPWMGPIOPort),
  mIn1GPIOPin(aIn1GPIOPin),
  mIn2GPIOPin(aIn2GPIOPin),
  mPWMGPIOPin(aPWMGPIOPin) {

  // In1.
  GPIOPinTypeGPIOOutput(mIn1GPIOPort, mIn1GPIOPin);
  GPIOPadConfigSet(mIn1GPIOPort,
		   mIn1GPIOPin,
		   GPIO_STRENGTH_4MA,
		   GPIO_PIN_TYPE_STD);

  // In2.
  GPIOPinTypeGPIOOutput(mIn2GPIOPort, mIn2GPIOPin);
  GPIOPadConfigSet(mIn2GPIOPort,
		   mIn2GPIOPin,
		   GPIO_STRENGTH_4MA,
		   GPIO_PIN_TYPE_STD);

  // PWM.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
  GPIOPinTypePWM(mPWMGPIOPort, mPWMGPIOPin);
  GPIOPadConfigSet(mPWMGPIOPort,
		   mPWMGPIOPin,
		   GPIO_STRENGTH_4MA,
		   GPIO_PIN_TYPE_STD);

  PWMGenConfigure(PWM_BASE,
		  PWM_GEN_1,
		  PWM_GEN_MODE_DOWN |
		  PWM_GEN_MODE_NO_SYNC |
		  PWM_GEN_MODE_DBG_STOP |
		  PWM_GEN_MODE_GEN_NO_SYNC |
		  PWM_GEN_MODE_DB_NO_SYNC);

  // 8MHz SysClk = 125ns period.
  // 125ns * 400 = 50us = 20KHz PWM period.
  PWMGenPeriodSet(PWM_BASE,
		  PWM_GEN_1,
		  (400 - 1)); //ulPeriod);
  PWMPulseWidthSet(PWM_BASE,
		   PWM_GEN_1,
		   (320 - 1)); //ulWidth);
  //PWMGenEnable(PWM_BASE,
  //	       PWM_GEN_1);
  PWMOutputState(PWM_BASE,
		 PWM_OUT_2_BIT,
		 true);

}


void TB6612::TurnOnCW(unsigned int aDutyCycle) const {

  // In1: H
  // In2: L
  // Turn on PWM output.
  GPIOPinWrite(mIn1GPIOPort, mIn1GPIOPin, mIn1GPIOPin);
  GPIOPinWrite(mIn2GPIOPort, mIn2GPIOPin, 0);
  PWMGenEnable(PWM_BASE, PWM_GEN_1);
}


void TB6612::TurnOnCCW(unsigned int aDutyCycle) const {

  // In1: L
  // In2: H
  // Turn on PWM output.
  GPIOPinWrite(mIn1GPIOPort, mIn1GPIOPin, 0);
  GPIOPinWrite(mIn2GPIOPort, mIn2GPIOPin, mIn2GPIOPin);
  PWMGenEnable(PWM_BASE, PWM_GEN_1);
}


void TB6612::TurnOff(void) const {

  // Turn off PWM output.
  // In1: L
  // In2: L
  PWMGenDisable(PWM_BASE, PWM_GEN_1);
  GPIOPinWrite(mIn1GPIOPort, mIn1GPIOPin, 0);
  GPIOPinWrite(mIn2GPIOPort, mIn2GPIOPin, 0);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************
 
// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
