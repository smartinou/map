#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2016-2017, Martin Garon, All rights reserved.
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

// Forward declaration.
//class DBRec;


class NetIFRec
    : public DBRec {
public:
    NetIFRec();
    ~NetIFRec();

    // IDBRec.
    bool IsSane(void) override;
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
    unsigned int GetRecSize(void) const override;
    void Serialize(uint8_t * const aDataPtr) const override;
    void Deserialize(uint8_t const * const aDataPtr) override;

    struct RecStructTag {
        uint8_t  mCRC;
        char     mMagic[3];
        uint8_t  mUseDHCP;
        uint8_t  mUseIPv6;
        uint32_t mIPAddr;
        uint8_t  mIPv6Addr[16];
        uint32_t mSubnetMask;
        uint32_t mGWAddr;
    };

    struct RecStructTag mRec;
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
