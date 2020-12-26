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

#include "ISPISlaveCfg.h"


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
    //SPISlaveCfg(uint32_t aPort, uint8_t aPin);
    SPISlaveCfg(GPIO const &aGPIO);
    ~SPISlaveCfg() {}

    // ISPISlaveCfg interface.
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
#if 0
    uint32_t mCSnGPIOPort;
    uint8_t mCSnGPIOPin;
#else
    GPIO const mCSnGPIO;
#endif
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

