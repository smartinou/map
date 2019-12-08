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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <stddef.h>
#include <stdint.h>
#include <vector>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declarations


//! \brief Brief description.
//! Details follow...
//! ...here.
class DBRec {
public:
    bool IsDirty(void) const { return mIsDirty; }

    virtual bool IsSane(void) const = 0;
    virtual void ResetDflt(void) = 0;

    // DB static methods.
    static bool IsDBSane(void);
    static bool IsDBDirty(void);
    static void ResetDBDflt(void);
    static size_t GetDBSize(void);
    static size_t GetDBRecCount(void) { return mRecList.size(); }
    static void SerializeDB(uint8_t * aData);
    static void DeserializeDB(uint8_t const * aData);

protected:
    struct BaseRec {
        uint8_t mCRC;
        char mMagic[3];
    };

    DBRec();
    virtual ~DBRec();

    void SetIsDirty(void) { mIsDirty = true; }
    bool IsMagicGood(struct BaseRec const * const aBaseRec, char const aMagic[]) const;
    bool IsCRCGood(uint8_t const * const aData, size_t aSize) const;
    uint8_t ComputeCRC(uint8_t const * const aData, size_t aSize) const;

private:
    virtual size_t GetRecSize(void) const = 0;
    virtual void Serialize(uint8_t * const aData) const = 0;
    virtual void Deserialize(uint8_t const * const aData) = 0;

    void AddRec(void);

    static std::vector <DBRec *> mRecList;
    bool mIsDirty;

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
