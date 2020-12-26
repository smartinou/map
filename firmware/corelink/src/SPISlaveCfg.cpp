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
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stdio.h>

// TI Library.
// Compatible with both StellarisWare & TivaWare.
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

#include "SPISlaveCfg.h"

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

CoreLink::SPISlaveCfg::SPISlaveCfg(GPIO const &aGPIO)
    : mProtocol(PROTOCOL::MOTO_0)
    , mBitRate(0)
    , mDataWidth(8)
    //: SPISlaveCfg(aGPIO.GetPort(), aGPIO.GetPin()) {
    , mCSnGPIO(aGPIO) {
    // Ctor body.
    SetCSnGPIO();
}


void CoreLink::SPISlaveCfg::SetCSnGPIO(void) {

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


void CoreLink::SPISlaveCfg::AssertCSn(void) {

    MAP_GPIOPinWrite(mCSnGPIO.GetPort(), mCSnGPIO.GetPin(), 0);
}


void CoreLink::SPISlaveCfg::DeassertCSn(void) {

    MAP_GPIOPinWrite(mCSnGPIO.GetPort(), mCSnGPIO.GetPin(), mCSnGPIO.GetPin());
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
