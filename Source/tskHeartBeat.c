/*****************************************************************************/
/* Heartbeat LED handling task                                               */
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
#include <sysRTOS.h>
#include <drvStatLED.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define tskLED_DIM_STEP_COUNT 20

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint16_t l_dim_table[tskLED_DIM_STEP_COUNT] = {0, 0, 0, 5, 10, 15, 20, 30, 40, 50, 60, 65, 70, 75, 80, 85, 90, 93, 96, 100 };

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Heartbeat LED handling task function
void tskHeartbeat(void* in_argument)
{
	uint8_t led_dim_index = 0;
	int8_t led_dim_step = 1;

	while(1)
	{
		// set LED dimming
		drvStatLEDSetDim(l_dim_table[led_dim_index]);

		// update dim value index
		led_dim_index += led_dim_step;

		if(led_dim_index == 0 || led_dim_index == tskLED_DIM_STEP_COUNT-1)
			led_dim_step = -led_dim_step;

		// delay
		sysDelay(25);
	}
}
