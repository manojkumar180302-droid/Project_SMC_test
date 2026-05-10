/**
 * @file    startup.c
 * @brief   Startup functions for configuring RCC System and Bus Clock
 * @author  SMC
 * @date    September 2025
 *
 * This file contains function(s) to configure the RCC (Reset and Clock Control)
 * register to achieve a given system and bus clock.
 *
 */
 
#include <stdint.h>
#include <stdbool.h>
#include "SMC_40CR.h"
#include "startup.h"

#define CLKSEL_SWITCH_MAX_TIME_IN_CYCLES    500UL
#define HSIRDY_MAX_TIME_IN_CYCLES           1400UL
#define HSERDY_MAX_TIME_IN_CYCLES           4200UL
#define RCC_PLL_READYTIMEOUT_CYCLES	    350UL

/* =======================================================================================================
 * Private Functions
 * ======================================================================================================*/

/*
 * Check if RCC Registers are locked
 */
static inline bool RCC_IsRegLocked(void)
{
	return (RCC_LOCK & RCC_LOCK_LOCK_STATUS) ? true : false;

}

/*
 * Unlock RCC Registers
 */
static inline void RCC_Unlock_Registers(void)
{
	RCC_UNL = 0x56DD;
	RCC_UNH = 0xA3B2;
}

/*
 * Lock RCC registers
 */
static inline void RCC_Lock_Registers(void)
{
	  RCC_LOCK |= RCC_LOCK_LOCK;
}

/*
 *  Set clock to default clock
 */
static inline bool RCC_Set_Default_Clock(void)
{
	bool isClkReady = true;
	uint32_t clockselTimeout = 0U;

	if((RCC_CR & RCC_CR_CLKSEL) != 0)
	{
		// Clock is not default clock. Wait for CLKSEL to show 0b00 indicating DEF_CLOCK is selected for 500 CPU cycle
	    RCC_CR |= RCC_CR_DEF_CLOCK;
		// Wait for selected clock to change
		do
		{
			isClkReady = ((RCC_CR & RCC_CR_CLKSEL) == 0);
			clockselTimeout++;
		} while (!isClkReady && (clockselTimeout < CLKSEL_SWITCH_MAX_TIME_IN_CYCLES));
	}

	return isClkReady;
}

/*
 * Disable Default clock and wait for selected clock to change
 */
static inline bool RCC_Disable_DEF_Clock(void)
{
	bool isClkReady = false;
	uint32_t clockselTimeout = 0U;

	// Disable DEF clock
	RCC_CR &= ~(RCC_CR_DEF_CLOCK);
	// Wait for selected clock to change
	do
	{
		isClkReady = ((RCC_CR & RCC_CR_CLKSEL) != 0);
		clockselTimeout++;
	} while (!isClkReady && (clockselTimeout < CLKSEL_SWITCH_MAX_TIME_IN_CYCLES));

	return isClkReady;
}

/*
 * Set System clock speed
 */
