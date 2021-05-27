// *****************************************************************************
//
// Project: LwIP
//
// Module: TivaWare low-level Ethernet driver - support classes.
//
// *****************************************************************************

//! \file
//! \brief Ethernet driver support class.
//! \ingroup lwip

// *****************************************************************************
//
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// This project.
#include "TxDescriptor.h"

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

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

TxDescriptor::TxDescriptor()
    : tEMACDMADescriptor {
        .ui32CtrlStatus = DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_CHAINED | DES0_TX_CTRL_IP_ALL_CKHSUMS
        , .ui32Count = 0
        , .pvBuffer1 = nullptr
        , .DES3 = {0}
        , .ui32ExtRxStatus = 0
        , .ui32Reserved = 0
        , .ui32IEEE1588TimeLo = 0
        , .ui32IEEE1588TimeHi = 0
    }
{
    // Ctor body.
}


void TxDescriptor::FreePBuf(void) {
    if (mPBuf != nullptr) {
        pbuf_free(mPBuf);
        pvBuffer1 = nullptr;
    }
}


TxDescriptor *TxRingBuf::Create(size_t aSize) {
    // Fill the map of descriptors.
    for (size_t lIx = 0; lIx < aSize; lIx++) {
        TxDescriptor *lDescriptor = new TxDescriptor;
        if (lDescriptor != nullptr) {
            if (mHead == nullptr) {
                // Add to empty.
                mHead = lDescriptor;
                lDescriptor->ChainTo(mHead);
            } else {
                // Insert at "end" of circular list.
                lDescriptor->ChainTo(static_cast<TxDescriptor *>(mHead->DES3.pLink));
                mHead->ChainTo(lDescriptor);
            }
            // Don't give descriptor to HW yet: this will be done when pbufs are assigned.
        } else {
            // Failed to allocated number of requested descriptors in the chain.
            // Free them and bailout.
            Free();
            return nullptr;
        }
    }

    mSize = aSize;
    mTail = mHead;
    mBkp = mHead;
    return mHead;
}


TxDescriptor *TxRingBuf::PutPBufs(struct pbuf *aPBuf, bool aIsFirstPBuf) {
    if (aIsFirstPBuf) {
        // Save the head in case we need to backtrack.
        SaveHead();
    }

    TxDescriptor *lDescriptor = mHead;
    while (aPBuf && !IsFull() && !lDescriptor->IsHWOwned()) {
        // Operation succeeded. Update head pointer.
        mHead = lDescriptor->GetNext();

        // Check if this is a chained pbuf.
        if (aPBuf->next != nullptr) {
            TxDescriptor *lDummy = PutPBufs(aPBuf, false);
            if (lDummy == nullptr) {
                // Failed to store other pbufs of the chain.
                break;
            }
        } else {
            // This is the end of the chain of pbufs. Mark it.
            lDescriptor->SetFrameEnd();
        }
        lDescriptor->SetPBuf(aPBuf);
        lDescriptor->SetPayload(aPBuf->payload);
        lDescriptor->SetLen(aPBuf->len);

        if (aIsFirstPBuf) {
            // This is the 1st descriptor of the chain. Mark it.
            lDescriptor->SetFrameStart();
        }

        // Give to HW last: this way, if failing to allocate a descriptor in the chain,
        // it will be easier to reclaim them for SW usage.
        lDescriptor->GiveToHW();
        return lDescriptor;
    }

    // Either no pbuf, ring full or descriptor owned by HW.
    if (aIsFirstPBuf) {
        RestoreHead();
    }
    return nullptr;
}


TxDescriptor *TxRingBuf::GetPBufs(void) {

    // Should be SW-owned now, but check anyway.
    TxDescriptor *lDescriptor = mTail;
    if ((lDescriptor != nullptr) && !lDescriptor->IsHWOwned()) {
        // This descriptor was released from the HW.
        // Increment Tail as far as the pbuf spans.
        struct pbuf *lPBuf = lDescriptor->GetPBuf();
        mTail = lDescriptor->GetNext();
        while (lPBuf->next != nullptr) {
            // Opt: check that the stored pbuf for this descriptor is null as expected.
            lPBuf = lPBuf->next;
            mTail = mTail->GetNext();
        }

        // Once tail goes beyond the last descriptor, free the pbuf.
        lDescriptor->FreePBuf();
        return mTail;
    }

    // Reached current descriptor or hit a busy descriptor prematurely: bail out.
    return nullptr;

}


void TxRingBuf::Free(void) {

    TxDescriptor * const lDescriptor = mHead;
    for (size_t lIx = 0; lIx < mSize; lIx++) {
        
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
