// *****************************************************************************
//
// Project: LwIP
//
// Module: LM3S6965 low-level Ethernet driver.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

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
// TI Library.
#include <hw_ethernet.h>
#include <hw_ints.h>
#include <hw_memmap.h>
#include <hw_types.h>
#include <driverlib/ethernet.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>

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

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// Low-level init.
// Will be called by LwIP at init stage.
err_t LwIPDrv::StaticEtherIFInit(struct netif * const aNetIF) {
    // Initialize all ethernet interfaces.
    for (auto& lKeyValue : LwIPDrv::sMap) {
        err_t lErr = lKeyValue.second->EtherIFInit(lKeyValue.first);
        if (lErr != ERR_OK) {
            // Return on 1st error.
            return lErr;
        }
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


// TODO: add parameter for the network interface.
void LwIPDrv::StaticISR(void) {
    //find_in_map->second->ISR();
}


uint32_t LwIPDrv::GetIPAddress(void) const {
    return mNetIF.ip_addr.addr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

LwIPDrv::LwIPDrv(unsigned int aIndex, unsigned int aPBufQSize)
    : mMyIndex(aIndex) {

    mPBufQ = new PBufQ(aPBufQSize);

    // Associate this <struct netif *, LwIPDrv>.
    LwIPDrv::sMap.insert(std::pair<struct netif * const, LwIPDrv *const>(&mNetIF, this));
    //LwIPDrv::sVector.push_back(this);
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


bool LwIPDrv::PBufQ::Put(struct pbuf *aPBufPtr) {
    unsigned int lNextQWr = mQWrIx + 1;

    if (lNextQWr == Q_DIM(mPBufRing)) {
        lNextQWr = 0;
    }

    if (lNextQWr != mQRdIx) {
        // The queue isn't full so we add the new frame at the current
        // write position and move the write pointer.
        mPBufRing[mQWrIx] = aPBufPtr;
        if ((++mQWrIx) == Q_DIM(mPBufRing)) {
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
        if ((++mQRdIx) == Q_DIM(mPBufRing)) {
            mQRdIx = 0;
        }
    }

    return lPBuf;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
