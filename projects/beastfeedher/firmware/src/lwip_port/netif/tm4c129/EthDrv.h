#pragma once
// *******************************************************************************
//
// Project: LwIP
//
// Module: TivaWare low-level Ethernet driver.
//
// *******************************************************************************

//! \file
//! \brief Ethernet driver class.
//! Compatible with TivaWare library.
//! \ingroup lwip

// ******************************************************************************
//
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// LwIP.
#include "lwip/pbuf.h"

#include "LwIPDrv.h"

#include "RxDescriptor.h"
#include "TxDescriptor.h"

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
    err_t EtherIFOut(struct pbuf * const aPBuf) override;
    void Rd(void) override;
    void Wr(void) override {}

    err_t EtherIFInit(struct netif * const aNetIF) override;
    void ISR(void) override;

    // Local interface.
    struct pbuf *LowLevelRx(RxDescriptor * const aDescriptor);
    //void FreePBuf(struct pbuf * const aPBuf);

    void EnableRxInt(void);

    RxDescriptorChain mRxRingBuf;
    TxRingBuf mTxRingBuf;
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
