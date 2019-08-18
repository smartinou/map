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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
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

std::vector <DBRec *> DBRec::mRecList;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// DB static methods.
bool DBRec::IsDBSane(void) {
    for (std::vector<DBRec *>::iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        // Return after 1st "insane" record.
        if (!(*lDBIt)->IsSane()) {
            return false;
        }
    }

    return true;
}


bool DBRec::IsDBDirty(void) {
    for (std::vector<DBRec *>::const_iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        // Return after 1st dirty record.
        if ((*lDBIt)->IsDirty()) {
            return true;
        }
    }

    return false;
}


void DBRec::ResetDBDflt(void) {
    for (std::vector<DBRec *>::iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        (*lDBIt)->ResetDflt();
    }
}


size_t DBRec::GetDBSize(void) {

    size_t lDBSize = 0;
    for (std::vector<DBRec *>::const_iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        lDBSize += (*lDBIt)->GetRecSize();
    }

    return lDBSize;
}


void DBRec::SerializeDB(uint8_t * aData) {
    for (std::vector<DBRec *>::const_iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        (*lDBIt)->Serialize(aData);
        unsigned int lSize = (*lDBIt)->GetRecSize();
        aData += lSize;
    }
}


void DBRec::DeserializeDB(uint8_t const * aData) {
    for (std::vector<DBRec *>::iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        (*lDBIt)->Deserialize(aData);
        unsigned int lSize = (*lDBIt)->GetRecSize();
        aData += lSize;
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
