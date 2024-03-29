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
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
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

#include "SPIDev.h"

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

CoreLink::SPIDev::SPIDev(uint32_t aBaseAddr, uint32_t aClkRate, SSIPinCfg const &aSPIMasterPinCfgRef)
    : PeripheralDev(aBaseAddr, aClkRate)
    , mLastSPICfgPtr(nullptr) {

    MAP_SSIDisable(aBaseAddr);
        aSPIMasterPinCfgRef.SetPins();
    MAP_SSIEnable(aBaseAddr);
}


CoreLink::SPIDev::~SPIDev() {

}


void CoreLink::SPIDev::RdData(
    uint8_t aAddr,
    uint8_t * const aData,
    unsigned int aLen,
    CoreLink::ISPISlaveCfg &aSPICfgRef
)
{

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
}


void CoreLink::SPIDev::RdData(
    uint8_t * const aData,
    unsigned int aLen,
    CoreLink::ISPISlaveCfg &aSPICfgRef
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
}


void CoreLink::SPIDev::WrData(
    uint8_t aAddr,
    uint8_t const * const aData,
    unsigned int aLen,
    CoreLink::ISPISlaveCfg &aSPICfgRef
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
}


void CoreLink::SPIDev::WrData(
    uint8_t const * const aData,
    unsigned int aLen,
    CoreLink::ISPISlaveCfg &aSPICfgRef
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
}


uint8_t CoreLink::SPIDev::PushPullByte(uint8_t const aByte) {

    unsigned int const lBaseAddr = GetBaseAddr();
    unsigned long lRxData = 0UL;
    MAP_SSIDataPut(lBaseAddr, aByte);
    MAP_SSIDataGet(lBaseAddr, &lRxData);

    return static_cast<uint8_t>(lRxData);
}


uint8_t CoreLink::SPIDev::PushPullByte(uint8_t const aByte, CoreLink::ISPISlaveCfg &aSPICfgRef) {

    SetCfg(aSPICfgRef);
    return PushPullByte(aByte);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void CoreLink::SPIDev::SetCfg(ISPISlaveCfg &aSPISlaveCfgRef) {

    if (mLastSPICfgPtr != &aSPISlaveCfgRef) {
        // The config changed since the last SPI call.
        // Reconfigure with the newly specified config.
        // Set the new SPI config as the new one.
        uint32_t lBaseAddr = GetBaseAddr();
        MAP_SSIDisable(lBaseAddr);

        // Could check that data width is in range [4, 16].
        ISPISlaveCfg::PROTOCOL lProtocol = aSPISlaveCfgRef.GetProtocol();
        unsigned int lNativeProtocol = ToNativeProtocol(lProtocol);
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
}


unsigned int CoreLink::SPIDev::ToNativeProtocol(ISPISlaveCfg::PROTOCOL aProtocol) {

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
