#pragma once
// *******************************************************************************
//
// Project: LwIP
//
// Module: TivaWare low-level Ethernet driver - support classes.
//
// *******************************************************************************

//! \file
//! \brief Ethernet driver support class.
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

//#include <cstdint>
//#include <cstddef>
// LwIP.
#include "lwip/pbuf.h"

#include <driverlib/emac.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class TxDescriptor
    : public tEMACDMADescriptor {
public:
    TxDescriptor();
    ~TxDescriptor() {}

    void SetPBuf(struct pbuf * const aBuffer) {mPBuf = aBuffer;}
    struct pbuf *GetPBuf(void) const {return mPBuf;}
    void SetPayload(void *aPayload) {pvBuffer1 = aPayload;}
    void FreePBuf(void);
    void GiveToHW(void) {ui32CtrlStatus |= DES0_TX_CTRL_OWN;}
    bool IsHWOwned(void) const {return ui32CtrlStatus & DES0_TX_CTRL_OWN;}
    void SetFrameStart(void) {ui32CtrlStatus |= DES0_TX_CTRL_FIRST_SEG;}
    void SetFrameEnd(void) {ui32CtrlStatus |= DES0_TX_CTRL_LAST_SEG;}
    void SetLen(uint32_t aLen) {
        ui32Count &= (~(DES1_TX_CTRL_BUFF1_SIZE_M << DES1_TX_CTRL_BUFF1_SIZE_S));
        ui32Count |= ((aLen & DES1_TX_CTRL_BUFF1_SIZE_M) << DES1_TX_CTRL_BUFF1_SIZE_S);
    }

    TxDescriptor *GetNext(void) const {return static_cast<TxDescriptor *>(DES3.pLink);}
    void ChainTo(TxDescriptor * const aDescriptor) {DES3.pLink = aDescriptor;}

private:
    struct pbuf *mPBuf = nullptr;
};


// Ring of Tx descriptors.
// Uses (=wastes) one spare descriptor to detect the full condition.
// PutPBufs attempts to attach (=put) a chain of pbufs in as many descriptors.
// The pbufs are given to HW for transmission.
// Overflow will cause NEWER pbufs (packets) to be lost.
// Once the packets are out, GetPBufs() is called to remove and free the pbufs.
// It will move the Tail pointer forward for as many elements of pbuf chain.
// The descriptors are created in a circular chained-list.
class TxRingBuf {
public:
    TxRingBuf() : mSize(0) {}
    ~TxRingBuf() {Free();}

    TxDescriptor *Create(size_t aSize);
    TxDescriptor *PutPBufs(struct pbuf *aPBuf, bool aIsFirstPBuf = true);
    TxDescriptor *GetPBufs(void);

private:
    bool IsEmpty(void) const {return (mHead == mTail);}
    bool IsFull(void) const {return (mHead->GetNext() == mTail);}
    void SaveHead(void) {mBkp = mHead;}
    void RestoreHead(void) {mHead = mBkp;}
    void Free(void);

    size_t mSize;
    TxDescriptor *mHead = nullptr;
    TxDescriptor *mTail = nullptr;
    TxDescriptor *mBkp = nullptr;
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
