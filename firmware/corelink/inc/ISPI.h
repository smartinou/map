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

#include "CoreLinkPeripheralDev.h"


namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class ISPISlaveCfg {
public:
    virtual ~ISPISlaveCfg() {}

    enum class PROTOCOL {
        MOTO_0 = 0,
        MOTO_1,
        MOTO_2,
        MOTO_3,
        TI,
        NMW
    };

#ifdef _WIN32
    typedef enum class PROTOCOL protocol_t;
#else
    typedef enum PROTOCOL protocol_t;
#endif
    virtual void SetProtocol(protocol_t aProtocol) = 0;
    virtual void SetBitRate(unsigned int aBitRate) = 0;
    virtual void SetDataWidth(unsigned int aDataWidth) = 0;

    virtual protocol_t GetProtocol(void) const = 0;
    virtual unsigned int GetBitRate(void) const = 0;
    virtual unsigned int GetDataWidth(void) const = 0;

    virtual void AssertCSn(void) = 0;
    virtual void DeassertCSn(void) = 0;
};


// [MG] STARTING TO WONDER IF THIS WHOLE CLASS IS USEFULL AT ALL.
// [MG] WOULDN'T SETTING THE PINS DIRECTLY IN SPIDev BE MORE EFFICIENT?
class SSIPinCfg {
public:
    SSIPinCfg(unsigned int aSSIID) : mID(aSSIID) {}
    virtual ~SSIPinCfg() {}

    unsigned int GetID(void) const { return mID; }
    virtual void SetPins(void) const = 0;

private:
    unsigned int mID;
};


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

