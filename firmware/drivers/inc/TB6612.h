#ifndef TB6612_H_
#define TB6612_H_
// *******************************************************************************
//
// Project: Beast Feed'Her.
//
// Module: Motor controller class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard Libraries.

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class TB6612 {
 public:
  TB6612(unsigned long aIn1GPIOPort,
	 unsigned int  aIn1GPIOPin,
	 unsigned long aIn2GPIOPort,
	 unsigned int  aIn2GPIOPin,
	 unsigned long aPWMGPIOPort,
	 unsigned int  aPWMGPIOPin);
  ~TB6612() {}

  // Sets/clears the entry for the specified time, rounded to quarter hour.
  void TurnOnCW(unsigned int aDutyCycle) const;
  void TurnOnCCW(unsigned int aDutyCycle) const;
  void TurnOff(void) const;

  // Gets the next set entry from current time.
 private:
  unsigned long const mIn1GPIOPort;
  unsigned long const mIn2GPIOPort;
  unsigned long const mPWMGPIOPort;

  unsigned int  const mIn1GPIOPin;
  unsigned int  const mIn2GPIOPin;
  unsigned int  const mPWMGPIOPin;
};

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
#endif // TB6612_H_
