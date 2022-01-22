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
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
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

class SPISlaveCfg final
    : public ISPISlaveCfg {
public:
    SPISlaveCfg(GPIO const &aGPIO);
    ~SPISlaveCfg() = default;

    // ISPISlaveCfg interface.
    void SetProtocol(protocol_t aProtocol) override {mProtocol = aProtocol;}
    void SetBitRate(unsigned int aBitRate) override {mBitRate = aBitRate;}
    void SetDataWidth(unsigned int aDataWidth) override {mDataWidth = aDataWidth;}

    protocol_t GetProtocol(void) const override {return mProtocol;}
    unsigned int GetBitRate(void) const override {return mBitRate;}
    unsigned int GetDataWidth(void) const override {return mDataWidth;}

    void AssertCSn(void) override;
    void DeassertCSn(void) override;

private:
    SPISlaveCfg() = delete;

    void SetCSnGPIO(void);

    protocol_t mProtocol;
    unsigned long mBitRate;
    unsigned long mDataWidth;
    GPIO const mCSnGPIO;
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

