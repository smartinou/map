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


tEMACDMADescriptor *TxRingBuf::Create(size_t aSize) {
    // Fill the map of descriptors.
    for (size_t lIx = 0; lIx < aSize; lIx++) {
        TxDescriptor *lDescriptor = new TxDescriptor;
        if (lDescriptor != nullptr) {
            if (mTail == nullptr) {
                // Add to empty.
                mTail = lDescriptor;
                //mHead = lDescriptor;
                lDescriptor->ChainTo(mTail);
            } else {
                // Insert at "end" of circular list.
                lDescriptor->ChainTo(static_cast<TxDescriptor *>(mTail->DES3.pLink));
                mTail->ChainTo(lDescriptor);
                mTail = lDescriptor;
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
    mHead = mTail;
    return mHead;
}


bool TxRingBuf::PushPBuf(struct pbuf * const aPBuf) {
    // Get the head descriptor and start assigning PBuf chain to it.
    bool constexpr sIsFirstPBuf = true;
    return PushPBuf(mHead, aPBuf, sIsFirstPBuf);
}


bool TxRingBuf::PopPBuf(void) {
    return PopPBuf(mHead);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

TxDescriptor *TxRingBuf::GetNext(TxDescriptor * const aDescriptor) const {
    if (!aDescriptor) {
        return mHead;
    } else if (!IsFull(aDescriptor)) {
        return aDescriptor->GetNext();
    }

    return nullptr;
}


bool TxRingBuf::PushPBuf(TxDescriptor * const aDescriptor, struct pbuf * const aPBuf, bool aIsFirstPBuf) {

    if (aPBuf && aDescriptor && !aDescriptor->IsHWOwned()) {
        aDescriptor->Reset();
        // Check if this is a chained pbuf.
        if (aPBuf->next != nullptr) {
            // Acquire next descriptor and let recursive call test it.
            TxDescriptor * const lNextDescriptor = GetNext(aDescriptor);
            bool lResult = PushPBuf(lNextDescriptor, aPBuf->next);
            if (!lResult) {
                // Something went wrong while setting the next element of the pbuf chain.
                // Recurse back propagating the result up to 1st descriptor.
                return false;
            }
        } else {
            // This is the end of the chain of pbufs.
            // Mark the descriptor and set the new head to next descriptor.
            aDescriptor->SetFrameEnd();
            SetHead(aDescriptor->GetNext());
        }
   
        aDescriptor->SetPBuf(aPBuf);
        aDescriptor->SetPayload(aPBuf->payload);
        aDescriptor->SetLen(aPBuf->len);

        if (aIsFirstPBuf) {
            // This is the 1st descriptor of the chain. Mark it.
            aDescriptor->SetFrameStart();
        }

        // Give to HW last: this way, if failing to allocate a descriptor in the chain,
        // it will be easier to reclaim them for SW usage.
        // Also to avoid race condition with multiple descriptors.
        aDescriptor->GiveToHW();
        return true;
    }

    return false;
}


bool TxRingBuf::PopPBuf(TxDescriptor const * const aCurrentDescriptor) {

    bool lResult = true;
    TxDescriptor *lDescriptor = mTail;
    while ((lDescriptor != aCurrentDescriptor) && !lDescriptor->IsHWOwned()) {
        if (lDescriptor->IsFrameStart()) {
            // Free the attached pbuf.
            lDescriptor->FreePBuf();
        }

        // Crude check for any errors for this descriptor.
        if (lDescriptor->IsErrSet()) {
            lResult = false;
        }
        // Nothing to do for intermediate descriptors but reclaim it.
        lDescriptor = lDescriptor->GetNext();
        mTail = lDescriptor;
    }

    return lResult;
}


void TxRingBuf::Free(void) {

    // Start from the current head.
    TxDescriptor * lDescriptor = mHead;
    for (size_t lIx = 0; lIx < mSize; lIx++) {
        TxDescriptor * lNextDescriptor = lDescriptor->GetNext();
        lDescriptor->FreePBuf();
        delete lDescriptor;
        lDescriptor = lNextDescriptor;
    }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
