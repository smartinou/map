#ifndef RTCC_AO_H_
#define RTCC_AO_H_
// *******************************************************************************
//
// Project: Active Object Library
//
// Module: RTCC QP Active Object.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2017-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "DBRec.h"
#include "CalendarRec.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class SPIDev;
class SPISlaveCfg;
class DS3234;


//! \brief Brief description.
//! Details follow...
//! ...here.
class RTCC_AO : public QP::QActive {
 public:
  RTCC_AO();

  static RTCC_AO     * const GetInstancePtr(void);
  static QP::QActive * const GetOpaqueAOInstancePtr(void);

  void ISRCallback(void);

  float GetTemperature(void) const;
  Time &GetTime(void);
  Date &GetDate(void);

 protected:
  static QP::QState Initial(RTCC_AO         * const aMePtr,
                            QP::QEvt  const * const aEvtPtr);
  static QP::QState Running(RTCC_AO         * const aMePtr,
                            QP::QEvt  const * const aEvtPtr);
 private:
  static unsigned int InitRTCC(RTCC_AO         * const aMePtr,
                               QP::QEvt  const * const aEvtPtr);
  static unsigned int InitDB(RTCC_AO         * const aMePtr,
                             QP::QEvt  const * const aEvtPtr);
  static unsigned int InitCalendar(RTCC_AO         * const aMePtr,
                                   QP::QEvt  const * const aEvtPtr);
  static unsigned int InitInterrupt(RTCC_AO         * const aMePtr,
                                    QP::QEvt  const * const aEvtPtr);

  static void SetNextCalendarEvt(RTCC_AO * const aMePtr);


  Time  mTime;
  Date  mDate;
  float mTemperature;

  CoreLink::SPISlaveCfg *mRTCSPISlaveCfgPtr;
  DS3234 *mDS3234Ptr;

  CalendarRec *mCalendarPtr;

  unsigned long mIntNbr;

  // The single instance of RTCC Active Object.
  static RTCC_AO *mInstancePtr;
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
#endif // RTCC_AO_H_
