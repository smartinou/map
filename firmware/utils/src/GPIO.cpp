// *****************************************************************************
//
// Project: Utilities.
//
// Module: GPIO.
//
// *****************************************************************************

//! \file
//! \brief GPIO class.
//! \ingroup utils_gpio

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
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

// This project.
#include "inc/GPIO.h"

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

GPIO::GPIO(unsigned long const aPort, unsigned int const aPin)
    : mPort(aPort)
    , mPin(aPin) {

  // Ctor body left intentionally empty.
}


GPIO::~GPIO() {

    // Dtor body left intentionally empty.
}


unsigned long GPIO::GetPort(void) const {
    return mPort;
}


unsigned int GPIO::GetPin(void) const {
    return mPin;
}


void GPIO::EnableSysCtlPeripheral(uint32_t aPort) {
    switch (aPort) {
    case GPIO_PORTA_AHB_BASE:
    case GPIO_PORTA_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); break;
    case GPIO_PORTB_AHB_BASE:
    case GPIO_PORTB_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); break;
    case GPIO_PORTC_AHB_BASE:
    case GPIO_PORTC_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); break;
    case GPIO_PORTD_AHB_BASE:
    case GPIO_PORTD_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); break;
    case GPIO_PORTE_AHB_BASE:
    case GPIO_PORTE_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); break;
    case GPIO_PORTF_AHB_BASE:
    case GPIO_PORTF_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); break;
    case GPIO_PORTG_AHB_BASE:
    case GPIO_PORTG_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); break;
    case GPIO_PORTH_AHB_BASE:
    case GPIO_PORTH_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH); break;
    case GPIO_PORTJ_AHB_BASE:
    case GPIO_PORTJ_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); break;
    case GPIO_PORTK_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); break;
    case GPIO_PORTL_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); break;
    case GPIO_PORTM_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); break;
    case GPIO_PORTN_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); break;
    case GPIO_PORTP_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); break;
    case GPIO_PORTQ_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ); break;
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
