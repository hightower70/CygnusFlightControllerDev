/*****************************************************************************/
/* Date-time storage and handling functions                                  */
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
#include <sysTimer.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// @brief Calculates ellapsed time in ticks (ms) since the given timestamp
// @param in_start_tick Timestemp for the start time
// @return Ellapsed tick from the specified time to current timestamp
uint32_t sysGetSystemTickSince(sysTick in_start_tick)
{
	uint32_t diff_time;

	diff_time = sysGetSystemTick() - in_start_tick;

	return diff_time;
}
