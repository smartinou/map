// *****************************************************************************
//
// Project: Utils.
//
// Module: Limit base class.
//
// *****************************************************************************

//! \file
//! \brief Limit class.
//! \ingroup utils

// *****************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "Limit.h"

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

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

Limit::Limit(unsigned int aLowerLimit,
	     unsigned int aUpperLimit)
  : mLowerLimit(aLowerLimit)
  , mUpperLimit(aUpperLimit)
  , mVal(aLowerLimit) {

  // Ctor body left intentionally empty.
}


Limit::Limit(unsigned int aLowerLimit,
	     unsigned int aUpperLimit,
	     unsigned int aVal)
  : mLowerLimit(aLowerLimit)
  , mUpperLimit(aUpperLimit)
  , mVal(aVal) {

  // Ctor body left intentionally empty.
  Set(aVal);
}


void Limit::Set(unsigned int aVal) {

  mVal = aVal;
  if (mVal < mLowerLimit) {
    mVal = mLowerLimit;
  } else if (mUpperLimit < mVal) {
    mVal = mUpperLimit;
  }    
}


// Prefix ++.
Limit& Limit::operator++ () {
  ++mVal;
  if (mVal > mUpperLimit) {
    mVal = mLowerLimit;
  }

  return *this;
}


// Postfix ++
Limit Limit::operator++ (int) {

  // make a copy for result.
  // Now use the prefix version to do the work.
  // return the copy (the old) value.
  Limit result(*this);
  ++(*this);
  return result;
}

Limit& Limit::operator-- () {
  if (mVal <= mLowerLimit) {
    mVal = mUpperLimit;
  } else {
    --mVal;
  }

  return *this;
}


Limit Limit::operator-- (int) {

  // make a copy for result.
  // Now use the prefix version to do the work.
  // return the copy (the old) value.
  Limit result(*this);
  --(*this);
  return result;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
