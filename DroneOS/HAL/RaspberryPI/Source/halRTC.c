/*****************************************************************************/
/* Real-time clock hal interface (Linux)                                     */
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
#include <halRTC.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes real time clock
void halRTCInit(void)
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets current Date & Time
/// @param out_datetime Current datetime
void sysDateTimeGet(sysDateTime* out_datetime)
{
	time_t t;
	struct tm *local_time;

	// get time
	t = time(NULL);	
	local_time = localtime(&t);
	
	// time
	out_datetime->Hour = (uint8_t)local_time->tm_hour;
	out_datetime->Minute = (uint8_t)local_time->tm_min;
	out_datetime->Second = (uint8_t)local_time->tm_sec;

	// date
	out_datetime->Year = (uint16_t)local_time->tm_year + 1900;
	out_datetime->Month = (uint8_t)local_time->tm_mon;
	out_datetime->Day = (uint8_t)local_time->tm_mday;

	//  day of week
	sysDateTimeUpdateDayOfWeek(out_datetime);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets Date & Time
/// @param in_datetime Datetime to set
void sysDateTimeSet(sysDateTime* in_datetime)
{
	// empty
	sysUNUSED(in_datetime);
}
