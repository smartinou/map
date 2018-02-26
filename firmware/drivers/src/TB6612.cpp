// *****************************************************************************
//
// Project: Component drivers.
//
// Module: Motor controller class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2016-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// TI Library.
#include "hw_types.h"
#include "gpio.h"

// This project.
#include "GPIOs.h"
#include "TB6612.h"

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
#if 0
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
  //           PWM_GEN_1);
  PWMOutputState(PWM_BASE,
                 PWM_OUT_2_BIT,
                 true);
}
#endif

TB6612::TB6612(GPIOs &aIn1,
               GPIOs &aIn2,
               GPIOs &aPWM)
  : mIn1(aIn1)
  , mIn2(aIn2)
  , mPWM(aPWM) {

  // In1.
  GPIOPinTypeGPIOOutput(mIn1.GetPort(), mIn1.GetPin());
  GPIOPadConfigSet(mIn1.GetPort(),
                   mIn1.GetPin(),
                   GPIO_STRENGTH_4MA,
                   GPIO_PIN_TYPE_STD);

  // In2.
  GPIOPinTypeGPIOOutput(mIn2.GetPort(), mIn2.GetPin());
  GPIOPadConfigSet(mIn2.GetPort(),
                   mIn2.GetPin(),
                   GPIO_STRENGTH_4MA,
                   GPIO_PIN_TYPE_STD);

  // PWM.
  GPIOPinTypeGPIOOutput(mPWM.GetPort(), mPWM.GetPin());
  GPIOPadConfigSet(mPWM.GetPort(),
                   mPWM.GetPin(),
                   GPIO_STRENGTH_4MA,
                   GPIO_PIN_TYPE_STD);
}


void TB6612::TurnOnCW(unsigned int aDutyCycle) const {

  // PWM: H
  // In1: H
  // In2: L
  //PWMGenEnable(PWM_BASE, PWM_GEN_1);
  GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
  GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), mIn1.GetPin());
  GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), 0);
}


void TB6612::TurnOnCCW(unsigned int aDutyCycle) const {

  // PWM: H
  // In1: L
  // In2: H
  GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
  //PWMGenEnable(PWM_BASE, PWM_GEN_1);
  GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), 0);
  GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), mIn2.GetPin());
}


void TB6612::TurnOff(void) const {

  // PWM: H
  // In1: L
  // In2: L
  //PWMGenDisable(PWM_BASE, PWM_GEN_1);
  GPIOPinWrite(mPWM.GetPort(), mPWM.GetPin(), mPWM.GetPin());
  GPIOPinWrite(mIn1.GetPort(), mIn1.GetPin(), 0);
  GPIOPinWrite(mIn2.GetPort(), mIn2.GetPin(), 0);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
