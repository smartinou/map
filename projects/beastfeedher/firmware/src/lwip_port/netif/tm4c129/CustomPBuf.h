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

// LwIP.
#include "lwip/pbuf.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class CustomPBuf {
public:
    static void Init(void);
    static struct pbuf *New(RxDescriptor * const aDescriptor, size_t aCumulatedLen);

private:
    CustomPBuf() {}
    CustomPBuf(const CustomPBuf &) = delete;
    ~CustomPBuf() {}

    RxDescriptor *GetDescriptor(void) const {return mDescriptor;}
    void SetDescriptor(RxDescriptor * const aDescriptor) {mDescriptor = aDescriptor;}
    struct pbuf *Alloced(size_t aCumulatedLen);

    static void Free(struct pbuf *aPBuf);
    //static void DoNothing(struct pbuf *aPBuf);
    static void Delete(CustomPBuf * const aPBuf);
    static void FreeDescriptor(RxDescriptor * const aDescriptor);

    struct pbuf_custom mPBuf{};
    RxDescriptor * mDescriptor = nullptr;
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
