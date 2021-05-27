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
//#include "lwip/pbuf.h"

#include <driverlib/emac.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class RxDescriptor
    : public tEMACDMADescriptor {
public:
    RxDescriptor(uint8_t * const aBuffer, size_t aSize);
    ~RxDescriptor() {}

    void GiveToHW(void) {ui32CtrlStatus |= DES0_RX_CTRL_OWN;}
    bool IsHWOwned(void) const {return ui32CtrlStatus & DES0_RX_CTRL_OWN;}
    bool IsFrameValid(void) const { return !(ui32CtrlStatus & DES0_RX_STAT_ERR);}
    bool IsLastFrame(void) const {return ui32CtrlStatus & DES0_RX_STAT_LAST_DESC;}
    int32_t GetFrameLen(void) const {
        int32_t lLen = (ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M);
        return (lLen >>= DES0_RX_STAT_FRAME_LENGTH_S);
    }

    void *GetPayload(void) const {return pvBuffer1;}
    size_t GetPayloadSize(void) const {return ((ui32Count & DES1_RX_CTRL_BUFF1_SIZE_M) >> DES1_RX_CTRL_BUFF1_SIZE_S);}

    bool IsFirst(void) const {return ui32CtrlStatus & DES0_RX_STAT_FIRST_DESC;}
    bool IsLast(void) const {return ui32CtrlStatus & DES0_RX_STAT_LAST_DESC;}
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
#else
    RxDescriptor& operator=(const RxDescriptor& d) = delete;
#endif
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
