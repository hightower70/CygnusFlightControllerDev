/*****************************************************************************/
/* NEATO XV-11 Lidar Driver                                                  */
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
#include <sysRTOS.h>
#include <halUART.h>
#include <drvLidar.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvXV11LIDAR_START_BYTE 0xfa
#define drvXV11LIDAR_INDEX_LOW_BYTE 0xa0
#define drvXV11LIDAR_INDEX_HIGH_BYTE 0xf9
#define drvXV11LIDAR_INVALID_INDEX 0xff

#define drvXV11LIDAR_DISTANCE_BUFFER_COUNT 2
#define drvXV11LIDAR_DISTANCE_BUFFER_ENTRY_COUNT 360

#define drvXV11LIDAR_INVALID_DISTANCE_FLAG 0x8000
#define drvXV11LIDAR_WARING_FLAG 0x4000

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// Stream parser state codes
typedef enum
{
	drvXV11Lidar_PS_Start,
	drvXV11Lidar_PS_Index,
	drvXV11Lidar_PS_SpeedLow,
	drvXV11Lidar_PS_SpeedHigh,

	drvXV11Lidar_PS_Data0_0,
	drvXV11Lidar_PS_Data0_1,
	drvXV11Lidar_PS_Data0_2,
	drvXV11Lidar_PS_Data0_3,

	drvXV11Lidar_PS_Data1_0,
	drvXV11Lidar_PS_Data1_1,
	drvXV11Lidar_PS_Data1_2,
	drvXV11Lidar_PS_Data1_3,

	drvXV11Lidar_PS_Data2_0,
	drvXV11Lidar_PS_Data2_1,
	drvXV11Lidar_PS_Data2_2,
	drvXV11Lidar_PS_Data2_3,

	drvXV11Lidar_PS_Data3_0,
	drvXV11Lidar_PS_Data3_1,
	drvXV11Lidar_PS_Data3_2,
	drvXV11Lidar_PS_Data3_3,

	drvXV11Lidar_PS_CheckSumLow,
	drvXV11Lidar_PS_CheckSumHigh

} drvXV11LidarParserState;

/// Data entry for received lidar distance scan
typedef struct
{
	uint16_t Distance;
	uint16_t Strength;
} drvXV11LidarDataEntry;

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void drvXV11LidarUARTRxCallback(uint8_t in_char, void* in_interrupt_param);
static void drvXV11LidarUARTTxCallback(void* in_interrupt_param);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint8_t l_uart_index = 0;

static uint32_t l_checksum;
static bool l_checksum_data_low;
static uint16_t l_current_index;
static uint16_t l_start_index;


static drvXV11LidarParserState l_parser_state;
static uint16_t l_word_data;
static drvXV11LidarDataEntry l_distance_data[drvXV11LIDAR_DISTANCE_BUFFER_COUNT][drvXV11LIDAR_DISTANCE_BUFFER_ENTRY_COUNT];
static uint8_t l_receiver_distance_buffer_index;
static uint8_t l_current_distance_buffer_index;
static uint16_t l_rpm;
static uint16_t l_current_distance;
static uint16_t l_current_strength;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

void drvLidarInitialize(void)
{
	halUARTConfigInfo uart_config;

	// init varables
	l_checksum = 0;
	l_word_data = 0;
	l_checksum_data_low = true;
	l_parser_state = drvXV11Lidar_PS_Start;
	l_receiver_distance_buffer_index = 0;
	l_current_distance_buffer_index = 0;

	// UART init
	halUARTConfigInfoInit(&uart_config);
	uart_config.RxReceivedCallback = drvXV11LidarUARTRxCallback;
	uart_config.TxEmptyCallback = drvXV11LidarUARTTxCallback;
	halUARTConfig(l_uart_index, &uart_config);
	halUARTSetBaudRate(l_uart_index, 115200);
}

uint16_t drvLidarGetDistance(uint16_t in_angle)
{
	if (in_angle >= drvXV11LIDAR_DISTANCE_BUFFER_ENTRY_COUNT)
		return drvLIDAR_INVALID_DISTANCE;

	if(l_distance_data[l_current_distance_buffer_index][in_angle].Strength == 0)
		return drvLIDAR_INVALID_DISTANCE;

	return l_distance_data[l_current_distance_buffer_index][in_angle].Distance;
}