static inline void RCC_Set_PLL_SYS_Mul_Div(System_Clock_Speeds_t sysClkSpeed)
{

	// Start out by setting the SYS_DIV and BUS_DIV to 0. Also zero out the PLL register
	RCC_CR &= ~((RCC_CR_BUS_DIV) | (RCC_CR_SYS_DIV));
	RCC_PLLCFGR = 0;

	switch (sysClkSpeed)
	{
	case SYS_CLOCK_SPEED_160M:
		// Set a PLL Multiplication factor of 4 to get 160M (0b010 for MUL bits)
		RCC_PLLCFGR =  RCC_PLLCFGR_MUL_1;
		break;

	case SYS_CLOCK_SPEED_80M:
		// Set a PLL Multiplication factor of 2 to get 80M (0b001)
		RCC_PLLCFGR =  RCC_PLLCFGR_MUL_0;
		break;

	case SYS_CLOCK_SPEED_40M:
		// Set a PLL Multiplication factor of 1. Nothing to do!
		break;

	case SYS_CLOCK_SPEED_20M:
		// Set a PLL Division factor of 2 (0b001)
		RCC_PLLCFGR = RCC_PLLCFGR_DIV_0;
		break;

	case SYS_CLOCK_SPEED_10M:
		// Set a PLL Division factor of 4 (0b010)
		RCC_PLLCFGR = RCC_PLLCFGR_DIV_1;
		break;

	case SYS_CLOCK_SPEED_5M:
		// Set a PLL Division factor of 8 (0b100)
		RCC_PLLCFGR = RCC_PLLCFGR_DIV_2;
		break;

	case SYS_CLOCK_SPEED_2_5M:
		// Set a PLL Division factor of 8 and SYS_DIV value of 2 (0b01)
		RCC_PLLCFGR = RCC_PLLCFGR_DIV_2;
		RCC_CR |= RCC_CR_SYS_DIV_0;
		break;

	case SYS_CLOCK_SPEED_1_25M:
		// Set a PLL Division factor of 8 and SYS_DIV value of 4 (0b10)
		RCC_PLLCFGR = RCC_PLLCFGR_DIV_2;
		RCC_CR |= RCC_CR_SYS_DIV_1;
		break;

	case SYS_CLOCK_SPEED_UNDEFINED:
	default:
		// Should never have got here since we already checked this.
		// Should probably error out here
		break;
	}
}

/*
 * Set BUS Div
 */
static inline void RCC_Set_bus_DIV(uint32_t busDiv)
{
	  RCC_CR |= busDiv;
}

/* Turn on the PLL and wait for it to be ready. Max time to be ready
 *  is in CPU cycles so if we adhere to this value in a loop we'll be
 *  sure to have waited that long.
 */
static inline bool RCC_Set_PLL_On(void)
{
	bool isPllReady = false;
	uint32_t timeoutCycles = 0U;

	RCC_CR |= RCC_CR_PLLON;
	do
	{
		isPllReady = ((RCC_CR & RCC_CR_PLL_RDY) != 0);
		timeoutCycles++;
	} while (!isPllReady && (timeoutCycles < RCC_PLL_READYTIMEOUT_CYCLES));

	return isPllReady;
}

/* Internal clock!
* Set HSION to 1 and wait for HSIRDY to be 1
* Then set DEF_CLOCK to 0 and wait for CLKSEL to be HSI
*/
static inline bool RCC_Set_HSION(void)
{
	bool isClkReady = false;
	uint32_t timeoutCycles = 0U;

    RCC_CR |= RCC_CR_HSION;

    do
    {
      isClkReady = ((RCC_CR & RCC_CR_HSIRDY) != 0);
      timeoutCycles++;
    } while (!isClkReady && (timeoutCycles < HSIRDY_MAX_TIME_IN_CYCLES));

    return isClkReady;
}

/* Set External clock!
* Set HSEON to 1 and wait for HSERDY to be 1. If successfully set return true else false
*/
static inline bool RCC_Set_HSEON(void)
{
	bool isClkReady = false;
	uint32_t timeoutCycles = 0U;

    RCC_CR |= RCC_CR_HSEON;
    timeoutCycles = 0U;
    do
    {
      isClkReady = ((RCC_CR & RCC_CR_HSERDY) != 0);
      timeoutCycles++;
    } while (!isClkReady && (timeoutCycles < HSERDY_MAX_TIME_IN_CYCLES));

    return isClkReady;
}

/*
 * Check if HSI clock is enabled or not
 */
static inline bool RCC_IsHSIClockEnabled(void)
{
	// The clksel value should be 0b01 for HSI
	return ((RCC_CR & RCC_CR_CLKSEL) == RCC_CR_CLKSEL_0);
}

/** Check if HSE clock is enabled or not */
static inline bool RCC_IsHSEClockEnabled(void)
{
	// The clksel value should be 0b10 for HSE
	return ((RCC_CR & RCC_CR_CLKSEL) == RCC_CR_CLKSEL_1);
}

/* =======================================================================================================
 * Public Functions
 * ======================================================================================================*/

