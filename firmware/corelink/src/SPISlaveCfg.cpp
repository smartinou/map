// *****************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *****************************************************************************

//! \file
//! \brief CoreLink peripheral SPI device class definition.
//! \ingroup corelink_peripherals

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

// This project.
#include "SPISlaveCfg.h"

// Standard Libraries.
#include <cstdint>

// TI Library.
// Compatible with both StellarisWare & TivaWare.
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

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

void CoreLink::SPISlaveCfg::InitCSnGPIO() const noexcept {

    // Enable and configures the GPIO pin used for CSn.
    // The proper GPIO peripheral must be enabled using
    // SysCtlPeripheralEnable() prior to the following calls,
    // otherwise CPU will rise a HW fault.
    MAP_GPIOPinTypeGPIOOutput(mCSnGPIO.GetPort(), mCSnGPIO.GetPin());
    MAP_GPIOPadConfigSet(
        mCSnGPIO.GetPort(),
        mCSnGPIO.GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );

    // Put the CSn pin in deasserted state.
    DeassertCSn();
}


void CoreLink::SPISlaveCfg::AssertCSn() const noexcept {

    MAP_GPIOPinWrite(mCSnGPIO.GetPort(), mCSnGPIO.GetPin(), 0);
}


void CoreLink::SPISlaveCfg::DeassertCSn() const noexcept {

    MAP_GPIOPinWrite(mCSnGPIO.GetPort(), mCSnGPIO.GetPin(), mCSnGPIO.GetPin());
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
