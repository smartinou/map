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
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <stddef.h>

// This project.
#include "DBRec.h"

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

std::vector<DBRec *> DBRec::mRecList;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// DB static methods.
bool DBRec::IsDBSane(void) {
    for (const auto lRec : mRecList) {
        // Return after 1st corrupted record.
        if (!lRec->IsSane()) {
            return false;
        }
    }

    return true;
}


bool DBRec::IsDBDirty(void) {
    for (const auto lRec : mRecList) {
        // Return after 1st dirty record.
        if (lRec->IsDirty()) {
            return true;
        }
    }

    return false;
}


void DBRec::ResetDBDflt(void) {
    for (const auto lRec : mRecList) {
        lRec->ResetDflt();
    }
}


size_t DBRec::GetDBSize(void) {

    size_t lDBSize = 0;
    for (const auto lRec : mRecList) {
        lDBSize += lRec->GetRecSize();
    }

    return lDBSize;
}


void DBRec::SerializeDB(uint8_t * aData) {
    for (const auto lRec : mRecList) {
        lRec->Serialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }
}


void DBRec::DeserializeDB(uint8_t const * aData) {
    for (const auto lRec : mRecList) {
        lRec->Deserialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }
}


void DBRec::StaticUpdateCRC(void) {
    for (auto lRec : mRecList) {
        lRec->UpdateCRC();
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

DBRec::DBRec()
    : mIsDirty(false) {

    // Ctor body.
    AddRec();
}


DBRec::~DBRec() {
    // Dtor body left intentionally empty.
}


void DBRec::AddRec(void) {
    mRecList.push_back(this);
}


uint8_t DBRec::ComputeCRC(uint8_t const * const aData, size_t aSize) const {

    uint8_t lCRC = 0;
    for (size_t lIx = 1; lIx < aSize; lIx++) {
        lCRC += aData[lIx];
    }
    return ~lCRC;
}


bool DBRec::IsMagicGood(struct BaseRec const * const aBaseRec, char const aMagic[]) const {

    if ((aMagic[0] == aBaseRec->mMagic[0])
        && (aMagic[1] == aBaseRec->mMagic[1])
        && (aMagic[2] == aBaseRec->mMagic[2])
    ) {
        return true;
    }
    return false;
}


bool DBRec::IsCRCGood(uint8_t const * const aData, size_t aSize) const {
    uint8_t mRecCRC = reinterpret_cast<DBRec::BaseRec const * const>(aData)->mCRC;
    uint8_t lCRC = ComputeCRC(aData, aSize);
    if (lCRC == mRecCRC) {
        return true;
    }

    return false;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
