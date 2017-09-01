/*****************************************************************************/
/* Status LED driver functions                                               */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <halIODefinitions.h>
#include <stm32f4xx_hal.h>
#include <halHelpers.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Status LED driver initialization function
void drvStatLEDInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// Configure status LED pin
	GPIO_InitStruct.Pin = halLD3_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;

	HAL_GPIO_Init(halLD3_PORT, &GPIO_InitStruct);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Set status LED current dimming value
/// @param in_dim Dimming value [0..100]
void drvStatLEDSetDim(uint8_t in_dim)
{
	if(in_dim == 0)
		drvHAL_SetPinHigh(halLD3_PORT, halLD3_PIN);
	else
		drvHAL_SetPinLow(halLD3_PORT, halLD3_PIN);
}
