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
    struct pbuf_custom *GetPBuf(void) {return &mPBuf;}
    RxDescriptor *GetDescriptor(void) const {return mDescriptor;}
    void SetDescriptor(RxDescriptor * const aDescriptor) {mDescriptor = aDescriptor;}
    struct pbuf *Alloced(void);

    static void Init(void);
    static CustomPBuf *New(RxDescriptor * const aDescriptor = nullptr);
    static void Delete(CustomPBuf * const aPBuf);

private:
    CustomPBuf() {}
    CustomPBuf(const CustomPBuf &) = delete;
    ~CustomPBuf() {}

    static void Free(struct pbuf *aPBuf);
    static void DoNothing(struct pbuf *aPBuf);
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
