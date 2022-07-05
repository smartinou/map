#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Network interface DB.
//
// *******************************************************************************

//! \file
//! \brief Network interface DB class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

//! \brief Network configuration database.
class NetIFRec
    : public DBRec {
protected:
    class Token {};
    template<class T>
    friend std::shared_ptr<T> DBRec::Create();

public:
    explicit NetIFRec(Token /* Dummy */);

    // DBRec interface.
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
    // DBRec interface.
    unsigned int GetRecSize(void) const override;
    void Serialize(uint8_t * const aDataPtr) const override;
    void Deserialize(uint8_t const * const aDataPtr) override;
    void UpdateCRC(void) override;

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

    static DBRec::Magic constexpr sMagic = { 'N', 'E', 'T' };
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
