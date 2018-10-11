#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Limit base class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2016-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
class Limit {
 public:
  explicit Limit(unsigned int aLowerLimit,
		 unsigned int aUpperLimit);
  explicit Limit(unsigned int aLowerLimit,
		 unsigned int aUpperLimit,
		 unsigned int aVal);

  virtual ~Limit() {};

  virtual unsigned int Get(void) const { return mVal; }
  virtual void Set(unsigned int mVal);

  Limit& operator++ ();
  Limit  operator++ (int);

  Limit& operator-- ();
  Limit  operator-- (int);

 private:
  unsigned int mLowerLimit;
  unsigned int mUpperLimit;
  unsigned int mVal;
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