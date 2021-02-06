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


#include "inc/GPIO.h"

#include "ISPI.h"
#include "ISPISlaveCfg.h"
#include "SSIPinCfg.h"

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class SPIDev
    : public ISPIDev, public PeripheralDev {
public:
    SPIDev(uint32_t aBaseAddr, uint32_t aClkRate, SSIPinCfg &aSPIMasterPinCfgRef);
    ~SPIDev();

    void RdData(
        uint8_t aAddr,
        uint8_t * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) override;

    void RdData(
        uint8_t * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) override;

    void WrData(
        uint8_t aAddr,
        uint8_t const * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) override;

    void WrData(
        uint8_t const * const aData,
        unsigned int aLen,
        ISPISlaveCfg &aSPICfgRef
    ) override;

    uint8_t PushPullByte(uint8_t const aByte) override;
    uint8_t PushPullByte(uint8_t const aByte, ISPISlaveCfg &aSPICfgRef) override;

private:
    void SetCfg(ISPISlaveCfg &aSPISlaveCfgRef);

    static unsigned int ToNativeProtocol(ISPISlaveCfg::PROTOCOL aProtocol);

    ISPISlaveCfg const *mLastSPICfgPtr;
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

