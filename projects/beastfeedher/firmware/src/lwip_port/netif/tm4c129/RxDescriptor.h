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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard Libraries.
#include <cstdint>
#include <cstddef>

// LwIP.
#include <lwip/pbuf.h>
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
    [[nodiscard]] explicit constexpr RxDescriptor(uint8_t * const aBuffer, size_t aSize) noexcept
    //RxDescriptor::RxDescriptor(uint8_t * const aBuffer, size_t const aSize)
        : tEMACDMADescriptor {
            .ui32CtrlStatus{0}
            , .ui32Count{(DES1_RX_CTRL_CHAINED
                | ((static_cast<uint32_t>(aSize) << DES1_RX_CTRL_BUFF1_SIZE_S) & DES1_RX_CTRL_BUFF1_SIZE_M))}
            , .pvBuffer1{aBuffer}
            , .DES3{0}
            , .ui32ExtRxStatus{0}
            , .ui32Reserved{0}
            , .ui32IEEE1588TimeLo{0}
            , .ui32IEEE1588TimeHi{0}
        }
        , mCustomPBuf {
            .mPBuf {
                .pbuf{},
                .custom_free_function = RxDescriptor::Free
            }
            , .mDescriptor {this}
        }
    {
        // Ctor body.
    }

    void GiveToHW() noexcept {ui32CtrlStatus |= DES0_RX_CTRL_OWN;}
    [[nodiscard]] bool IsHWOwned() const noexcept {return ui32CtrlStatus & DES0_RX_CTRL_OWN;}
    [[nodiscard]] bool IsFrameValid() const noexcept {return !(ui32CtrlStatus & DES0_RX_STAT_ERR);}
    [[nodiscard]] bool IsFirstFrame() const noexcept {return ui32CtrlStatus & DES0_RX_STAT_FIRST_DESC;}
    [[nodiscard]] bool IsLastFrame() const noexcept {return ui32CtrlStatus & DES0_RX_STAT_LAST_DESC;}
    [[nodiscard]] auto GetFrameLen() const noexcept -> size_t {
        uint32_t const lLen = (ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M);
        return (lLen >> DES0_RX_STAT_FRAME_LENGTH_S);
    }

    [[nodiscard]] auto GetNext() const noexcept -> RxDescriptor* {return static_cast<RxDescriptor *>(DES3.pLink);}
    void ChainTo(RxDescriptor * const aDescriptor) noexcept {DES3.pLink = aDescriptor;}

    [[nodiscard]] auto GetAllocedPBuf(size_t aCumulatedLen) noexcept -> struct pbuf*;
    void FreeDescriptors() noexcept;

    // Operators.
    operator RxDescriptor*() {return this;}
    operator const RxDescriptor*() {return this;}
    RxDescriptor& operator=(const RxDescriptor& d) = delete;

private:
    [[nodiscard]] void *GetPayload() const noexcept {return pvBuffer1;}
    [[nodiscard]] auto GetPayloadSize() const noexcept -> size_t {
        return ((ui32Count & DES1_RX_CTRL_BUFF1_SIZE_M) >> DES1_RX_CTRL_BUFF1_SIZE_S);
    }

    static void Free(struct pbuf *aPBuf);

    // Custom PBuf structure, as described in zero-copy RX ethernet driver (LwIP).
    // Needs to reference the owner descriptor, since this struct can't be upcast to RxDescriptor,
    struct MyCustomPBuf {
        struct pbuf_custom mPBuf{};
        RxDescriptor *mDescriptor{};
    } mCustomPBuf;
};


class RxDescriptorChain final {
public:
    //RxDescriptorChain() {}
    ~RxDescriptorChain();

    RxDescriptor *GetNext();
    tEMACDMADescriptor *Create(uint32_t aBaseAddr, size_t aChainSize, size_t aBufferSize);

private:
    void Add(uint8_t * const aBuffer, size_t aSize);

    size_t mSize{0};
    uint8_t *mBuffer{nullptr};
    RxDescriptor *mHead{nullptr};
    RxDescriptor *mTail{nullptr};
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
