// *****************************************************************************
//
// Project: Utils
//
// Module: DB.
//
// *******************************************************************************

//! \file
//! \brief DB record class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// This project.
#include "DBRec.h"

// Standard Libraries.
#include <algorithm>

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

std::vector<DBRec::Ptr> DBRec::mRecList;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// DB static methods.
auto DBRec::IsDBSane() noexcept -> bool {
    return std::all_of(
        mRecList.cbegin(),
        mRecList.cend(),
        [](const auto& aRec) {return aRec->IsSane();}
    );
}


auto DBRec::IsDBDirty() noexcept -> bool {
    return std::any_of(
        mRecList.cbegin(),
        mRecList.cend(),
        [](const auto& aRec) {return aRec->IsDirty();}
    );
}


void DBRec::ResetDBDflt() noexcept {
    for (const auto& lRec : mRecList) {
        lRec->ResetDflt();
    }
}


auto DBRec::GetDBSize() noexcept -> size_t {

    size_t lDBSize = 0;
    for (const auto& lRec : mRecList) {
        lDBSize += lRec->GetRecSize();
    }

    return lDBSize;
}


void DBRec::SerializeDB(uint8_t * aData) {
    for (const auto& lRec : mRecList) {
        lRec->Serialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }
}


void DBRec::DeserializeDB(uint8_t const * aData) {
    for (const auto& lRec : mRecList) {
        lRec->Deserialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }
}


void DBRec::StaticUpdateCRC() noexcept {
    for (const auto& lRec : mRecList) {
        lRec->UpdateCRC();
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void DBRec::AddRec(DBRec::Ptr aDBRec) {
    mRecList.push_back(aDBRec);
}


auto DBRec::ComputeCRC(std::span<uint8_t const> const &aSpan) noexcept -> uint8_t {

    uint8_t lCRC = 0;
    for (auto const lByte : aSpan) {
        lCRC += lByte;
    }
    return ~lCRC;
}


auto DBRec::IsMagicGood(
    struct BaseRec const &aBaseRec,
    Magic const &aMagic
) noexcept -> bool
{
    return (aMagic == aBaseRec.mMagic);
}


auto DBRec::IsCRCGood(std::span<uint8_t const> const &aSpan) noexcept -> bool {
    // Span contains full record, including computed CRC.
    // Result should be 0x00.
    auto const lComputedCRC = ComputeCRC(aSpan);
    return (lComputedCRC == 0);
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
