#pragma once
// *******************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *******************************************************************************

//! \file
//! \brief CoreLink peripheral SPI device class declaration.
//! \ingroup corelink_peripherals

// ******************************************************************************
//
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "CoreLinkPeripheralDev.h"

#include "ISPISlaveCfg.h"

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class ISPIDev {
public:
    virtual ~ISPIDev() {}

    virtual void RdData(
        uint8_t aAddr,
        uint8_t * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) = 0;

    virtual void RdData(
        uint8_t * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) = 0;

    virtual void WrData(
        uint8_t aAddr,
        uint8_t const * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) = 0;

    virtual void WrData(
        uint8_t const * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) = 0;

    virtual uint8_t PushPullByte(uint8_t const aByte) = 0;
    virtual uint8_t PushPullByte(uint8_t const aByte, ISPISlaveCfg &aSPICfgRef) = 0;
};


} // namespace CoreLink

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
