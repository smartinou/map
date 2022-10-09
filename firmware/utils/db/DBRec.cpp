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

std::vector<DBRec::Ptr> DBRec::sRecList;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// DB static methods.
bool DBRec::IsDBSane() noexcept {
    return std::all_of(
        sRecList.cbegin(),
        sRecList.cend(),
        [](auto const & aRec) {return aRec->IsSane();}
    );
}


bool DBRec::IsDBDirty() noexcept {
    return std::any_of(
        sRecList.cbegin(),
        sRecList.cend(),
        [](auto const & aRec) {return aRec->IsDirty();}
    );
}


void DBRec::ResetDBDflt() noexcept {
    for (auto const& lRec : sRecList) {
        lRec->ResetDflt();
    }
}


auto DBRec::GetDBSize() noexcept -> size_t {

    size_t lDBSize{0};
    for (auto const & lRec : sRecList) {
        lDBSize += lRec->GetRecSize();
    }

    return lDBSize;
}


void DBRec::SerializeDB(uint8_t * aData) {
    for (auto const & lRec : sRecList) {
        lRec->Serialize(aData);
        auto const lSize = lRec->GetRecSize();
        aData += lSize;
    }
}


void DBRec::DeserializeDB(uint8_t const * aData) {
    for (auto const & lRec : sRecList) {
        lRec->Deserialize(aData);
        auto const lSize = lRec->GetRecSize();
        aData += lSize;
    }
}


void DBRec::StaticUpdateCRC() noexcept {
    for (auto const & lRec : sRecList) {
        lRec->UpdateCRC();
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void DBRec::AddRec(DBRec::Ptr aDBRec) {
    sRecList.push_back(std::move(aDBRec));
}


auto DBRec::ComputeCRC(std::span<uint8_t const> const &aSpan) noexcept -> uint8_t {

    uint8_t lCRC{0};
    for (auto const lByte : aSpan) {
        lCRC += lByte;
    }
    return ~lCRC;
}


bool DBRec::IsMagicGood(
    struct BaseRec const &aBaseRec,
    Magic const &aMagic
) noexcept
{
    return (aMagic == aBaseRec.mMagic);
}


bool DBRec::IsCRCGood(std::span<uint8_t const> const &aSpan) noexcept {
    // Span contains full record, including computed CRC.
    // Result should be 0x00.
    auto const lComputedCRC{ComputeCRC(aSpan)};
    return (lComputedCRC == 0);
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
