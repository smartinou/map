#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar.
//
// *******************************************************************************

//! \file
//! \brief Feeding calendar class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "DBRec.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class NetIFRec
    : public DBRec {
public:
    NetIFRec();
    ~NetIFRec();

    // DBRec.
    bool IsSane(void) const override;
    void ResetDflt(void) override;

    // Extended object's interface.
    bool     UseDHCP(void) const;
    uint32_t GetIPAddr(void) const;
    uint32_t GetSubnetMask(void) const;
    uint32_t GetGWAddr(void) const;

    void SetUseDHCP(bool aUseHDCP);
    void SetIPAddr(uint32_t aIPAddr);
    void SetSubnetMask(uint32_t aSubnetMask);
    void SetGWAddr(uint32_t aGWAddr);

private:
    // DBRec.
    unsigned int GetRecSize(void) const override;
    void Serialize(uint8_t * const aDataPtr) const override;
    void Deserialize(uint8_t const * const aDataPtr) override;

    struct RecData {
        BaseRec mBase;
        uint8_t  mUseDHCP;
        uint8_t  mUseIPv6;
        uint32_t mIPAddr;
        uint8_t  mIPv6Addr[16];
        uint32_t mSubnetMask;
        uint32_t mGWAddr;
    };

    struct RecData mRec;

    static char constexpr sMagic[3] = { 'N', 'E', 'T' };
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
