#pragma once
// *******************************************************************************
//
// Project: Utils
//
// Module: DB.
//
// *******************************************************************************

//! \file
//! \brief DB record class.
//! \ingroup utils_db

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

// Standard Library.
#include <array>
#include <memory>
#include <span>
#include <cstdint>
#include <vector>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Base database class.
//! Register each object of DBRec into vector on ctor.
class DBRec
    : public std::enable_shared_from_this<DBRec> {
public:
    virtual ~DBRec() = default;

    [[nodiscard]] auto IsDirty() const -> bool {return mIsDirty;}
    [[nodiscard]] virtual auto IsSane() const -> bool = 0;
    virtual void ResetDflt() = 0;

    // DB static methods.
    static auto IsDBSane() noexcept -> bool;
    static auto IsDBDirty() noexcept -> bool;
    static void ResetDBDflt() noexcept;
    static auto GetDBSize() noexcept -> size_t;
    static auto GetDBRecCount() noexcept -> size_t {return mRecList.size();}
    static void SerializeDB(uint8_t * aData);
    static void DeserializeDB(uint8_t const * aData);
    static void StaticUpdateCRC() noexcept;
    static void ClearAllDB() noexcept {mRecList.clear();}

    template <typename T, typename...Args>
    [[nodiscard]] static auto Create(Args&&... aArgs) -> std::shared_ptr<T> {
        auto lRec = std::make_shared<T>(
            typename T::UseCreateFunc{},
            std::forward<Args>(aArgs)...
        );
        lRec->AddRec(lRec->shared_from_this());
        return lRec;
    }

protected:
    struct UseCreateFunc {
        explicit UseCreateFunc() = default;
    };
    explicit DBRec([[maybe_unused]] UseCreateFunc /* Dummy */) noexcept {}
    DBRec(DBRec const &) = delete;

    using Magic = std::array<char, 3>;
    struct BaseRec {
        uint8_t mCRC{};
        Magic mMagic{};
    };

    using Ptr = std::shared_ptr<DBRec>;

    static void AddRec(Ptr aDBRec);
    void SetIsDirty() noexcept {mIsDirty = true;}
    [[nodiscard]] static auto IsMagicGood(struct BaseRec const &aBaseRec, Magic const &aMagic) noexcept -> bool;
    [[nodiscard]] static auto IsCRCGood(std::span<uint8_t const> const &aSpan) noexcept -> bool;
    [[nodiscard]] static auto ComputeCRC(std::span<uint8_t const> const &aSpan) noexcept -> uint8_t;

private:
    [[nodiscard]] virtual auto GetRecSize() const -> size_t = 0;
    virtual void Serialize(uint8_t * aData) const = 0;
    virtual void Deserialize(uint8_t const * aData) = 0;
    virtual void UpdateCRC() = 0;

    static std::vector<Ptr> mRecList;
    bool mIsDirty{};
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
