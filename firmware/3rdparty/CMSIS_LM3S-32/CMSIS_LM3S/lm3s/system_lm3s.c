/**************************************************************************//**
 * @file     system_ARMCM3.c
 * @brief    CMSIS Device System Source File for
 *           ARMCM3 Device Series
 * @version  V5.00
 * @date     07. September 2016
 ******************************************************************************/
/*
 * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include "lm3s_cmsis.h"
#include "lm3s_config.h"

/*----------------------------------------------------------------------------
  Local functions
 *---------------------------------------------------------------------------*/
__INLINE static uint32_t getOscClk (uint32_t xtal, uint32_t oscSrc);

//
// The following macros are used to distinguish among various Stellaris
// device classes and version numbers based on the SYSCTL->DID0 register.
//

#define CLASS_IS_SANDSTORM (((SYSCTL->DID0 & 0x70000000) == 0x00000000) || \
                            ((SYSCTL->DID0 & 0x70FF0000) == 0x10000000))

#define CLASS_IS_FURY       ((SYSCTL->DID0 & 0x70FF0000) == 0x10010000)
#define CLASS_IS_DUSTDEVIL  ((SYSCTL->DID0 & 0x70FF0000) == 0x10030000)
#define CLASS_IS_TEMPEST    ((SYSCTL->DID0 & 0x70FF0000) == 0x10040000)

#define REVISION_IS_A0      ((SYSCTL->DID0 & 0x0000FFFF) == 0x00000000)
#define REVISION_IS_A1      ((SYSCTL->DID0 & 0x0000FFFF) == 0x00000001)
#define REVISION_IS_A2      ((SYSCTL->DID0 & 0x0000FFFF) == 0x00000002)

#define REVISION_IS_B0      ((SYSCTL->DID0 & 0x0000FFFF) == 0x00000100)
#define REVISION_IS_B1      ((SYSCTL->DID0 & 0x0000FFFF) == 0x00000101)

#define REVISION_IS_C1      ((SYSCTL->DID0 & 0x0000FFFF) == 0x00000201)
#define REVISION_IS_C2      ((SYSCTL->DID0 & 0x0000FFFF) == 0x00000202)

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define XTALM       ( 6000000UL)            /* Main         oscillator freq */
#define XTALI       (12000000UL)            /* Internal     oscillator freq */
#define XTAL30K     (   30000UL)            /* Internal 30K oscillator freq */
#define XTAL32K     (   32768UL)            /* external 32K oscillator freq */

#define PLL_CLK    (200000000UL)
#define ADC_CLK     (PLL_CLK/25)
#define CAN_CLK     (PLL_CLK/50)

static const uint32_t sXtalTbl[] = {
    1000000,
    1843200,
    2000000,
    2457600,
    3579545,
    3686400,
    4000000,
    4096000,
    4915200,
    5000000,
    5120000,
    6000000,
    6144000,
    7372800,
    8000000,
    8192000,
    10000000,
    12000000,
    12288000,
    13560000,
    14318180,
    16000000,
    16384000
};

/*----------------------------------------------------------------------------
  Externals
 *----------------------------------------------------------------------------*/
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  extern uint32_t __Vectors;
#endif

/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = XTALI;   /*!< System Clock Frequency (Core Clock) */


/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
/**
 * Updates the SystemCoreClock global variable.
 *
 * @param  none
 * @return none
 *
 * @brief  Determine clock frequency according to clock register values.
 */
