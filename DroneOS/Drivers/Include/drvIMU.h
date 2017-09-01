/*****************************************************************************/
/* Eight channel servo output driver                                         */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvIMU_h
#define __drvIMU_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/// Maximum number of the handled sensor chips
#define drvIMU_MaxSensorCount		4

/// Sensor class/type values
#define drvIMU_SC_ACCELERATION	0x01
#define drvIMU_SC_GYRO					0x02
#define drvIMU_SC_MAGNETIC			0x04
#define drvIMU_SC_BAROMETRIC		0x08

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// Sensor config function type
typedef enum
{
	drvIMU_CF_Unknown,		/// Invalid
	drvIMU_CF_Detect,			/// Detect sensor existence
	drvIMU_CF_SelfTest		/// Run self test of the sensor
} drvIMUControlFunction;

typedef void (*drvIMUCallbackFunction)(bool in_success, void* in_interupt_param);
typedef void (*drvIMUSensorControlFunction)(drvIMUControlFunction in_function, void* in_function_parameter);
typedef void (*drvIMUSensorReadFunction)(void);

/// Sensor detect function parameter
typedef struct
{
	bool Success;
	drvIMUSensorControlFunction Control;
	drvIMUSensorReadFunction Read;
	uint8_t Class;
} drvIMUDetectParameter;

typedef struct
{
	bool Success;
} drvIMUSelfTestParameter;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvIMUInit(void);
void drvIMUStartWriteAndReadBlock(uint8_t in_address, uint8_t* in_write_buffer, uint8_t in_write_buffer_length, uint8_t* in_read_buffer, uint8_t in_read_buffer_length, drvIMUCallbackFunction in_callback_function);
void drvIMUStartWriteAndWriteBlock(uint8_t in_address, uint8_t* in_buffer1, uint8_t in_buffer1_length, uint8_t* in_buffer2, uint8_t in_buffer2_length, drvIMUCallbackFunction in_callback_function);

#endif
