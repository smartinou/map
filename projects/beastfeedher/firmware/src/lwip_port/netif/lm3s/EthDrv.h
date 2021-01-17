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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
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

// Forward declarations.
namespace QP {
    class QActive;
} // namespace QP


class EthDrv
    : public LwIPDrv {
public:
    EthDrv(unsigned int aIndex, EthernetAddress const &aEthernetAddress, unsigned int aBufQueueSize);
    ~EthDrv() {}

    void DrvInit(
        QP::QActive * const aAO,
        bool aUseDHCP,
        uint32_t aIPAddr,
        uint32_t aSubnetMask,
        uint32_t aGWAddr
    ) override;
    err_t EtherIFInit(struct netif * const aNetIF) override;
    err_t EtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf) override;
    void Rd(void) override;
    void Wr(void) override;
    void ISR(void) override;

private:

    void LowLevelTx(struct pbuf * const aPBuf);
    struct pbuf *LowLevelRx(void);
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
