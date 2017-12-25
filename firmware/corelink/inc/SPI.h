#ifndef CORELINK_SPI_H_
#define CORELINK_SPI_H_
// *******************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *******************************************************************************

//! \file
//! \brief CoreLink peripheral SPI device class declaration.
//! \ingroup corelink_peripherals


// ******************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Driver Library.
#include "hw_types.h"
#include "ssi.h"

#include "CoreLinkPeripheralDev.h"

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class SPISlaveCfg {
 public:
  SPISlaveCfg();
  ~SPISlaveCfg() {}

  enum PROTOCOL_ENUM_TAG {
    MOTO_0 = SSI_FRF_MOTO_MODE_0,
    MOTO_1 = SSI_FRF_MOTO_MODE_1,
    MOTO_2 = SSI_FRF_MOTO_MODE_2,
    MOTO_3 = SSI_FRF_MOTO_MODE_3,
    TI     = SSI_FRF_TI,
    NMW    = SSI_FRF_NMW
  };
  
  typedef enum PROTOCOL_ENUM_TAG protocol_t;

  void SetProtocol(protocol_t aProtocol)     { mProtocol    = aProtocol;  }
  void SetBitRate(unsigned int aBitRate)     { mBitRate     = aBitRate;   }
  void SetDataWidth(unsigned int aDataWidth) { mDataWidth   = aDataWidth; }
  void SetCSnGPIO(unsigned long aPort, unsigned int aPin);

  protocol_t    GetProtocol(void)  const { return mProtocol;  }
  unsigned int  GetBitRate(void)   const { return mBitRate;   }
  unsigned int  GetDataWidth(void) const { return mDataWidth; }

  void AssertCSn(void);
  void DeassertCSn(void);

 private:
  protocol_t    mProtocol;
  unsigned long mBitRate;
  unsigned long mDataWidth;
  
  unsigned long mCSnGPIOPort;
  unsigned char mCSnGPIOPin;
};

// [MG] STARTING TO WONDER IF THIS WHOLE CLASS IS USEFULL AT ALL.
// [MG] WOULDN'T SETTING THE PINS DIRECTLY IN SPIDev BE MORE EFFICIENT?
class SSIPinCfg {
 public:
  SSIPinCfg(unsigned int aSSIID) : mID(aSSIID) {}
  virtual ~SSIPinCfg() {}

  unsigned int GetID(void) const { return mID; }
  virtual void SetPins(void) const = 0;

 private:
  unsigned int mID;
};

 
class SPIDev : public PeripheralDev {
 public:
  // [MG] INSTEAD OF PASSING SSIPinCfg, MAYBE BETTER SIMPLY PASSING ID.
  // [MG] COULD BE A DEFAULT SetPin() METHOD TO SET SSI0 ON PORTA.
  // [MG] OR WOULDN'T PASSING A PTR TO FUNCTION BE SIMPLER TOO?
  // [MG] SIGNATURE COULD BE void (*)(void *)
  SPIDev(uint32_t aBaseAddr, SSIPinCfg &aSPIMasterPinCfgRef);
  ~SPIDev();

  void RdData(uint8_t      aAddr,
              uint8_t     *aDataPtr,
              unsigned int aLen,
              SPISlaveCfg &aSPICfgRef);

  void RdData(uint8_t     *aDataPtr,
              unsigned int aLen,
              SPISlaveCfg &aSPICfgRef);

  void WrData(uint8_t        aAddr,
              uint8_t const *aDataPtr,
              unsigned int   aLen,
              SPISlaveCfg   &aSPICfgRef);

  void WrData(uint8_t const *aDataPtr,
              unsigned int   aLen,
              SPISlaveCfg   &aSPICfgRef);

  uint8_t PushPullByte(uint8_t const aByte);
  uint8_t PushPullByte(uint8_t const aByte,
                       SPISlaveCfg  &aSPICfgRef);

 private:
  void SetCfg(SPISlaveCfg &aSPISlaveCfgRef);

  SPISlaveCfg *mLastSPICfgPtr;
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
}
#endif // CORELINK_SPI_H_
