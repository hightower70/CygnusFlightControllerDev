/*****************************************************************************/
/* Date-time storage and handling functions                                  */
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
#include <sysDateTime.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define sysDT_NUMBER_OF_DAYS 7                     // Number of days in a week
#define sysDT_ORIGIN_YEAR    1970                  // the begin year

#define sysDT_SECONDS_IN_DAY 86400								// number of seconds in a day
#define sysDT_SECONDS_IN_HOUR 3600								// number of seconds in a hour
#define sysDT_SECONDS_IN_A_MINUTE 60							// number of seconds in a minute

/*****************************************************************************/
/* Local variables                                                           */
/*****************************************************************************/
//const sysString l_day_names[sysDT_NUMBER_OF_DAYS] = { (sysString)"Sunday", (sysString)"Monday", (sysString)"Tuesday", (sysString)"Wednesday", (sysString)"Thursday", (sysString)"Friday", (sysString)"Saturday" };
#ifdef sysDT_ENABLE_DAYLIGHT_SAVING
bool l_daylight_saving_time_changed = false;
#endif

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
#ifdef sysDT_ENABLE_DAYLIGHT_SAVING
static void sysGetDaylightSavingStartDate(sysDateTime* inout_datetime);
static void sysGetDaylightSavingEndDate(sysDateTime* inout_datetime);
#endif

