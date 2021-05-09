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

#include <map>

#include "LwIPDrv.h"

#include <driverlib/emac.h>

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
    void Wr(void) override;

    err_t EtherIFInit(struct netif * const aNetIF) override;
    void ISR(void) override;

    // Local interface.
    bool LowLevelTx(struct pbuf * const aPBuf, bool aIsFirstPBuf);
    struct pbuf *LowLevelRx(void);
    void FreePBuf(struct pbuf * const aPBuf);

    void EnableRxInt(void);

    class RxDescriptor
        : public tEMACDMADescriptor {
    public:
        RxDescriptor(struct pbuf * const aBuffer);
        ~RxDescriptor() {}

        struct pbuf *GetPBuf(void) const {return mPBuf;}
        void GiveToHW(void) {ui32CtrlStatus |= DES0_RX_CTRL_OWN;}
        bool IsHWOwned(void) const {return ui32CtrlStatus & DES0_RX_CTRL_OWN;}
        bool IsFrameValid(void) const { return !(ui32CtrlStatus & DES0_RX_STAT_ERR);}
        bool IsLastFrame(void) const {return ui32CtrlStatus & DES0_RX_STAT_LAST_DESC;}
        int32_t GetFrameLen(void) const {
            int32_t lLen = ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M;
            return (lLen >>= DES0_RX_STAT_FRAME_LENGTH_S);
        }

        RxDescriptor *GetNext(void) const {return static_cast<RxDescriptor *>(DES3.pLink);}
        void ChainTo(RxDescriptor * const aDescriptor) {DES3.pLink = aDescriptor;}
    
        // Operators.
        operator RxDescriptor*() {return this;}
        operator const RxDescriptor*() {return this;}
#if 0
        RxDescriptor& operator=(const RxDescriptor& d) {
            ui32CtrlStatus = d.ui32CtrlStatus;
            ui32Count = d.ui32Count;
            pvBuffer1 = d.pvBuffer1;
            DES3 = d.DES3;
            ui32ExtRxStatus = d.ui32ExtRxStatus;
            ui32Reserved = d.ui32Reserved;
            ui32IEEE1588TimeLo = d.ui32IEEE1588TimeLo;
            ui32IEEE1588TimeHi = d.ui32IEEE1588TimeHi;
            return *this;
        }
#endif
    private:
        struct pbuf * const mPBuf;
    };


    class RxDescriptorChain {
    public:
        RxDescriptorChain() {mMap.clear();}
        ~RxDescriptorChain();

        RxDescriptor &GetNext(void);
        RxDescriptor &GetDescriptor(struct pbuf * const aPBuf) {return *(mMap[aPBuf]);}
        RxDescriptor *Create(uint32_t aBaseAddr, unsigned int aChainSize, unsigned int aPktSize);

        struct pbuf * GetPBuf(RxDescriptor *aDescriptor) const {return aDescriptor->GetPBuf();}

    private:
        void Add(unsigned int aPktSize);

        RxDescriptor *mCurrentDescriptor = nullptr;
        std::map<struct pbuf *, RxDescriptor *> mMap;
    };

    RxDescriptorChain mRxDescriptors;

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
        TxDescriptor *PutPBufs(struct pbuf *aPBuf, bool aIsFirstPBuf);
        struct pbuf *GetPBufs(TxDescriptor * const aCurrent);
        void SaveHead(void) {mBkp = mHead;}
        void RestoreHead(void) {mHead = mBkp;}

    private:
        bool IsEmpty(void) const {return (mHead == mTail);}
        bool IsFull(void) const {return (mHead->GetNext() == mTail);}
        void Free(void) {}

        size_t mSize;
        TxDescriptor *mHead = nullptr;
        TxDescriptor *mTail = nullptr;
        TxDescriptor *mBkp = nullptr;
    };

    TxRingBuf mTxDescriptors;
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
