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
#include "RxDescriptor.h"

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

RxDescriptor::RxDescriptor(uint8_t * const aBuffer, size_t aSize)
    : tEMACDMADescriptor {
        .ui32CtrlStatus = 0
        , .ui32Count = (DES1_RX_CTRL_CHAINED
            | ((static_cast<uint32_t>(aSize) << DES1_RX_CTRL_BUFF1_SIZE_S) & DES1_RX_CTRL_BUFF1_SIZE_M))
        , .pvBuffer1 = aBuffer
        , .DES3 = {0}
        , .ui32ExtRxStatus = 0
        , .ui32Reserved = 0
        , .ui32IEEE1588TimeLo = 0
        , .ui32IEEE1588TimeHi = 0
        }
    , mCustomPBuf {
        .mPBuf {.custom_free_function = RxDescriptor::Free}
    } {

    // Ctor body.
    // Assign free function.
    mCustomPBuf.mPBuf.custom_free_function = RxDescriptor::Free;
    mCustomPBuf.mDescriptor = this;
}


struct pbuf *RxDescriptor::GetAllocedPBuf(size_t aCumulatedLen) {
    struct pbuf *lPBuf = pbuf_alloced_custom(
        PBUF_RAW,
        (GetFrameLen() - aCumulatedLen),
        PBUF_REF,
        &mCustomPBuf.mPBuf,
        GetPayload(),
        GetPayloadSize()
    );

    return lPBuf;
}


RxDescriptorChain::~RxDescriptorChain() {

    // Free the whole memory buffer allocated for individual descriptors.
    if (mBuffer != nullptr) {
        delete [] mBuffer;
    }

    // Free each Descriptor of the chain.
    // [MG] THIS IS ALL WRONG.
    RxDescriptor *lDescriptor = mHead;
    while (lDescriptor->GetNext() != mHead) {
        lDescriptor = lDescriptor->GetNext();
        delete lDescriptor;
    }
}


RxDescriptor *RxDescriptorChain::GetNext(void) {

    RxDescriptor * const lDescriptor = mHead;
    if (!lDescriptor->IsHWOwned()) {
        if (lDescriptor->IsFrameValid()) {
            mHead = lDescriptor->GetNext();
            return lDescriptor;
        }
    }

    return nullptr;
}


RxDescriptor *RxDescriptorChain::Create(uint32_t aBaseAddr, size_t aChainSize, size_t aBufferSize) {

    // Allocate the memory in one chunk. Split it into individual descriptors.
    mBuffer = new uint8_t [aChainSize * aBufferSize];
    uint8_t *lPtr = mBuffer;
    if (mBuffer != nullptr) {
        // Create new descriptors.
        for (unsigned int lIx = 0; lIx < aChainSize; lIx++) {
            Add(lPtr, aBufferSize);
            lPtr += aBufferSize;
        }

        // The receiver SM will fetch the next descriptor than the one it's being set to.
        // Return tail so 1st descriptor used points at head.
        mSize = aBufferSize;
        return mHead;
    }

    return nullptr;
}


void RxDescriptorChain::Add(uint8_t * const aBuffer, size_t aSize) {

    if ((aBuffer != nullptr) && aSize) {
        RxDescriptor * const lDescriptor = new RxDescriptor(aBuffer, aSize);
        // Add to ring buffer. Leverage the chain pointer part of the descriptor.
        if (lDescriptor != nullptr) {
            if (mTail == nullptr) {
                // Add to empty.
                mTail = lDescriptor;
                mHead = lDescriptor;
                lDescriptor->ChainTo(mTail);
            } else {
                // Insert at "end" of circular list.
                lDescriptor->ChainTo(static_cast<RxDescriptor *>(mTail->DES3.pLink));
                mTail->ChainTo(lDescriptor);
                mTail = lDescriptor;
            }
            lDescriptor->GiveToHW();

        } else {
            // Nothing to create and/or chain!
        }
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void RxDescriptor::Free(struct pbuf *aPBuf) {

    // Upcast to a MyCustomPBuf object.
    MyCustomPBuf * const lCustomPBuf = reinterpret_cast<MyCustomPBuf *>(aPBuf);

    // Nothing to do for the pbuf itself: unref by the stack.
    // Give descriptors back to HW from end to start of chain.
    lCustomPBuf->mDescriptor->FreeDescriptors();
}


void RxDescriptor::FreeDescriptors() {

    if (!IsLastFrame()) {
        GetNext()->FreeDescriptors();
    }

    // Give descriptor back to HW from end to start of chain.
    GiveToHW();
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
