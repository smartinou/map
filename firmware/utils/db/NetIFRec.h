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
    template<class T, typename...Args>
    friend auto DBRec::Create(Args&&... aArgs) -> std::shared_ptr<T>;

public:
    explicit NetIFRec(UseCreateFunc const aDummy) noexcept
        : DBRec{aDummy} {}

    // DBRec interface.
    [[nodiscard]] auto IsSane() const noexcept -> bool override;
    void ResetDflt() noexcept override;

    // Extended object's interface.
    [[nodiscard]] auto UseDHCP() const noexcept -> bool;
    [[nodiscard]] auto GetIPAddr() const noexcept -> uint32_t;
    [[nodiscard]] auto GetSubnetMask() const noexcept -> uint32_t;
    [[nodiscard]] auto GetGWAddr() const noexcept -> uint32_t;

    void SetUseDHCP(bool aUseDHCP) noexcept;
    void SetIPAddr(uint32_t aIPAddr) noexcept;
    void SetSubnetMask(uint32_t aSubnetMask) noexcept;
    void SetGWAddr(uint32_t aGWAddr) noexcept;

private:
    // DBRec interface.
    [[nodiscard]] auto GetRecSize() const noexcept -> size_t override;
    void Serialize(uint8_t *aDataPtr) const override;
    void Deserialize(uint8_t const *aDataPtr) override;
    void UpdateCRC() noexcept override;

    static constexpr DBRec::Magic sMagic{ 'N', 'E', 'T' };

    struct RecData {
        struct BaseRec mBase{{}, {sMagic}};
        // CONSIDER STORING bool's, IPAddress'es.
        uint8_t mUseDHCP{1};
        uint8_t mUseIPv6{0};
        uint32_t mIPAddr{};
        uint8_t mIPv6Addr[16]{};
        uint32_t mSubnetMask{};
        uint32_t mGWAddr{};
    };

    struct RecData mRec;
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
