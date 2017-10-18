// *****************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *****************************************************************************

//! \file
//! \brief CoreLink peripheral SPI device class definition.
//! \ingroup corelink_peripherals

// *****************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Driver Library.
#include "hw_memmap.h"
#include "hw_ssi.h"
#include "hw_sysctl.h"
#include "hw_types.h"

#include "gpio.h"
//#include "ssi.h"

#include "SPI.h"

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

CoreLink::SPISlaveCfg::SPISlaveCfg() :
  mProtocol(MOTO_0),
  mBitRate(0),
  mDataWidth(8),
  mCSnGPIOPort(0),
  mCSnGPIOPin(0) {

  // Ctor body left intentionally empty.
}


void CoreLink::SPISlaveCfg::SetCSnGPIO(unsigned long aPort,
				       unsigned int  aPin) {

  mCSnGPIOPort = aPort;
  mCSnGPIOPin  = aPin;

  // Enable and configures the GPIO pin used for CSn.
  // The proper GPIO peripheral must be enabled using
  // SysCtlPeripheralEnable() prior to the following calls,
  // otherwise CPU will rise a HW fault.
  // [MG] COULD CALL SysCtlPeripheralEnable() BASED ON PORT.
  GPIOPinTypeGPIOOutput(mCSnGPIOPort, mCSnGPIOPin);
  GPIOPadConfigSet(mCSnGPIOPort,
		   mCSnGPIOPin,
		   GPIO_STRENGTH_2MA,
		   GPIO_PIN_TYPE_STD);

  // Put the CSn pin in deasserted state.
  //GPIOPinWrite(mCSnGPIOPort, mCSnGPIOPin, mCSnGPIOPin);
  DeassertCSn();
}


inline void CoreLink::SPISlaveCfg::AssertCSn(void) {

  GPIOPinWrite(mCSnGPIOPort, mCSnGPIOPin, 0);
}


inline void CoreLink::SPISlaveCfg::DeassertCSn(void) {

  GPIOPinWrite(mCSnGPIOPort, mCSnGPIOPin, mCSnGPIOPin);
}

#if 0
unsigned int CoreLink::SPISlaveCfg::ToNativeProtocol(void) const {

  switch (mProtocol) {
  case MOTO_0: return SSI_FRF_MOTO_MODE_0; break;
  case MOTO_1: return SSI_FRF_MOTO_MODE_1; break;
  case MOTO_2: return SSI_FRF_MOTO_MODE_2; break;
  case MOTO_3: return SSI_FRF_MOTO_MODE_3; break;
  case TI:     return SSI_FRF_TI;          break;
  case NMW:    return SSI_FRF_NMW;         break;
  }
}
#endif


CoreLink::SPIDev::SPIDev(uint32_t   aBaseAddr,
			 SSIPinCfg &aSPIMasterPinCfgRef) :
  PeripheralDev(aBaseAddr),
  mLastSPICfgPtr(0) {

  SSIDisable(aBaseAddr);
    aSPIMasterPinCfgRef.SetPins();
  SSIEnable(aBaseAddr);
}


CoreLink::SPIDev::~SPIDev() {

}


void CoreLink::SPIDev::RdData(uint8_t                aAddr,
			      uint8_t               *aDataPtr,
			      unsigned int           aLen,
			      CoreLink::SPISlaveCfg &aSPICfgRef) {

  SetCfg(aSPICfgRef);

  // Assert the assigned CSn pin.
  aSPICfgRef.AssertCSn();

  // -Send address.
  // -Push dummy data (0s) as many as requested to read.
  unsigned long lRxData = 0;
  SSIDataPut(GetBaseAddr(), aAddr);
  SSIDataGet(GetBaseAddr(), &lRxData);

  while (aLen > 0) {
    SSIDataPut(GetBaseAddr(), 0);
    SSIDataGet(GetBaseAddr(), &lRxData);
    *aDataPtr = static_cast<uint8_t>(lRxData);
    aDataPtr++;
    aLen--;
  }

  // Deassert the assigned CSn pin.
  aSPICfgRef.DeassertCSn();
}


void CoreLink::SPIDev::RdData(uint8_t               *aDataPtr,
			      unsigned int           aLen,
			      CoreLink::SPISlaveCfg &aSPICfgRef) {

  SetCfg(aSPICfgRef);

  // Assert the assigned CSn pin.
  aSPICfgRef.AssertCSn();

  // -Push dummy data (0s) as many as requested to read.
  unsigned long lRxData = 0;
  while (aLen > 0) {
    SSIDataPut(GetBaseAddr(), 0);
    SSIDataGet(GetBaseAddr(), &lRxData);
    *aDataPtr = static_cast<uint8_t>(lRxData);
    aDataPtr++;
    aLen--;
  }

  // Deassert the assigned CSn pin.
  aSPICfgRef.DeassertCSn();
}


void CoreLink::SPIDev::WrData(uint8_t                aAddr,
			      uint8_t const         *aDataPtr,
			      unsigned int           aLen,
			      CoreLink::SPISlaveCfg &aSPICfgRef) {

  SetCfg(aSPICfgRef);

  // Assert the assigned CSn pin.
  aSPICfgRef.AssertCSn();

  // -Send address.
  // -Push data as many as requested to write.
  // -Read dummy data to empty receive register.
  // -Wait for all bytes to transmit.
  unsigned int lBaseAddr = GetBaseAddr();
  unsigned long lRxData = 0;
  SSIDataPut(lBaseAddr, aAddr);
  SSIDataGet(lBaseAddr, &lRxData);

  while (aLen > 0) {
    SSIDataPut(lBaseAddr, *aDataPtr);
    SSIDataGet(lBaseAddr, &lRxData);
    aDataPtr++;
    aLen--;
  }

  // Deassert the assigned CSn pin.
  aSPICfgRef.DeassertCSn();
}


void CoreLink::SPIDev::WrData(uint8_t const         *aDataPtr,
			      unsigned int           aLen,
			      CoreLink::SPISlaveCfg &aSPICfgRef) {

  SetCfg(aSPICfgRef);

  // Assert the assigned CSn pin.
  aSPICfgRef.AssertCSn();

  // -Push data as many as requested to write.
  // -Read dummy data to empty receive register.
  // -Wait for all bytes to transmit.
  unsigned int lBaseAddr = GetBaseAddr();
  unsigned long lRxData = 0;
  while (aLen > 0) {
    SSIDataPut(lBaseAddr, *aDataPtr);
    SSIDataGet(lBaseAddr, &lRxData);
    aDataPtr++;
    aLen--;
  }

  // Deassert the assigned CSn pin.
  aSPICfgRef.DeassertCSn();
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void CoreLink::SPIDev::SetCfg(SPISlaveCfg &aSPISlaveCfgRef) {

  if (mLastSPICfgPtr != &aSPISlaveCfgRef) {
    // The config changed since the last SPI call.
    // Reconfigure with the newly specified config.
    // Set the new SPI config as the new one.
    uint32_t lBaseAddr = GetBaseAddr();
    SSIDisable(lBaseAddr);

    // Could check that data wiDth is in range [4, 16].
    SSIConfigSetExpClk(lBaseAddr,
		       SysCtlClockGet(),
		       aSPISlaveCfgRef.GetProtocol(), //ToNativeProtocol(),
		       SSI_MODE_MASTER,
		       aSPISlaveCfgRef.GetBitRate(),
		       aSPISlaveCfgRef.GetDataWidth());

    // Make this the "new" slave configuration and
    // enable SPI operations.
    mLastSPICfgPtr = &aSPISlaveCfgRef;
    SSIEnable(lBaseAddr);
  }
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