/**
 * @brief  Configures the system and bus clock config based on what is provided
 * @note   This function assumes a 40MHz starting clock. (Note: This is required 
 *         for HSE, and for HSI the internal clock is always 40MHz starting out).
 * @param sysClockSpeed Desired system clock speed enum
 * @param busClockDivider Desired bus clock divider
 * @param isHsiClock If true, then desired to use the internal HSI clock. If false, then use external HSE clock.
 * @retval 0 if success, -1 if timeout or error
 */
System_Clock_Error_t SetSystemAndBusClockConfig(System_Clock_Speeds_t sysClockSpeed, Bus_Clock_Dividers_t BusClockDivider, bool isHsiClock)
{
  uint32_t busClockRegVal = 0;

  // Validate the system clock speed
  if ((sysClockSpeed > SYS_CLOCK_SPEED_MAX_ENUM_VAL) || (sysClockSpeed == SYS_CLOCK_SPEED_UNDEFINED))
  {
    return SYS_CLOCK_INVALID_PARAM;
  }

  /// Set Bus Division bits as per bus dBusClockDivider value
  switch (BusClockDivider) {
    case BUS_CLOCK_DIVIDER_0:
      busClockRegVal = 0;
      break;

    case BUS_CLOCK_DIVIDER_2: // Divide by 2
      busClockRegVal = RCC_CR_BUS_DIV_0; // Set bits 7-8 to 0b01
      break;
    
    case BUS_CLOCK_DIVIDER_4: // Divide by 4
      busClockRegVal = RCC_CR_BUS_DIV_1; // Set bits 7-8 to 0b10
      break;

    default:
    	return SYS_CLOCK_INVALID_PARAM;
      break;
  }


  /*STEP 1*/

  /* If everything is valid up to this point, check if RCC registers are locked or unlocked
  	  if locked, unlock the RCC registers */
  if(RCC_IsRegLocked() == true)
  {
	  RCC_Unlock_Registers();
  }

  // If the current clock is not the default clock, then we need to switch it
  // before proceeding. If not able to set default clock, return error.
  if(RCC_Set_Default_Clock() == false)
  {
	  return SYS_CLOCK_DEFCLK_SWITCH_FAILED;
  }

  // Ensure the LOCK_STATUS bit shows unlocked,
  if (RCC_IsRegLocked() == true)
  {
    return SYS_CLOCK_UNLOCK_REG_FAILED;
  }

  /* STEP 2 */

  // Configure PLL and SYS Clock divider for the desired clock speed
  RCC_Set_PLL_SYS_Mul_Div(sysClockSpeed);

  /* STEP 3 & 4 */

  // Turn on the PLL and wait for it to be ready. If failed, return with error
  if(RCC_Set_PLL_On() == false)
  {
	  return SYS_CLOCK_PLL_ON_FAILED;
  }

  /* STEP 5 */

  // Set the BUS_DIV value so the bus clock is correct
  RCC_Set_bus_DIV(busClockRegVal);

  /* STEP 7 */
  // Check if using internal or external clock
  if (isHsiClock)
  {
	  // Set Internal Clock
	  if(RCC_Set_HSION() == false)
	  {
		  return SYS_CLOCK_HSI_ON_FAILED;
	  }

	  // Disable Default clock
	  if(RCC_Disable_DEF_Clock() == false)
	  {
		  return SYS_CLOCK_DEF_CLOCK_DISABLED_FAILED;
	  }

	  if(RCC_IsHSIClockEnabled() == false)
	  {
		  return SYS_CLOCK_SET_HSI_CLOCK_FAILED;
	  }

  } /* STEP 6*/
  else
  {
	  // Set External clock
	  if(RCC_Set_HSEON() == false)
	  {
		  return SYS_CLOCK_HSE_ON_FAILED;
	  }

	  // Disable Default clock
	  if(RCC_Disable_DEF_Clock() == false)
	  {
		  return SYS_CLOCK_DEF_CLOCK_DISABLED_FAILED;
	  }

	  if(RCC_IsHSEClockEnabled() == false)
	  {
		  return SYS_CLOCK_SET_HSE_CLOCK_FAILED;
	  }
  }

  /* STEP 8*/
  // Success, lock RCC register
  RCC_Lock_Registers();

  return SYS_CLOCK_NO_ERROR;
}