static void drvXV11LidarUARTRxCallback(uint8_t in_char, void* in_interrupt_param)
{
	// update checksum
	if (l_checksum_data_low)
	{
		// store low byte
		l_word_data = in_char;
	}
	else
	{
		// calculate new checksum
		l_word_data |= (((uint16_t)in_char) << 8);
		if (l_parser_state != drvXV11Lidar_PS_CheckSumLow && l_parser_state != drvXV11Lidar_PS_CheckSumHigh)
			l_checksum = (l_checksum << 1) + l_word_data;
	}
	l_checksum_data_low = !l_checksum_data_low;

	// process received character
	switch (l_parser_state)
	{
		// wait for start byte
		case drvXV11Lidar_PS_Start:
			if (in_char == drvXV11LIDAR_START_BYTE)
			{
				// restart checksum calculator
				l_checksum = 0;
				l_word_data = in_char;
				l_checksum_data_low = false;
				l_parser_state = drvXV11Lidar_PS_Index;
			}
			break;

		// wait for index byte
		case drvXV11Lidar_PS_Index:
			if (in_char >= drvXV11LIDAR_INDEX_LOW_BYTE && in_char <= drvXV11LIDAR_INDEX_HIGH_BYTE)
			{
				// calculate index in degreee
				l_current_index = 4 * (in_char - drvXV11LIDAR_INDEX_LOW_BYTE);

				// next state
				l_parser_state = drvXV11Lidar_PS_SpeedLow;
			}
			else
			{
				// communication error -> restart parsing
				l_parser_state = drvXV11Lidar_PS_Start;
			}
			break;

		// speed low byte
		case drvXV11Lidar_PS_SpeedLow:
			l_parser_state = drvXV11Lidar_PS_SpeedHigh;
			break;

		// speed high byte
		case drvXV11Lidar_PS_SpeedHigh:
			l_rpm = l_word_data;
			l_parser_state = drvXV11Lidar_PS_Data0_0;
			break;

		// process first data byte
		case drvXV11Lidar_PS_Data0_0:
		case drvXV11Lidar_PS_Data1_0:
		case drvXV11Lidar_PS_Data2_0:
		case drvXV11Lidar_PS_Data3_0:
			l_parser_state++;
			break;

		// process second data byte
		case drvXV11Lidar_PS_Data0_1:
		case drvXV11Lidar_PS_Data1_1:
		case drvXV11Lidar_PS_Data2_1:
		case drvXV11Lidar_PS_Data3_1:
			l_current_distance = l_word_data;
			l_parser_state++;
			break;

		// process third data byte
		case drvXV11Lidar_PS_Data0_2:
		case drvXV11Lidar_PS_Data1_2:
		case drvXV11Lidar_PS_Data2_2:
		case drvXV11Lidar_PS_Data3_2:
			l_parser_state++;
			break;

		// process fourth data byte
		case drvXV11Lidar_PS_Data0_3:
		case drvXV11Lidar_PS_Data1_3:
		case drvXV11Lidar_PS_Data2_3:
		case drvXV11Lidar_PS_Data3_3:
			l_current_strength = l_word_data;

			// check results
			if ((l_current_distance & drvXV11LIDAR_INVALID_DISTANCE_FLAG) != 0 || (l_current_distance & drvXV11LIDAR_WARING_FLAG) != 0)
			{
				l_current_distance = 0;
				l_current_strength = 0;
			}

			// store data
			l_distance_data[l_receiver_distance_buffer_index][l_current_index].Distance = l_current_distance;
			l_distance_data[l_receiver_distance_buffer_index][l_current_index].Strength= l_current_strength;

			l_current_index++;
			l_parser_state++;
			break;


		// checksum low byte
		case drvXV11Lidar_PS_CheckSumLow:
			l_parser_state = drvXV11Lidar_PS_CheckSumHigh;
			break;

		// checksum high byte
		case drvXV11Lidar_PS_CheckSumHigh:
		{
			uint16_t checksum = ((l_checksum)+(l_checksum >> 15)) & 0x7FFF;
			if (checksum == l_word_data)
			{
				// packet received
			}

			// check for complete scan
			if (l_current_index == drvXV11LIDAR_DISTANCE_BUFFER_ENTRY_COUNT)
			{
				l_current_distance_buffer_index = l_receiver_distance_buffer_index;
				l_receiver_distance_buffer_index = 1 - l_receiver_distance_buffer_index;

				drvLidarScanIsCompleteCallback();

				l_parser_state = drvXV11Lidar_PS_Start;
			}

			l_parser_state = drvXV11Lidar_PS_Start;
		}
		break;
	}
}

static void drvXV11LidarUARTTxCallback(void* in_interrupt_param)
{

}

uint16_t drvXV11LidarGetRPM(void)
{
	return l_rpm / 64;
}