void SystemCoreClockUpdate(void) {

  uint32_t ulRCC;
  uint32_t ulRCC2;
  uint32_t ulPLL;
  uint32_t ulClk;

  // Read RCC and RCC2.  For Sandstorm-class devices (which do not have
  // RCC2), the RCC2 read will return 0, which indicates that RCC2 is
  // disabled (since the SYSCTL_RCC2_USERCC2 bit is clear).
  ulRCC  = SYSCTL->RCC;
  ulRCC2 = SYSCTL->RCC2;

  // Get the base clock rate.
  switch ((ulRCC2 & 0x80000000) ?
          (ulRCC2 & 0x00000070) :
          (ulRCC  & 0x00000030)) {
    // The main oscillator is the clock source.
    // Determine its rate from the crystal setting field.
    case 0x00: {
      ulClk = sXtalTbl[(ulRCC & 0x000007C0) >> 6];
      break;
    }

    // The internal oscillator is the source clock.
    case 0x10: {
      // See if this is a Sandstorm-class or Fury-class device.
      if (CLASS_IS_SANDSTORM) {
	// The internal oscillator on a Sandstorm-class device is 15 MHz +/- 50%.
        ulClk = 15000000;
      } else if ((CLASS_IS_FURY && REVISION_IS_A2) ||
		 (CLASS_IS_DUSTDEVIL && REVISION_IS_A0)) {
	// The internal oscillator on a rev A2 Fury-class device and
	// a rev A0 Dustdevil-class device is 12 MHz +/- 30%.
	ulClk = 12000000;
      } else {
	// The internal oscillator on all other devices is 16 MHz.
	ulClk = 16000000;
      }
      break;
    }

    // The internal oscillator divided by four is the source clock.
    case 0x20: {
      // See if this is a Sandstorm-class or Fury-class device.
      if (CLASS_IS_SANDSTORM) {
	// The internal oscillator on a Sandstorm-class device is 15 MHz +/- 50%.
	ulClk = 15000000 / 4;
      } else if ((CLASS_IS_FURY      && REVISION_IS_A2) ||
		 (CLASS_IS_DUSTDEVIL && REVISION_IS_A0)) {
	// The internal oscillator on a rev A2 Fury-class device and
	// a rev A0 Dustdevil-class device is 12 MHz +/- 30%.
	ulClk = 12000000 / 4;
      } else {
	// The internal oscillator on a Tempest-class device is 16MHz.
	ulClk = 16000000 / 4;
      }
      break;
    }

    // The internal 30 KHz oscillator is the source clock.
    case 0x30: {
      // The internal 30 KHz oscillator has an accuracy of +/- 30%.
      ulClk = 30000;
      break;
    }

    // The 4.19 MHz clock from the hibernate module is the clock source.
    case 0x60: {
      ulClk = 4194304;
      break;
    }

    // The 32 KHz clock from the hibernate module is the source clock.
    case 0x70: {
      ulClk = 32768;
      break;
    }

    // An unknown setting, so return a zero clock (that is, an unknown clock rate).
    default: {
      return;
    }
  }

  // See if the PLL is being used.
  if (((ulRCC2 & 0x80000000) && !(ulRCC2 & 0x00000800)) ||
      (!(ulRCC2 & 0x80000000) && !(ulRCC  & 0x00000800))) {
    // Get the PLL configuration.
    ulPLL = SYSCTL->PLLCFG;

    // See if this is a Sandstorm-class or Fury-class device.
    if (CLASS_IS_SANDSTORM) {
      // Compute the PLL output frequency based on its input frequency.
      // The formula for a Sandstorm-class devices is
      // "(xtal * (f + 2)) / (r + 2)".
      ulClk = ((ulClk * (((ulPLL & 0x00003FE0) >> 5) + 2)) /
               (((ulPLL & 0x000001F) >> 0) + 2));
    } else {
      // Compute the PLL output frequency based on its input frequency.
      // The formula for a Fury-class device is
      // "(xtal * f) / ((r + 1) * 2)".
      ulClk = ((ulClk * ((ulPLL & 0x00003FE0) >> 5)) /
               ((((ulPLL & 0x000001F) >> 0) + 1) * 2));
    }

    // See if the optional output divide by 2 is being used.
    if (ulPLL & 0x00004000) {
      ulClk /= 2;
    }

    // See if the optional output divide by 4 is being used.
    if (ulPLL & 0x00008000) {
      ulClk /= 4;
    }
  }

  // See if the system divider is being used.
  if (ulRCC & 0x00400000) {
    // Adjust the clock rate by the system clock divider.
    if (ulRCC2 & 0x80000000) {
      if ((ulRCC2   & 0x40000000) &&
	  (((ulRCC2  & 0x80000000) &&
	    !(ulRCC2 & 0x00000800)) ||
	   (!(ulRCC2 & 0x80000000) &&
	    !(ulRCC  & 0x00000800)))) {
	ulClk = ((ulClk * 2) / (((ulRCC2 & (0x1F800000 | 0x00400000))
				 >> (23 - 1)) + 1));
      } else {
	ulClk /= (((ulRCC2 & 0x1F800000) >> 23) + 1);
      }
    } else {
      ulClk /= (((ulRCC & 0x07800000) >> 23) + 1);
    }
  }

  SystemCoreClock = ulClk; /* Set the computed clock rate */
}


/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System and update the SystemCoreClock variable.
 */
void SystemInit (void) {
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  SCB->VTOR = (uint32_t) &__Vectors;
#endif

#if (CLOCK_SETUP)

  uint32_t volatile i;

  // Set default values.
  SYSCTL->RCC2 = 0x07802810; 
  SYSCTL->RCC  = 0x078E3AD1;

  // set value with BYPASS, PWRDN set, USESYSDIV reset.
  SYSCTL->RCC  = (RCC_Val  | (1UL << 11) | (1UL << 13)) & ~(1UL << 22);

  // Set value with BYPASS, PWRDN set.
  // Wait a while.
  SYSCTL->RCC2 = (RCC2_Val | (1UL << 11) | (1UL << 13));
  for (i = 0; i < 1000; i++) {
    __NOP();
  }

  // set value with BYPASS, USESYSDIV reset.
  SYSCTL->RCC  = (RCC_Val  | (1UL << 11)) & ~(1UL << 22);

  // Set value with BYPASS.
  // Wait a while.
  SYSCTL->RCC2 = (RCC2_Val | (1UL << 11));
  for (i = 0; i < 1000; i++) {
    __NOP();
  }

  // set value with BYPASS.
  SYSCTL->RCC  = (RCC_Val | (1UL << 11));

  if ((((RCC_Val  & (1UL << 13)) == 0) && ((RCC2_Val & (1UL << 31)) ==0)) ||
      (((RCC2_Val & (1UL << 13)) == 0) && ((RCC2_Val & (1UL << 31)) !=0))) {
    // Wait until PLL is locked.
    while ((SYSCTL->RIS & (1UL << 6)) != (1UL << 6)) {
      __NOP();
    }
  }

  // Set values and wait a while.
  SYSCTL->RCC  = (RCC_Val);
  SYSCTL->RCC2 = (RCC2_Val);
  for (i = 0; i < 1000; i++) {
    __NOP();
  }

#endif // CLOCK_SETUP

  SystemCoreClockUpdate();
}
