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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "ISPISlaveCfg.h"

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Interface for SPI devices.
class ISPIMasterDev {
public:
    virtual ~ISPIMasterDev() {}

    virtual void RdData(
        uint8_t const aAddr,
        uint8_t * const aData,
        std::size_t aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) = 0;

    virtual void RdData(
        uint8_t * const aData,
        std::size_t aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) = 0;

    virtual void WrData(
        uint8_t const aAddr,
        uint8_t const * const aData,
        std::size_t aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) = 0;

    virtual void WrData(
        uint8_t const * const aData,
        std::size_t aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) = 0;

    virtual uint8_t PushPullByte(uint8_t const aByte) = 0;
    virtual uint8_t PushPullByte(uint8_t const aByte, ISPISlaveCfg const &aSPICfgRef) = 0;
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
