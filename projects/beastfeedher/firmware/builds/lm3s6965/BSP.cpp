// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Main entry point.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"
#include "qep.h"
#include "qf.h"

// CMSIS Library.
#include "lm3s_cmsis.h"

// TI Library.
#include "hw_types.h"
#include "hw_ints.h"
#include "systick.h"
#include "uartstdio.h"

#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"

// Common Library.
#include "Button.h"

#include "DS3234.h"
#include "SSD1329.h"
#include "SPI.h"

// This application.
#include "BeastFeedHerMgr.h"
#include "BSP.h"
#include "RTCC_AO.h"

Q_DEFINE_THIS_FILE

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
// DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
//
enum KernelUnawareISRs { // see NOTE00
    // ...
    MAX_KERNEL_UNAWARE_CMSIS_PRI  // keep always last
};

// "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI <= QF_AWARE_ISR_CMSIS_PRI);
#if 1
enum KernelAwareISRs {
    SYSTICK_PRIO = QF_AWARE_ISR_CMSIS_PRI, // see NOTE00
    GPIOPORTA_PRIO,
    GPIOPORTE_PRIO,
    GPIOPORTF_PRIO,
    // ...
    MAX_KERNEL_AWARE_CMSIS_PRI // keep always last
};
// "kernel-aware" interrupts should not overlap the PendSV priority
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));
#endif

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

class LM3S6965SSIPinCfg : public SSIPinCfg {
 public:
  LM3S6965SSIPinCfg(unsigned int aSPIID) : SSIPinCfg(aSPIID) {}
  ~LM3S6965SSIPinCfg() {}

  void SetPins(void) const;
};

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

static CoreLink::SPIDev *sSPIDevPtr;

// Extern declarations.
extern BeastFeedHerMgr *gMain_BeastFeedHerMgrPtr;
extern QP::QActive     *gMain_BeastFeedHerMgrAOPtr;

// Button objects.
Button *sFeedButtonPtr                      =   static_cast<Button *>(0);
Button *sNavButtonArray[BSP_NAV_BUTTON_QTY] = { static_cast<Button *>(0) };

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

unsigned int BSPGPIOPortToInt(unsigned long aGPIOPort) {

  switch (aGPIOPort) {
  default:
  case GPIO_PORTA_BASE: return INT_GPIOA;
  case GPIO_PORTB_BASE: return INT_GPIOB;
  case GPIO_PORTC_BASE: return INT_GPIOC;
  case GPIO_PORTD_BASE: return INT_GPIOD;
  case GPIO_PORTE_BASE: return INT_GPIOE;
  case GPIO_PORTF_BASE: return INT_GPIOF;
  case GPIO_PORTG_BASE: return INT_GPIOG;
  case GPIO_PORTH_BASE: return INT_GPIOH;    
  }

  return 0;
}


CoreLink::SPIDev * BSPInit(void) {

  // NOTE: SystemInit() already called from the startup code
  //  but SystemCoreClock needs to be updated
  //
  // [MG] Test if SystemCoreClockUpdate() if the same than SysCtlClockSet().
  //SystemCoreClockUpdate();
  SysCtlClockSet(SYSCTL_SYSDIV_1
                 | SYSCTL_USE_OSC
                 | SYSCTL_OSC_MAIN
                 | SYSCTL_XTAL_8MHZ);


  // Enable the clock to the peripherals used by the application.
  // Initialize SPI Master.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
  SSIPinCfg *lSPIMasterPinCfgPtr = new LM3S6965SSIPinCfg(0);
  sSPIDevPtr = new CoreLink::SPIDev(SSI0_BASE, *lSPIMasterPinCfgPtr);

    // Create all required buttons.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  sFeedButtonPtr = new Button(GPIO_PORTF_BASE, GPIO_PIN_1, 0U);

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  sNavButtonArray[BSP_NAV_BUTTON_UP]     = new Button(GPIO_PORTE_BASE, GPIO_PIN_0, 0U);
  sNavButtonArray[BSP_NAV_BUTTON_DOWN]   = new Button(GPIO_PORTE_BASE, GPIO_PIN_1, 0U);
  sNavButtonArray[BSP_NAV_BUTTON_LEFT]   = new Button(GPIO_PORTE_BASE, GPIO_PIN_2, 0U);
  sNavButtonArray[BSP_NAV_BUTTON_RIGHT]  = new Button(GPIO_PORTE_BASE, GPIO_PIN_3, 0U);
  sNavButtonArray[BSP_NAV_BUTTON_SELECT] = new Button(GPIO_PORTF_BASE, GPIO_PIN_1, 0U);

#if 0
  // Initialize the OLED display.
  // Create an SPI slave configuration for the OLED display.
  CoreLink::SPISlaveCfg *lOLEDSPISlaveCfgPtr = new CoreLink::SPISlaveCfg();

  lOLEDSPISlaveCfgPtr->SetProtocol(CoreLink::SPISlaveCfg::MOTO_2);
  lOLEDSPISlaveCfgPtr->SetBitRate(4000000);
  lOLEDSPISlaveCfgPtr->SetDataWidth(8);
  lOLEDSPISlaveCfgPtr->SetCSnGPIO(GPIO_PORTA_BASE, GPIO_PIN_3);

  // The GPIO peripheral must be enabled before use.
  // Otherwise this will generate a HW fault exception.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  SSD1329 *lSSD1329Ptr = new SSD1329(*lSPIDevPtr,
                                     *lOLEDSPISlaveCfgPtr,
                                     GPIO_PORTC_BASE,
                                     GPIO_PIN_7,
                                     GPIO_PORTC_BASE,
                                     GPIO_PIN_6,
                                     128,
                                     96);
  lSSD1329Ptr->Init();
  lSSD1329Ptr->DrawImg(&gcat_drawing_128x91_neg_ImgBuf[0][0],
                       0,
                       0,
                       gcat_drawing_128x91_neg_Width,
                       gcat_drawing_128x91_neg_Height);

  lSSD1329Ptr->DrawStr("Bonjour",
                       0,
                       0,
                       0x8);
