/*****************************************************************************/
/* Hardware Abstraction Layer Functions (general, system wide functions)     */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/


/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stm32f4xx_hal.h>
#include <drvHAL.h>


// Get the frequency (in Hz) of the source clock for the given timer.
// On STM32F405/407/415/417 there are 2 cases for how the clock freq is set.
// If the APB prescaler is 1, then the timer clock is equal to its respective
// APB clock.  Otherwise (APB prescaler > 1) the timer clock is twice its
// respective APB clock.  See DM00031020 Rev 4, page 115.
uint32_t drvHALTimerGetSourceFrequency(uint32_t tim_id)
{
	uint32_t source;

	if (tim_id == 1 || (8 <= tim_id && tim_id <= 11))
	{
		// TIM{1,8,9,10,11} are on APB2
		source = HAL_RCC_GetPCLK2Freq();
		if ((uint32_t)((RCC->CFGR & RCC_CFGR_PPRE2) >> 3) != RCC_HCLK_DIV1)
		{
			source *= 2;
		}
	}
	else
	{
		// TIM{2,3,4,5,6,7,12,13,14} are on APB1
		source = HAL_RCC_GetPCLK1Freq();
		if ((uint32_t)(RCC->CFGR & RCC_CFGR_PPRE1) != RCC_HCLK_DIV1)
		{
			source *= 2;
		}
	}

	return source;
}