/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Determies the current year is Leap Year or not
/// @return True if the current year is leap year
bool sysDateTimeIsLeapYear(uint16_t in_year)
{
	return ((((in_year % 4) == 0) && ((in_year % 100) != 0)) || ((in_year % 400) == 0));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns length of the current month (in days)
uint8_t sysDateTimeGetMonthLength(uint16_t in_year, uint8_t in_month)
{
	uint8_t month_length[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// check month
	if (in_month < 1 || in_month > 12)
		return 0;

	if (in_month == 2 && sysDateTimeIsLeapYear(in_year))
		return month_length[in_month - 1] + 1;
	else
		return month_length[in_month - 1];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts date, time to seconds (since Jan 1, sysDT_ORIGIN_YEAR)
/// @param in_datetime Data, time to convert
/// @return Number of seconds since Jan. 1 of origin year
uint32_t sysDateTimeConvertToSeconds(sysDateTime* in_datetime)
{
	uint16_t i;
	uint16_t days;

	// Calculate number of days spent so far from beginning of this year
	days = in_datetime->Day - 1;
	for (i = 1; i < in_datetime->Month; i++)
	{
		days += sysDateTimeGetMonthLength(in_datetime->Year, (uint8_t)i);
	}

	// calculate the number of days in the previous years
	for (i = sysDT_ORIGIN_YEAR; i < in_datetime->Year; i++)
	{
		days += sysDateTimeIsLeapYear(i) ? 366 : 365;
	}

	return ((uint32_t)days) * sysDT_SECONDS_IN_DAY + ((uint32_t)in_datetime->Hour) * sysDT_SECONDS_IN_HOUR + in_datetime->Minute * sysDT_SECONDS_IN_A_MINUTE + in_datetime->Second;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts seconds (since Jan 1, sysDT_ORIGIN_YEAR) to date and time 
/// @param out_datetime Data, time result of the conversion
/// @param in_seconds Number of seconds to convert (ellapsed since Jan 1 of origin year)
void sysDateTimeConvertFromSeconds(sysDateTime* out_datetime, uint32_t in_seconds)
{
	uint16_t year;
	uint8_t month, day;
	uint32_t month_length;
	uint8_t hour, minute;

	// calculate year
	year = sysDT_ORIGIN_YEAR;
	while (in_seconds > 365 * sysDT_SECONDS_IN_DAY)
	{
		if (sysDateTimeIsLeapYear(year))
		{
			if (in_seconds > 366 * sysDT_SECONDS_IN_DAY)
			{
				in_seconds -= 366 * sysDT_SECONDS_IN_DAY;
				year += 1;
			}
		}
		else
		{
			in_seconds -= 365 * sysDT_SECONDS_IN_DAY;
			year += 1;
		}
	}
	out_datetime->Year = year;

	// calculate month
	month = 1;
	while (true)
	{
		month_length = sysDateTimeGetMonthLength(year, month) * sysDT_SECONDS_IN_DAY;

		if (in_seconds >= month_length)
		{
			in_seconds -= month_length;
			month++;
		}
		else
			break;
	}
	out_datetime->Month = month;

	// store day
	day = 1;
	while (in_seconds >= sysDT_SECONDS_IN_DAY)
	{
		day++;
		in_seconds -= sysDT_SECONDS_IN_DAY;
	}
	out_datetime->Day = day;

	// calculate time
	hour = 0;
	while (in_seconds >= sysDT_SECONDS_IN_HOUR)
	{
		hour++;
		in_seconds -= sysDT_SECONDS_IN_HOUR;
	}
	out_datetime->Hour = hour;

	minute = 0;
	while (in_seconds >= sysDT_SECONDS_IN_A_MINUTE)
	{
		minute++;
		in_seconds -= sysDT_SECONDS_IN_A_MINUTE;
	}
	out_datetime->Minute = minute;
	out_datetime->Second = (uint8_t)in_seconds;

	// update day of week
	sysDateTimeUpdateDayOfWeek(out_datetime);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Normalizes Date (Days will be [1..MonthLength], Month will be [1..12], Hour [0..23], minutes [0..59], seconds [0..59] and DayOfWeek will be updated
/// @param inout_datetime Data, time struct to normalize
void sysDateTimeNormalize(sysDateTime* inout_datetime)
{
	// check seconds overflow
	while (inout_datetime->Second >= 60)
	{
		inout_datetime->Second -= 60;

		// increment minutes
		inout_datetime->Minute++;
	}

	// check for minutes overflow
	while (inout_datetime->Minute >= 60)
	{
		inout_datetime->Minute -= 60;

		// increment hour
		inout_datetime->Hour++;
	}

	// check hour overflow
	while (inout_datetime->Hour >= 24)
	{
		inout_datetime->Hour -= 24;

		// increment day
		inout_datetime->Day++;
	}

	// check for month overflow
	while (inout_datetime->Month > 12)
	{
		inout_datetime->Month -= 12;

		// increment year
		inout_datetime->Year++;
	}

	// check day overflow
	while (inout_datetime->Day > sysDateTimeGetMonthLength(inout_datetime->Year, inout_datetime->Month))
	{
		inout_datetime->Day -= sysDateTimeGetMonthLength(inout_datetime->Year, inout_datetime->Month);

		// increment month
		inout_datetime->Month++;

		// check for month overflow
		if (inout_datetime->Month > 12)
		{
			inout_datetime->Month -= 12;

			// increment year
			inout_datetime->Year++;
		}
	}

	// update day of week
	sysDateTimeUpdateDayOfWeek(inout_datetime);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Updates Day of Week on DateTime struct
/// @param inout_date Dat, time struct to modify day of week
void sysDateTimeUpdateDayOfWeek(sysDateTime* inout_date)
{
	uint8_t month;
	uint16_t year;

	year = inout_date->Year;
	month = inout_date->Month;

	if (month < 3)
	{
		month += 12;
		year--;
	}

	inout_date->DayOfWeek = (2 + inout_date->Day + (13 * month - 2) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Adds one second to the date, time struct
/// @param inout_datetime Date, time struct to modify
void sysDateTimeAddOneSecond(sysDateTime* inout_datetime)
{
#ifdef sysDT_ENABLE_DAYLIGHT_SAVING
	sysDateTime datetime;
#endif

	// increment seconds
	inout_datetime->Second++;

	// normalize date
	sysDateTimeNormalize(inout_datetime);

	// handle daylight saving
#ifdef sysDT_ENABLE_DAYLIGHT_SAVING
	// do this check only if second and minute is zero
	if (in_datetime->Second == 0 && in_datetime->Minute == 0)
	{
		// daylight saving start
		datetime = *in_datetime;
		sysGetDaylightSavingStartDate(&datetime);

		if (krnlIsEqualDateTime(in_datetime, &datetime))
			in_datetime->Hour++;

		// daylight saving end
		datetime = *in_datetime;
		sysGetDaylightSavingEndDate(&datetime);

		if (krnlIsEqualDateTime(in_datetime, &datetime))
		{
			if (!l_daylight_saving_time_changed)
			{
				in_datetime->Hour--;
				l_daylight_saving_time_changed = true;
			}
			else
				l_daylight_saving_time_changed = false;
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets and clears daylight saving time changed flag
bool sysDateTimeGetAndClearDaylightSavingTimeChangedFlag(void)
{
#ifdef sysDT_ENABLE_DAYLIGHT_SAVING
	bool retval = l_daylight_saving_time_changed;

	l_daylight_saving_time_changed = false;

	return retval;
#else
	return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Compares two DateTime structs ans returns true when they are equal
/// @param in_datetime1 DateTime struct for comparision
/// @param in_datetime2 DateTime struct for comparision
/// @return True if two date and time values are same
bool sysDateTimeIsEqual(sysDateTime* in_datetime1, sysDateTime* in_datetime2)
{
	return in_datetime1->Second == in_datetime2->Second &&
		in_datetime1->Minute == in_datetime2->Minute &&
		in_datetime1->Hour == in_datetime2->Hour &&
		in_datetime1->Day == in_datetime2->Day &&
		in_datetime1->Month == in_datetime2->Month &&
		in_datetime1->Year == in_datetime2->Year;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Compare datetimes
/// @param in_datetime1 Datatime1 to compare
/// @param in_datetime2 Datatime1 to compare
/// @return -1 if datetime1 < datetime2, 0 if datetime1==datetime2, 1 if datetime1 > datetime2
int8_t sysDateTimeCompare(sysDateTime* in_datetime1, sysDateTime* in_datetime2)
{
	// compare years
	if (in_datetime1->Year > in_datetime2->Year)
		return 1;

	if (in_datetime1->Year < in_datetime2->Year)
		return -1;

	// compare months
	if (in_datetime1->Month > in_datetime2->Month)
		return 1;

	if (in_datetime1->Month < in_datetime2->Month)
		return -1;

	// compare days
	if (in_datetime1->Day > in_datetime2->Day)
		return 1;

	if (in_datetime1->Day < in_datetime2->Day)
		return -1;

	// compare hours
	if (in_datetime1->Hour > in_datetime2->Hour)
		return 1;

	if (in_datetime1->Hour < in_datetime2->Hour)
		return -1;

	// compare minutes
	if (in_datetime1->Minute > in_datetime2->Minute)
		return 1;

	if (in_datetime1->Minute < in_datetime2->Minute)
		return -1;

	// compare seconds
	if (in_datetime1->Second > in_datetime2->Second)
		return 1;

	if (in_datetime1->Second < in_datetime2->Second)
		return -1;

	// time and dates are equal
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Compare time only part of datetime
/// @param in_datetime1 Datatime1 to compare
/// @param in_datetime2 Datatime1 to compare
/// @return -1 if time1 < time2, 0 if time1==time2, 1 if time1 > time2
int8_t sysDateTimeCompareTime(sysDateTime* in_datetime1, sysDateTime* in_datetime2)
{
	// compare hours
	if (in_datetime1->Hour > in_datetime2->Hour)
		return 1;

	if (in_datetime1->Hour < in_datetime2->Hour)
		return -1;

	// compare minutes
	if (in_datetime1->Minute > in_datetime2->Minute)
		return 1;

	if (in_datetime1->Minute < in_datetime2->Minute)
		return -1;

	// compare seconds
	if (in_datetime1->Second > in_datetime2->Second)
		return 1;

	if (in_datetime1->Second < in_datetime2->Second)
		return -1;

	// time and dates are equal
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Substracts Time only part of two datetimie structures without modifying date part
/// @param in_time Time to substract from
/// @param in_time_to_substract Time to substract
void sysDateTimeSubstractTime(sysDateTime* in_time, sysDateTime* in_time_to_substract)
{
	uint8_t carry = 0;

	// seconds
	if (in_time_to_substract->Second > in_time->Second)
	{
		in_time->Second = in_time->Second + 60 - in_time_to_substract->Second;
		carry = 1;
	}
	else
	{
		in_time->Second -= in_time_to_substract->Second;
		carry = 0;
	}

	// minutes
	if (in_time_to_substract->Minute + carry > in_time->Minute)
	{
		in_time->Minute = in_time->Minute + 60 - in_time_to_substract->Minute - carry;
		carry = 1;
	}
	else
	{
		in_time->Minute -= in_time_to_substract->Minute + carry;
		carry = 0;
	}

	// hours
	if (in_time_to_substract->Hour + carry > in_time->Hour)
	{
		in_time->Hour = in_time->Hour + 24 - in_time_to_substract->Hour - carry;
		carry = 1;
	}
	else
	{
		in_time->Hour -= in_time_to_substract->Hour + carry;
		carry = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets daylight saving start date
#ifdef sysDT_ENABLE_DAYLIGHT_SAVING
void 	sysGetDaylightSavingStartDate(sysDateTime* inout_datetime)
{
	// prepare datatime struct with the first day of the starting month of the daylight saving
	// year member must be valid
	inout_datetime->Month = 3; // daylight saving start at the last sunday of march
	inout_datetime->Day = 1; // first day of march
	inout_datetime->Hour = rtcDAYLIGHT_SAVING_START_HOUR;
	inout_datetime->Minute = 0;
	inout_datetime->Second = 0;

	krnlUpdateDayOfWeek(inout_datetime);

	// last sunday
	inout_datetime->Day = 31 - (inout_datetime->DayOfWeek + 2) % 7;
}
#endif

#ifdef sysDT_ENABLE_DAYLIGHT_SAVING
///////////////////////////////////////////////////////////////////////////////
// Get daylight saving end date
void sysGetDaylightSavingEndDate(sysDateTime* inout_datetime)
{
	// prepare datatime struct with the first day of the ending month of the daylight saving
	// year member must be valid
	inout_datetime->Month = 10; 	// daylight saving ends at last sunday of october
	inout_datetime->Day = 1;
	inout_datetime->Hour = rtcDAYLIGHT_SAVING_END_HOUR;
	inout_datetime->Minute = 0;
	inout_datetime->Second = 0;

	krnlUpdateDayOfWeek(inout_datetime);

	// last sunday
	inout_datetime->Day = 31 - (inout_datetime->DayOfWeek + 2) % 7;
}

#endif

/*****************************************************************************/
/* String conversion functions                                               */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Converty time to string (time only without date)
/// @param out_buffer Buffer receiving time as string
/// @param in_buffer_length Space in the buffer (number fo bytes)
/// @param in_pos Position in the buffer where converted string will start
/// @param in_datetime Datetime struct to convert (time only)
sysStringLength sysDateTimeConvertTimeToString(sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_datetime)
{
	// check buffer length
	if (in_pos + 9 >= in_buffer_length)
		return in_pos;

	// convert hour
	in_pos = sysWordToStringPos(out_buffer, in_buffer_length, in_pos, in_datetime->Hour, 2, 0, 0);
	if (out_buffer[in_pos - 2] == '0')
		out_buffer[in_pos - 2] = ' ';

	out_buffer[in_pos++] = ':';

	// convert minute
	in_pos = sysWordToStringPos(out_buffer, in_buffer_length, in_pos, in_datetime->Minute, 2, 0, 0);
	out_buffer[in_pos++] = ':';

	// convert seconds
	in_pos = sysWordToStringPos(out_buffer, in_buffer_length, in_pos, in_datetime->Second, 2, 0, 0);
	out_buffer[in_pos] = '\0';

	return in_pos;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts date to string (date only without time)
/// @param out_buffer Buffer receiving time as string
/// @param in_buffer_length Space in the buffer (number fo bytes)
/// @param in_pos Position in the buffer where converted string will start
/// @param in_datetime Datetime struct to convert (date only)
sysStringLength sysDateTimeConvertDateToString(sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_date, sysChar in_separator)
{
	// check buffer length
	if (in_pos + 11 >= in_buffer_length)
		return in_pos;

	// convert year
	in_pos = sysWordToStringPos(out_buffer, in_buffer_length, in_pos, in_date->Year, 4, 0, 0);
	out_buffer[in_pos++] = in_separator;

	// convert month
	in_pos = sysWordToStringPos(out_buffer, in_buffer_length, in_pos, in_date->Month, 2, 0, 0);
	out_buffer[in_pos++] = in_separator;

	// convert day
	in_pos = sysWordToStringPos(out_buffer, in_buffer_length, in_pos, in_date->Day, 2, 0, 0);
	out_buffer[in_pos] = '\0';

	return in_pos;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts string to Date
/// @param Buffer conatining the string
/// @param Buffer length
/// @param Position within the buffer
/// @param Success flag
/// @param DateTime struct to store result date
/// @param Separator character
void sysDateTimeConvertFromString(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator, sysDateTimeFormat in_format)
{
	uint16_t year;
	uint8_t month, day;

	//TODO: format handling
	sysSkipWhitespaces(in_buffer, in_buffer_length, inout_pos);
	sysStringToWord(in_buffer, in_buffer_length, inout_pos, inout_success, &year);
	sysCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	sysStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &month);
	sysCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	sysStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &day);

	// validate
	if (*inout_success)
	{
		// validate
		if (month < 1 && month > 12)
			*inout_success = false;
		else
		{
			if (day < 1 || day > sysDateTimeGetMonthLength(year, month))
				*inout_success = false;
		}
	}

	// update dateimte
	if (*inout_success)
	{
		out_datetime->Year = year;
		out_datetime->Month = month;
		out_datetime->Day = day;

		sysDateTimeUpdateDayOfWeek(out_datetime);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts string to Time
/// @param Buffer conatining the string
/// @param Buffer length
/// @param Position within the buffer
/// @param Success flag
/// @param DateTime struct to store result time
/// @param Separator character
void sysDateTimeStringToTime(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator)
{
	uint8_t hour, minute, second;

	//TODO: format handling
	sysSkipWhitespaces(in_buffer, in_buffer_length, inout_pos);
	sysStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &hour);
	sysCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	sysStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &minute);
	sysCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	sysStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &second);

	// validate
	if (*inout_success)
	{
		// validate
		if (hour > 23 || minute > 59 || second > 59)
			*inout_success = false;
	}

	// update datetime
	if (*inout_success)
	{
		out_datetime->Hour = hour;
		out_datetime->Minute = minute;
		out_datetime->Second = second;
	}
}