#endif

  // Debug UART port.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTStdioInit(0);
  UARTprintf("Hello!\n");

  return sSPIDevPtr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// QF callbacks ==============================================================
void QP::QF::onStartup(void) {

  // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
  //SysTick_Config(SystemCoreClock / GAME::BSP_TICKS_PER_SEC);
  SysTickPeriodSet(800000);
  IntPrioritySet(FAULT_SYSTICK, 0x80); //SYSTICK_PRIO
  SysTickIntEnable();
  SysTickEnable();

  // assing all priority bits for preemption-prio. and none to sub-prio.
  //NVIC_SetPriorityGrouping(0U); from CMSIS
  IntPriorityGroupingSet(0U);
  IntPrioritySet(INT_GPIOA, 0x20); //GPIOPORTA_PRIO);
  IntPrioritySet(INT_GPIOE, 0x40); //GPIOPORTE_PRIO);
  IntPrioritySet(INT_GPIOF, 0x60); //GPIOPORTF_PRIO);

  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
  GPIOPadConfigSet(GPIO_PORTF_BASE,
                   GPIO_PIN_0,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);
  GPIOPinWrite(GPIO_PORTF_BASE,
               GPIO_PIN_0,
               GPIO_PIN_0);
}

//............................................................................
void QP::QV::onIdle(void) {  // called with interrupts disabled, see NOTE01

  // Toggle the user LED, ON then OFF.
  QF_INT_DISABLE();
  GPIOPinWrite(GPIO_PORTF_BASE,
               GPIO_PIN_0,
               GPIO_PIN_0);
  GPIOPinWrite(GPIO_PORTF_BASE,
               GPIO_PIN_0,
               0);
  QF_INT_ENABLE();
}


//............................................................................
void QP::QF::onCleanup(void) {
}


extern "C" void Q_onAssert(char const *module, int loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    //QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
    //NVIC_SystemReset();
}


void LM3S6965SSIPinCfg::SetPins(void) const {

  // Supports a single SSI device on port A.
  switch (GetID()) {
  case 0:
    // PA2: SSI0CLK
    // PA3: SSI0FSS
    // PA4: SSI0RX
    // PA5: SSI0TX
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeSSI(GPIO_PORTA_BASE,
                   GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTA_BASE,
                     GPIO_PIN_2 | GPIO_PIN_5,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);
    break;
  default:
    // Do nothing.
    break;
  }
}


extern "C" {

void SysTick_Handler(void);
void SysTick_Handler(void) {

  // Process time events for rate 0.
  // Publish to suscribers.
  static QP::QEvt const sTickEvt = { SIG_TIME_TICK, 0U, 0U };
  QP::QF::TICK_X(0U, &l_SysTick_Handler);
  QP::QF::PUBLISH(&sTickEvt, &l_SysTick_Handler);
}


// GPIO port A interrupt handler.
void GPIOPortA_IRQHandler(void);
void GPIOPortA_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTA_BASE, lIsMasked);
  if (GPIO_PIN_6 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTA_BASE, GPIO_PIN_6);
    RTCC_AO::GetInstancePtr()->ISRCallback();
  }
}


// GPIO port E interrupt handler.
void GPIOPortE_IRQHandler(void);
void GPIOPortE_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTE_BASE, lIsMasked);
  if (GPIO_PIN_0 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_0);
    sNavButtonArray[BSP_NAV_BUTTON_UP]->GenerateEvt();
  }

  if (GPIO_PIN_1 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
    sNavButtonArray[BSP_NAV_BUTTON_DOWN]->GenerateEvt();
  }

  if (GPIO_PIN_2 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_2);
    sNavButtonArray[BSP_NAV_BUTTON_LEFT]->GenerateEvt();
  }

  if (GPIO_PIN_3 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_3);
    sNavButtonArray[BSP_NAV_BUTTON_RIGHT]->GenerateEvt();
  }

  // Process state of other pins here if required.

}


// GPIO port F interrupt handler.
void GPIOPortF_IRQHandler(void);
void GPIOPortF_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTF_BASE, lIsMasked);
  if (GPIO_PIN_1 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTF_BASE, GPIO_PIN_1);
    sFeedButtonPtr->GenerateEvt(*gMain_BeastFeedHerMgrAOPtr);
  }

  // Process state of other pins here if required.
}
 
} // extern C

// *****************************************************************************
// NOTE00:
// The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
// ISR priority that is disabled by the QF framework. The value is suitable
// for the NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
// with the numerical values of priorities equal or higher than
// QF_AWARE_ISR_CMSIS_PRI) are allowed to call the QK_ISR_ENTRY/QK_ISR_ENTRY
// macros or any other QF/QK  services. These ISRs are "QF-aware".
//
// Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
// level (i.e., with the numerical values of priorities less than
// QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
// Such "QF-unaware" ISRs cannot call any QF/QK services. In particular they
// can NOT call the macros QK_ISR_ENTRY/QK_ISR_ENTRY. The only mechanism
// by which a "QF-unaware" ISR can communicate with the QF framework is by
// triggering a "QF-aware" ISR, which can post/publish events.
//
// NOTE01:
// The QV::onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV::onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
// NOTE02:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
