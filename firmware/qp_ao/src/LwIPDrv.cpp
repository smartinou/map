// *****************************************************************************
//
// Project: LwIP
//
// Module: LwIP Ethernet driver.
//
// *******************************************************************************

//! \file
//! \brief LwIP Ethernet Driver class.
//! \ingroup lwip

// *****************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Libraries.
#include <string.h>

// QP Library.
#include <qpcpp.h>

extern "C" {
// LwIP.
#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"

// TI Library.
#include <hw_ethernet.h>
#include <hw_ints.h>
#include <hw_memmap.h>
#include <hw_types.h>
#include <driverlib/ethernet.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>

#include "netif/etharp.h"
} // extern "C"

#include "LwIPDrv.h"

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

std::map<struct netif * const, LwIPDrv * const> LwIPDrv::sMap;
std::vector<LwIPDrv *> LwIPDrv::sVector({nullptr});

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void LwIPDrv::StaticInit(
    QP::QActive * const aAO,
    bool aUseDHCP,
    uint32_t aIPAddress,
    uint32_t aSubnetMask,
    uint32_t aGWAddress
) {
    // Go through all registered network drivers and call their Init() function.
    for (std::vector<LwIPDrv *>::iterator lIt = sVector.begin(); lIt != sVector.end(); ++lIt) {
        (*lIt)->DrvInit(aAO, aUseDHCP, aIPAddress, aSubnetMask, aGWAddress);
    }
}


void LwIPDrv::StaticRd(unsigned int aIndex) {
    sVector[aIndex]->Rd();
}


void LwIPDrv::StaticWr(unsigned int aIndex) {
    sVector[aIndex]->Wr();
}


void LwIPDrv::StaticISR(unsigned int aIndex) {
    sVector[aIndex]->ISR();
}


uint8_t const *LwIPDrv::StaticGetMACAddress(unsigned int aIndex) {
    return sVector[aIndex]->GetMACAddress();
}


uint32_t LwIPDrv::StaticGetIPAddress(unsigned int aIndex) {
    return sVector[aIndex]->GetIPAddress();
}


uint32_t LwIPDrv::StaticGetSubnetMask(unsigned int aIndex) {
    return sVector[aIndex]->GetSubnetMask();
}


uint32_t LwIPDrv::StaticGetDefaultGW(unsigned int aIndex) {
    return sVector[aIndex]->GetDefaultGW();
}


uint8_t const *LwIPDrv::GetMACAddress(void) const {
    return &mNetIF.hwaddr[0];
}


uint32_t LwIPDrv::GetIPAddress(void) const {
    return mNetIF.ip_addr.addr;
}


uint32_t LwIPDrv::GetSubnetMask(void) const {
    return mNetIF.netmask.addr;
}


uint32_t LwIPDrv::GetDefaultGW(void) const {
    return mNetIF.gw.addr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// Low-level init.
// Will be called by LwIP at init stage, everytime a netif is added.
err_t LwIPDrv::StaticEtherIFInit(struct netif * const aNetIF) {
    // Find the instance in hash that owns this struct netif.
    auto lIt = LwIPDrv::sMap.find(aNetIF);
    if (lIt != LwIPDrv::sMap.end()) {
        return lIt->second->EtherIFInit(aNetIF);
    }

    return ERR_ARG;
}


err_t LwIPDrv::StaticEtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf) {
    // Find the instance in hash that owns this struct netif.
    auto lIt = LwIPDrv::sMap.find(aNetIF);
    if (lIt != LwIPDrv::sMap.end()) {
        return lIt->second->EtherIFOut(aNetIF, aPBuf);
    }

    return ERR_ARG;
}


LwIPDrv::LwIPDrv(unsigned int aIndex, unsigned int aPBufQSize)
    : mMyIndex(aIndex)
    , mNetIF{0} {

    mPBufQ = new PBufQ(aPBufQSize);

    // Associate this <struct netif *, LwIPDrv>.
    LwIPDrv::sMap.insert(std::pair<struct netif * const, LwIPDrv *const>(&mNetIF, this));
    LwIPDrv::sVector[aIndex] = this;
}


LwIPDrv::PBufQ::PBufQ(unsigned int aQSize)
    : mPBufRing(nullptr)
    , mRingSize(aQSize)
    , mQWrIx(0)
    , mQRdIx(0)
    , mQOverflow(0) {

    // Ctor body.
    mPBufRing = new struct pbuf *[aQSize];
}


bool LwIPDrv::PBufQ::IsEmpty(void) const {
    return (mQWrIx == mQRdIx);
}


bool LwIPDrv::PBufQ::Put(struct pbuf * const aPBufPtr) {
    unsigned int lNextQWr = mQWrIx + 1;

    if (lNextQWr == mRingSize) {
        lNextQWr = 0;
    }

    if (lNextQWr != mQRdIx) {
        // The queue isn't full so we add the new frame at the current
        // write position and move the write pointer.
        mPBufRing[mQWrIx] = aPBufPtr;
        if ((++mQWrIx) == mRingSize) {
            mQWrIx = 0;
        }

        // Successfully posted the pbuf.
        return true;
    } else {
        // The stack is full so we are throwing away this value.
        // Keep track of the number of times this happens.
        mQOverflow++;
        // Could not post the pbuf.
        return false;
    }
}


struct pbuf *LwIPDrv::PBufQ::Get(void) {
    struct pbuf *lPBuf = nullptr;

    if (!IsEmpty()) {
        // The queue is not empty so return the next frame from it.
        // Adjust the read pointer accordingly.
        lPBuf = mPBufRing[mQRdIx];
        if ((++mQRdIx) == mRingSize) {
            mQRdIx = 0;
        }
    }

    return lPBuf;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
