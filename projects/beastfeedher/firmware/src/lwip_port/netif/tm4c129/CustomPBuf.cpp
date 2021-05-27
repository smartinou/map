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

static constexpr uint16_t sRxPoolSize = 8;
LWIP_MEMPOOL_DECLARE(RX_POOL, sRxPoolSize, sizeof(CustomPBuf), "Zero-copy RX PBUF pool");

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void CustomPBuf::Init(void) {
    LWIP_MEMPOOL_INIT(RX_POOL);
}


CustomPBuf *CustomPBuf::New(RxDescriptor * const aDescriptor) {
    // Get a PBuf from the pool.
    CustomPBuf * const lPBuf  = static_cast<CustomPBuf * const>(LWIP_MEMPOOL_ALLOC(RX_POOL));
    if (lPBuf != nullptr) {
        lPBuf->SetDescriptor(aDescriptor);

        // Assign free function.
        lPBuf->mPBuf.custom_free_function = CustomPBuf::Free;
    }

    return lPBuf;
}


void CustomPBuf::Delete(CustomPBuf * const aPBuf) {
    // Return to pool of PBuf;
    LWIP_MEMPOOL_FREE(RX_POOL, aPBuf);
}


struct pbuf *CustomPBuf::Alloced(void) {
    RxDescriptor * const lDescriptor = GetDescriptor();
    if (lDescriptor != nullptr) {
        struct pbuf *lPBuf = pbuf_alloced_custom(
            PBUF_RAW,
            lDescriptor->GetFrameLen(),
            PBUF_REF,
            GetPBuf(),
            lDescriptor->GetPayload(),
            lDescriptor->GetPayloadSize());
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


void CustomPBuf::FreeDescriptor(RxDescriptor * const aDescriptor) {

    if (aDescriptor != nullptr) {
        if (!aDescriptor->IsLast()) {
            FreeDescriptor(aDescriptor->GetNext());
        }

        // Give descriptor back to HW from end to start of chain.
        aDescriptor->GiveToHW();
    }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
