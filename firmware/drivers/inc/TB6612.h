#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: TB6612.
//
// *******************************************************************************

//! \file
//! \brief TB6612 class.
//! \ingroup ext_peripherals

// ******************************************************************************
//
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "IMotorControl.h"
#include "inc/GPIO.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief TB6612 motor controller.
// Creates one motor controller instance.
// A physical TB6612 has two control instances (A & B).
class TB6612 final
    : public IMotorControl {

public:
    explicit TB6612(GPIO const &aIn1, GPIO const &aIn2, GPIO const &aPWM) noexcept;

    // IMotorController interface.
    void TurnOnCW(unsigned int aDutyCycle = 100) const noexcept override;
    void TurnOnCCW(unsigned int aDutyCycle = 100) const noexcept override;
    void TurnOff() const noexcept override;

 private:
    GPIO mIn1;
    GPIO mIn2;
    GPIO mPWM;
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
