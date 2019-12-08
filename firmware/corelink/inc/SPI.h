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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************


#include "inc/GPIO.h"

#include "ISPI.h"


namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class SPISlaveCfg
    : public ISPISlaveCfg {
public:
    SPISlaveCfg(unsigned int aPort, unsigned int aPin);
    SPISlaveCfg(GPIO const &aGPIO);
    ~SPISlaveCfg() {}

    //typedef enum PROTOCOL_ENUM_TAG protocol_t;

    void SetProtocol(protocol_t aProtocol) override { mProtocol = aProtocol; }
    void SetBitRate(unsigned int aBitRate) override { mBitRate = aBitRate; }
    void SetDataWidth(unsigned int aDataWidth) override { mDataWidth = aDataWidth; }

    protocol_t GetProtocol(void) const override { return mProtocol; }
    unsigned int GetBitRate(void) const override { return mBitRate; }
    unsigned int GetDataWidth(void) const override { return mDataWidth; }

    void AssertCSn(void) override;
    void DeassertCSn(void) override;

private:
    void SetCSnGPIO(void);

    SPISlaveCfg() = delete;

    protocol_t mProtocol;
    unsigned long mBitRate;
    unsigned long mDataWidth;

    unsigned long mCSnGPIOPort;
    unsigned int mCSnGPIOPin;
};


class SPIDev
    : public ISPIDev, public PeripheralDev {
public:
    SPIDev(uint32_t aBaseAddr, SSIPinCfg &aSPIMasterPinCfgRef);
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

