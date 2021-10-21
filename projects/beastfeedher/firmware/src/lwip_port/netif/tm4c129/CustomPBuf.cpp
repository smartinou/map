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

// LwIP.
#include <lwip/memp.h>

// This project.
#include "RxDescriptor.h"
#include "CustomPBuf.h"

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

static constexpr uint16_t sRxPoolSize = 8; // CHECK THIS SIZE IS MATCHING RX BUFS IN RING.
LWIP_MEMPOOL_DECLARE(RX_POOL, sRxPoolSize, sizeof(CustomPBuf), "Zero-copy RX PBUF pool");

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void CustomPBuf::Init(void) {
    LWIP_MEMPOOL_INIT(RX_POOL);
}


struct pbuf *CustomPBuf::New(RxDescriptor * const aDescriptor, size_t aCumulatedLen) {
    // Get a CustomPBuf from the pool.
    CustomPBuf * const lCustomPBuf = static_cast<CustomPBuf * const>(LWIP_MEMPOOL_ALLOC(RX_POOL));
    struct pbuf *lPBuf = nullptr;
    if (lCustomPBuf != nullptr) {
        lCustomPBuf->SetDescriptor(aDescriptor);
        lPBuf = lCustomPBuf->Alloced(aCumulatedLen);

        // Assign free function.
        lCustomPBuf->mPBuf.custom_free_function = CustomPBuf::Free;
    } else {
        // Duh! it's null! why?
        //lCustomPBuf->mPBuf.custom_free_function = CustomPBuf::DoNothing;
    }

    return lPBuf; //lCustomPBuf;
}

// MOVE THIS BELOW.
void CustomPBuf::Delete(CustomPBuf * const aPBuf) {
    // Return to pool of PBuf;
    LWIP_MEMPOOL_FREE(RX_POOL, aPBuf);
}

// MOVE THIS BELOW.
struct pbuf *CustomPBuf::Alloced(size_t aCumulatedLen) {
    if (mDescriptor != nullptr) {
        struct pbuf *lPBuf = pbuf_alloced_custom(
            PBUF_RAW,
            mDescriptor->GetFrameLen() - aCumulatedLen,
            PBUF_REF,
            &mPBuf,
            mDescriptor->GetPayload(),
            mDescriptor->GetPayloadSize());
        return lPBuf;
    }

    return nullptr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void CustomPBuf::Free(struct pbuf *aPBuf) {

    // Downcast to a PBuf object.
    CustomPBuf * const lPBuf = reinterpret_cast<CustomPBuf *>(aPBuf);
    RxDescriptor * const lDescriptor = lPBuf->GetDescriptor();

    // Return this PBuf to the pool. This will free it even if a full chain.
    Delete(lPBuf);

    // Give desriptors back to HW from end to start of chain.
    FreeDescriptor(lDescriptor);
}

#if 0
void CustomPBuf::DoNothing(struct pbuf *aPBuf) {
    static_cast<void>(aPBuf);
}
#endif

void CustomPBuf::FreeDescriptor(RxDescriptor * const aDescriptor) {

    if (aDescriptor != nullptr) {
        if (!aDescriptor->IsLast()) {
            FreeDescriptor(aDescriptor->GetNext()); //SHOULDN'T THIS BE A RXRING FUNCTION?
        }

        // Give descriptor back to HW from end to start of chain.
        aDescriptor->GiveToHW();
    }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
