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

// Standard libraries.
#include <cstddef>

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

    void Reset(void)  {
        ui32CtrlStatus &= ~DES0_TX_CTRL_FIRST_SEG;
        ui32CtrlStatus &= ~DES0_TX_CTRL_LAST_SEG;
    }
    void SetPBuf(struct pbuf * const aBuffer) {mPBuf = aBuffer;}
    struct pbuf *GetPBufs(void) const {return mPBuf;}
    void SetPayload(void *aPayload) {pvBuffer1 = aPayload;}
    void FreePBuf(void);
    void GiveToHW(void) {ui32CtrlStatus |= DES0_TX_CTRL_OWN;}
    bool IsHWOwned(void) const {return ui32CtrlStatus & DES0_TX_CTRL_OWN;}
    void SetFrameStart(void) {ui32CtrlStatus |= DES0_TX_CTRL_FIRST_SEG;}
    bool IsFrameStart(void) const {return (ui32CtrlStatus & DES0_TX_CTRL_FIRST_SEG);}
    void SetFrameEnd(void) {ui32CtrlStatus |= DES0_TX_CTRL_LAST_SEG;}
    void SetLen(uint32_t aLen) {
        ui32Count &= (~(DES1_TX_CTRL_BUFF1_SIZE_M << DES1_TX_CTRL_BUFF1_SIZE_S));
        ui32Count |= ((aLen & DES1_TX_CTRL_BUFF1_SIZE_M) << DES1_TX_CTRL_BUFF1_SIZE_S);
    }
    bool IsErrSet(void) const {return (ui32CtrlStatus & (DES0_TX_STAT_ERR | DES0_TX_STAT_IPH_ERR));}

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
// Once the packets are out, PopPBufs() is called to remove and free the pbufs.
// It will move the Tail pointer forward for as many elements of pbuf chain.
// The descriptors are created in a circular chained-list.
class TxRingBuf {
public:
    TxRingBuf() {}
    ~TxRingBuf() {Free();}

    TxDescriptor *Create(std::size_t aSize);
    bool PushPBuf(struct pbuf * const aPBuf);
    bool PopPBuf(void);

private:
    TxDescriptor *GetNext(TxDescriptor * const aDescriptor = nullptr) const;
    bool IsEmpty(void) const {return (mHead == mTail);}
    bool IsFull(TxDescriptor * const aDescriptor) const {return (aDescriptor->GetNext() == mTail);}
    void SetHead(TxDescriptor * const aDescriptor) {mHead = aDescriptor;}

    bool PushPBuf(TxDescriptor * const aDescriptor, struct pbuf * const aPBuf, bool aIsFirstPBuf = false);
    bool PopPBuf(TxDescriptor const * const aCurrentDescriptor);
    void Free(void);

    std::size_t mSize = 0;
    TxDescriptor *mHead = nullptr;
    TxDescriptor *mTail = nullptr;
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
