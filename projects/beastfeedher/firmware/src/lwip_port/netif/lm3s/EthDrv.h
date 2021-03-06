#pragma once
// *******************************************************************************
//
// Project: LwIP
//
// Module: LM3S6965 low-level Ethernet driver.
//
// *******************************************************************************

//! \file
//! \brief Ethernet driver class.
//! \ingroup lwip

// ******************************************************************************
//
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "LwIPDrv.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class EthDrv
    : public LwIPDrv {
public:
    EthDrv(unsigned int aIndex, EthernetAddress const &aEthernetAddress, unsigned int aBufQueueSize);
    ~EthDrv() {}

    void DisableAllInt(void) override;
    void EnableAllInt(void) override;

private:
    // LwIP Interface.
    err_t EtherIFInit(struct netif * const aNetIF) override;
    void ISR(void) override;

    void LowLevelTx(struct pbuf * const aPBuf) override;
    struct pbuf *LowLevelRx(void) override;
    void FreePBuf(struct pbuf * const aPBuf) override;

    void EnableRxInt(void) override;
    bool IsTxEmpty(void) const override;
};

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
