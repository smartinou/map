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
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <cstdint>

// TI Library.
#include <inc/hw_memmap.h>
#include <inc/hw_ssi.h>
#include <inc/hw_sysctl.h>
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/ssi.h>
#include <driverlib/sysctl.h>

#include "SPIMasterDev.h"

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

CoreLink::SPIMasterDev::SPIMasterDev(
    uint32_t const aBaseAddr,
    uint32_t const aClkRate,
    SSIPinCfg const &aSPIMasterPinCfgRef
)
    : PeripheralDev(aBaseAddr, aClkRate)
    , mLastSPICfgPtr(nullptr)
{
    MAP_SSIDisable(aBaseAddr);
        aSPIMasterPinCfgRef.SetPins();
    MAP_SSIEnable(aBaseAddr);
}


void CoreLink::SPIMasterDev::RdData(
    uint8_t const aAddr,
    uint8_t * const aData,
    std::size_t aLen,
    CoreLink::ISPISlaveCfg const &aSPICfgRef
) {

    SetCfg(aSPICfgRef);

    // Assert the assigned CSn pin.
    aSPICfgRef.AssertCSn();

    // -Send address.
    // -Push dummy data (0s) as many as requested to read.
    PushPullByte(aAddr);
    uint8_t *lPtr = aData;
    while (aLen > 0) {
        *lPtr = PushPullByte(0);
        lPtr++;
        aLen--;
    }

    // Deassert the assigned CSn pin.
    aSPICfgRef.DeassertCSn();
    return;
}


void CoreLink::SPIMasterDev::RdData(
    uint8_t * const aData,
    unsigned int aLen,
    CoreLink::ISPISlaveCfg const &aSPICfgRef
) {

    SetCfg(aSPICfgRef);

    // Assert the assigned CSn pin.
    aSPICfgRef.AssertCSn();

    // -Push dummy data (0s) as many as requested to read.
    uint8_t *lPtr = aData;
    while (aLen > 0) {
        *lPtr = PushPullByte(0);
        lPtr++;
        aLen--;
    }

    // Deassert the assigned CSn pin.
    aSPICfgRef.DeassertCSn();
    return;
}


void CoreLink::SPIMasterDev::WrData(
    uint8_t const aAddr,
    uint8_t const * const aData,
    unsigned int aLen,
    CoreLink::ISPISlaveCfg const &aSPICfgRef
) {

    SetCfg(aSPICfgRef);

    // Assert the assigned CSn pin.
    aSPICfgRef.AssertCSn();

    // -Send address.
    // -Push data as many as requested to write.
    // -Read dummy data to empty receive register.
    // -Wait for all bytes to transmit.
    PushPullByte(aAddr);

    uint8_t const *lPtr = aData;
    while (aLen > 0) {
        PushPullByte(*lPtr);
        lPtr++;
        aLen--;
    }

    // Deassert the assigned CSn pin.
    aSPICfgRef.DeassertCSn();
    return;
}


void CoreLink::SPIMasterDev::WrData(
    uint8_t const * const aData,
    unsigned int aLen,
    CoreLink::ISPISlaveCfg const &aSPICfgRef
) {

    SetCfg(aSPICfgRef);

    // Assert the assigned CSn pin.
    aSPICfgRef.AssertCSn();

    // -Push data as many as requested to write.
    // -Read dummy data to empty receive register.
    // -Wait for all bytes to transmit.
    uint8_t const *lPtr = aData;
    while (aLen > 0) {
        PushPullByte(*lPtr);
        lPtr++;
        aLen--;
    }

    // Deassert the assigned CSn pin.
    aSPICfgRef.DeassertCSn();
    return;
}


uint8_t CoreLink::SPIMasterDev::PushPullByte(uint8_t const aByte) {

    unsigned int const lBaseAddr = GetBaseAddr();
    unsigned long lRxData = 0UL;
    MAP_SSIDataPut(lBaseAddr, aByte);
    MAP_SSIDataGet(lBaseAddr, &lRxData);

    return static_cast<uint8_t>(lRxData);
}


uint8_t CoreLink::SPIMasterDev::PushPullByte(
    uint8_t const aByte,
    CoreLink::ISPISlaveCfg const &aSPICfgRef
) {

    SetCfg(aSPICfgRef);
    return PushPullByte(aByte);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void CoreLink::SPIMasterDev::SetCfg(ISPISlaveCfg const &aSPISlaveCfgRef) {

    if (mLastSPICfgPtr != &aSPISlaveCfgRef) {
        // The config changed since the last SPI call.
        // Reconfigure with the newly specified config.
        // Set the new SPI config as the new one.
        uint32_t const lBaseAddr = GetBaseAddr();
        MAP_SSIDisable(lBaseAddr);

        // Could check that data width is in range [4, 16].
        ISPISlaveCfg::PROTOCOL const lProtocol = aSPISlaveCfgRef.GetProtocol();
        unsigned int const lNativeProtocol = ToNativeProtocol(lProtocol);
        MAP_SSIConfigSetExpClk(
            lBaseAddr,
            GetClkRate(),
            lNativeProtocol,
            SSI_MODE_MASTER,
            aSPISlaveCfgRef.GetBitRate(),
            aSPISlaveCfgRef.GetDataWidth()
        );

        // Make this the "new" slave configuration and
        // enable SPI operations.
        mLastSPICfgPtr = &aSPISlaveCfgRef;
        MAP_SSIEnable(lBaseAddr);
    }
    return;
}


unsigned int CoreLink::SPIMasterDev::ToNativeProtocol(ISPISlaveCfg::PROTOCOL const aProtocol) {

    switch (aProtocol) {
    case ISPISlaveCfg::PROTOCOL::MOTO_0: return SSI_FRF_MOTO_MODE_0;
    case ISPISlaveCfg::PROTOCOL::MOTO_1: return SSI_FRF_MOTO_MODE_1;
    case ISPISlaveCfg::PROTOCOL::MOTO_2: return SSI_FRF_MOTO_MODE_2;
    case ISPISlaveCfg::PROTOCOL::MOTO_3: return SSI_FRF_MOTO_MODE_3;
    case ISPISlaveCfg::PROTOCOL::TI:     return SSI_FRF_TI;
    case ISPISlaveCfg::PROTOCOL::NMW:    return SSI_FRF_NMW;
    }

    // Should never get here.
    return SSI_FRF_MOTO_MODE_0;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
