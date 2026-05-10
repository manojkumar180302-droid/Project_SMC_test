/**
 * @file    startup.h
 * @brief   Startup definitions
 * @author  SMC
 * @date    September 2025
 *
 * This file contains function definitions and enums to configure the RCC (Reset and Clock Control)
 * register to achieve a given system and bus clock.
 *
 */

#ifndef STARTUP__H
#define STARTUP__H

#include <stdint.h>
#include <stdbool.h>


typedef enum {
  SYS_CLOCK_SPEED_UNDEFINED = 0,
  SYS_CLOCK_SPEED_160M =      1,
  SYS_CLOCK_SPEED_80M =       2,
  SYS_CLOCK_SPEED_40M =       3,
  SYS_CLOCK_SPEED_20M =       4,
  SYS_CLOCK_SPEED_10M =       5,
  SYS_CLOCK_SPEED_5M =        6,
  SYS_CLOCK_SPEED_2_5M =      7, // 2.5M
  SYS_CLOCK_SPEED_1_25M =     8, // 1.25M
  SYS_CLOCK_SPEED_MAX_ENUM_VAL = SYS_CLOCK_SPEED_1_25M
} System_Clock_Speeds_t;

typedef enum {
	BUS_CLOCK_DIVIDER_0 = 0,
	BUS_CLOCK_DIVIDER_2 = 2,
	BUS_CLOCK_DIVIDER_4 = 4,
}Bus_Clock_Dividers_t;

typedef enum {
	SYS_CLOCK_INVALID_PARAM = -9,
	SYS_CLOCK_DEFCLK_SWITCH_FAILED = -8,
	SYS_CLOCK_UNLOCK_REG_FAILED = -7,
	SYS_CLOCK_PLL_ON_FAILED = -6,
	SYS_CLOCK_HSI_ON_FAILED = -5,
	SYS_CLOCK_HSE_ON_FAILED = -4,
	SYS_CLOCK_DEF_CLOCK_DISABLED_FAILED = -3,
	SYS_CLOCK_SET_HSI_CLOCK_FAILED = -2,
	SYS_CLOCK_SET_HSE_CLOCK_FAILED = -1,
	SYS_CLOCK_NO_ERROR = 0,
}System_Clock_Error_t;

System_Clock_Error_t SetSystemAndBusClockConfig(System_Clock_Speeds_t sysClkSped, Bus_Clock_Dividers_t BusClockDivider, bool isHsiClock);

#endif // STARTUP__H
