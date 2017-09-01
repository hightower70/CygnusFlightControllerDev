/*****************************************************************************/
/* High resolution timer (1us) routines                                      */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysHighresTimer.h>

/*****************************************************************************/
/* Functions implemented in the HAL                                          */
/*****************************************************************************/
extern void halHighresTimerInit(void);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes high resolution timer
void sysHighresTimerInit(void)
{
	halHighresTimerInit();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Delay function (us resolution delay)
/// @param in_delay_us Delay in us
void sysHighresTimerDelay(uint32_t in_delay_us)
{
	sysHighresTimestamp start_time = sysHighresTimerGetTimestamp();
	sysHighresTimestamp diff_time;

	do
	{
		diff_time = sysHighresTimerGetTimestamp() - start_time;
	}	while( diff_time < in_delay_us );
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Get ellapsed time since a timestamp (in us)
/// @param in_start_time Start time timestamp
/// @return Ellapsed time in us
sysHighresTimestamp sysHighresTimerGetTimeSince(sysHighresTimestamp in_start_time)
{
	sysHighresTimestamp diff_time;

	diff_time = sysHighresTimerGetTimestamp() - in_start_time;

	return diff_time;
}

