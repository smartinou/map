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
    virtual ~ISPISlaveCfg() = default;

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

