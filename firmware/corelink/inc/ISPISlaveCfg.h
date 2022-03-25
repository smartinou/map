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

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class ISPISlaveCfg {
public:

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
    virtual void SetBitRate(unsigned int const aBitRate) = 0;

    virtual protocol_t GetProtocol(void) const = 0;
    virtual unsigned int GetBitRate(void) const = 0;
    virtual unsigned int GetDataWidth(void) const = 0;

    virtual void AssertCSn(void) const = 0;
    virtual void DeassertCSn(void) const = 0;

    virtual void InitCSnGPIO(void) const = 0;
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

