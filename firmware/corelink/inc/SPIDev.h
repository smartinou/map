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
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "CoreLinkPeripheralDev.h"
#include "ISPIDev.h"
#include "ISPISlaveCfg.h"
#include "SSIPinCfg.h"

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \class SPIDev
//! \brief SPI device
class SPIDev
    : public ISPIDev
    , public PeripheralDev {

public:
    explicit SPIDev(
        uint32_t const aBaseAddr,
        uint32_t const aClkRate,
        SSIPinCfg const &aSPIMasterPinCfgRef
    ) noexcept;
    ~SPIDev() = default;

    // ISPIDev interface.
    void RdData(
        uint8_t const aAddr,
        uint8_t * const aData,
        std::size_t aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) override;

    void RdData(
        uint8_t * const aData,
        unsigned int aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) override;

    void WrData(
        uint8_t const aAddr,
        uint8_t const * const aData,
        unsigned int aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) override;

    void WrData(
        uint8_t const * const aData,
        unsigned int aLen,
        ISPISlaveCfg const &aSPICfgRef
    ) override;

    uint8_t PushPullByte(uint8_t const aByte) override;
    uint8_t PushPullByte(uint8_t const aByte, ISPISlaveCfg const &aSPICfgRef) override;

private:
    void SetCfg(ISPISlaveCfg const &aSPISlaveCfgRef);

    static unsigned int ToNativeProtocol(ISPISlaveCfg::PROTOCOL const aProtocol);

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
