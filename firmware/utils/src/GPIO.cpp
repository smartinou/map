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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <cstdint>

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

void GPIO::EnableSysCtlPeripheral(unsigned long aPort) {
    switch (aPort) {
    case GPIO_PORTA_AHB_BASE: [[fallthrough]];
    case GPIO_PORTA_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); break;
    case GPIO_PORTB_AHB_BASE: [[fallthrough]];
    case GPIO_PORTB_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); break;
    case GPIO_PORTC_AHB_BASE: [[fallthrough]];
    case GPIO_PORTC_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); break;
    case GPIO_PORTD_AHB_BASE: [[fallthrough]];
    case GPIO_PORTD_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); break;
    case GPIO_PORTE_AHB_BASE: [[fallthrough]];
    case GPIO_PORTE_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); break;
    case GPIO_PORTF_AHB_BASE: [[fallthrough]];
    case GPIO_PORTF_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); break;
    case GPIO_PORTG_AHB_BASE: [[fallthrough]];
    case GPIO_PORTG_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); break;
    case GPIO_PORTH_AHB_BASE: [[fallthrough]];
    case GPIO_PORTH_BASE: MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH); break;
    case GPIO_PORTJ_AHB_BASE: [[fallthrough]];
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
