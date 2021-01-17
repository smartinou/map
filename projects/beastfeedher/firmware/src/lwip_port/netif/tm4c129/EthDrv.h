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
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <map>
#include <vector>

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

private:
    class Descriptor
        : public tEMACDMADescriptor {
    public:
        Descriptor() {}
        ~Descriptor() {}

        void *GetBuffer(void) const {return pvBuffer1;}
        void SetBuffer(struct pbuf * const aBuffer) {
            ui32CtrlStatus = 0;
            ui32Count = (DES1_RX_CTRL_CHAINED |
                (aBuffer->len << DES1_RX_CTRL_BUFF1_SIZE_S));
            pvBuffer1 = static_cast<struct pbuf * const>(aBuffer)->payload;
        }
        void GiveToHW(void) {ui32CtrlStatus |= DES0_RX_CTRL_OWN;}
        bool IsHWOwned(void) const {return ui32CtrlStatus & DES0_RX_CTRL_OWN;}
        bool IsFrameValid(void) const { return !(ui32CtrlStatus & DES0_RX_STAT_ERR);}
        bool IsLastFrame(void) const {return ui32CtrlStatus & DES0_RX_STAT_LAST_DESC;}
        int32_t GetFrameLen(void) const {
            int32_t lLen = ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M;
            return (lLen >>= DES0_RX_STAT_FRAME_LENGTH_S);
        }
        void ChainTo(Descriptor * const aDescriptor) {DES3.pLink = aDescriptor;}
    
        // Operators.
        operator Descriptor*() {return this;}
        operator const Descriptor*() {return this;}
#if 0
        Descriptor& operator=(const Descriptor& d) {
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
    };
#if 0
    // [MG] COMPOSITION-BASED CLASS: BAAAAAD!!!
    class RxDescriptor {
    public:
        RxDescriptor(void * const aBuffer);
        ~RxDescriptor() {}

        tEMACDMADescriptor &GetDescriptor(void) {return mDescriptor;}
        void *GetBuffer(void) const {return mDescriptor.pvBuffer1;}

        void GiveToHW(void);
        bool IsHWOwned(void) const;
        bool IsFrameValid(void) const;
        bool IsLastFrame(void) const;
        int32_t GetFrameLen(void) const;
        void ChainTo(RxDescriptor * const aRxDescriptor);

    private:
        tEMACDMADescriptor mDescriptor = {0};
    };
#endif

    class RxDescriptorChain {
    public:
        RxDescriptorChain(uint32_t aBaseAddr, unsigned int aChainSize, unsigned int aPktSize);
        ~RxDescriptorChain();

        Descriptor &GetNext(void);
        Descriptor &GetDescriptor(struct pbuf * const aPBuf) {return *mMap[aPBuf];}
        void Init(void);

    private:
        std::vector<Descriptor *> mRxDescriptors;
        //std::map<struct pbuf * const, tEMACDMADescriptor> mMap;
        std::map<struct pbuf *, Descriptor *> mMap;
        unsigned int mIndex = 0;
    };


    // LwIP Interface.
    err_t EtherIFInit(struct netif * const aNetIF) override;
    void ISR(void) override;

    void LowLevelTx(struct pbuf * const aPBuf) override;
    struct pbuf *LowLevelRx(void) override;
    void FreePBuf(struct pbuf * const aPBuf) override;

    void EnableRxInt(void) override;
    void EnableAllInt(void) override;
    bool IsTxEmpty(void) const override;

    tEMACDMADescriptor mRxDescriptorTbl[4];
    uint32_t mRxDescriptorIx = 0;

    RxDescriptorChain mRxDescriptors;
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
