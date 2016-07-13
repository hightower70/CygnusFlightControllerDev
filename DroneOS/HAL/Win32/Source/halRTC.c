/*****************************************************************************/
/* Real-time clock hal interface                                             */
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
#include <windows.h>
#include <halRTC.h>

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
	SYSTEMTIME time;

	GetLocalTime(&time);

	// time
	out_datetime->Hour = (uint8_t)time.wHour;
	out_datetime->Minute = (uint8_t)time.wMinute;
	out_datetime->Second = (uint8_t)time.wSecond;

	// date
	out_datetime->Year = (uint16_t)time.wYear;
	out_datetime->Month = (uint8_t)time.wMonth;
	out_datetime->Day = (uint8_t)time.wDay;

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
