/*****************************************************************************/
/* Date-time storage and handling functions                                  */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __sysDateTime_h
#define __sysDateTime_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <strString.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

// Date&Time
typedef struct
{
	uint8_t Hour;					// 0-23
	uint8_t Minute;				// 0-59
	uint8_t Second;				// 0-59

	uint16_t Year;
	uint8_t Month;				// 1-12
	uint8_t Day;					// 1-31

	uint8_t DayOfWeek;		// 0 - Sunday, 1 - Monday ... 6 - Saturday
} sysDateTime;

// DateTime format
typedef enum
{
	sysDateTimeFormat_YYYYMMDD,
} sysDateTimeFormat;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

bool sysDateTimeIsLeapYear(uint16_t in_year);
uint8_t sysDateTimeGetMonthLength(uint16_t in_year, uint8_t in_month);

uint32_t sysDateTimeConvertToSeconds(sysDateTime* in_datetime);
void sysDateTimeConvertFromSeconds(sysDateTime* out_datetime, uint32_t in_seconds);
void sysDateTimeNormalize(sysDateTime* inout_datetime);

void sysDateTimeUpdateDayOfWeek(sysDateTime* inout_date);

void sysDateTimeAddOneSecond(sysDateTime* inout_datetime);

bool sysDateTimeGetAndClearDaylightSavingTimeChangedFlag(void);

bool sysDateTimeIsEqual(sysDateTime* in_datetime1, sysDateTime* in_datetime2);
int8_t sysDateTimeCompare(sysDateTime* in_datetime1, sysDateTime* in_datetime2);
void sysDateTimeSubstractTime(sysDateTime* in_time, sysDateTime* in_time_to_substract);

sysStringLength sysDateTimeConvertTimeToString(sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_datetime);
sysStringLength sysDateTimeConvertDateToString(sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_date, sysChar in_separator);
void sysDateTimeConvertFromString(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator, sysDateTimeFormat in_format);
void sysDateTimeStringToTime(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator);

void sysDateTimeConvertFromString(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator, sysDateTimeFormat in_format);

void sysDateTimeGet(sysDateTime* out_datetime);
void sysDateTimeSet(sysDateTime* in_datetime);

#endif

