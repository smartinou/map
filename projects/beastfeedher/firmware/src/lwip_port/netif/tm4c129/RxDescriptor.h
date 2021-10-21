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

#include <cstdint>
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

class RxDescriptor final
    : public tEMACDMADescriptor {
public:
    RxDescriptor(uint8_t * const aBuffer, size_t aSize);
    ~RxDescriptor() {}

    void GiveToHW(void) {ui32CtrlStatus |= DES0_RX_CTRL_OWN;}
    bool IsHWOwned(void) const {return ui32CtrlStatus & DES0_RX_CTRL_OWN;}
    bool IsFrameValid(void) const {return !(ui32CtrlStatus & DES0_RX_STAT_ERR);}
    bool IsFirstFrame(void) const {return ui32CtrlStatus & DES0_RX_STAT_FIRST_DESC;}
    bool IsLastFrame(void) const {return ui32CtrlStatus & DES0_RX_STAT_LAST_DESC;}
    size_t GetFrameLen(void) const {
        uint32_t lLen = (ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M);
        return (lLen >>= DES0_RX_STAT_FRAME_LENGTH_S);
    }

    RxDescriptor *GetNext(void) const {return static_cast<RxDescriptor *>(DES3.pLink);}
    void ChainTo(RxDescriptor * const aDescriptor) {DES3.pLink = aDescriptor;}

    struct pbuf *GetAllocedPBuf(size_t aCumulatedLen);
    void FreeDescriptors();

    // Operators.
    operator RxDescriptor*() {return this;}
    operator const RxDescriptor*() {return this;}
    RxDescriptor& operator=(const RxDescriptor& d) = delete;

private:
    void *GetPayload(void) const {return pvBuffer1;}
    size_t GetPayloadSize(void) const {return ((ui32Count & DES1_RX_CTRL_BUFF1_SIZE_M) >> DES1_RX_CTRL_BUFF1_SIZE_S);}

    static void Free(struct pbuf *aPBuf);

    // Custom PBuf structure, as described in zero-copy RX ethernet driver (LwIP).
    // Need to reference self, since this struct can't be upcast to RxDescriptor,
    // because of the tEMACDMADescriptor in front of it.
    // THAT FUDGING CAN BE ARRANGED, DAMMIT!
    // tEMACDMADescriptor mDescriptor;
    struct MyCustomPBuf {
        struct pbuf_custom mPBuf{};
        RxDescriptor *mDescriptor;
    } mCustomPBuf;
};


class RxDescriptorChain {
public:
    RxDescriptorChain() {}
    ~RxDescriptorChain();

    RxDescriptor *GetNext(void);
    RxDescriptor *Create(uint32_t aBaseAddr, size_t aChainSize, size_t aBufferSize);

private:
    void Add(uint8_t * const aBuffer, size_t aSize);

    size_t mSize = 0;
    uint8_t *mBuffer = nullptr;
    RxDescriptor *mHead = nullptr;
    RxDescriptor *mTail = nullptr;
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
