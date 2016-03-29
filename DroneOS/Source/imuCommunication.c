/*****************************************************************************/
/* IMU Communication function                                                */
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
#include <sysTypes.h>
#include <drvIMU.h>
#include <sysRTOS.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define imuCommunication_Timeout 5 /// 5ms time should be enough for all imu communication

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static sysTaskNotify l_task_notify;
static volatile bool l_success;

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void imuCommunicationFinished(bool in_success, void* in_interrupt_param);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes IMU communication functions
void imuCommunicationInit(void)
{
	sysTaskNotifyCreate(l_task_notify);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Reads one byte register from the IMU sensor
/// @param in_i2c_address I2C bus address of the sensor
/// @param in_register_address Register address to read
/// @param out_register_value Pointer to store register value
/// @param inout_success Operation result (true - success, false - failed) Must be true before calling the function, otherwise the function immediatelly returns.
void imuReadByteRegister(uint8_t in_i2c_address, uint8_t in_register_address, uint8_t* out_register_value, bool* inout_success)
{
	uint8_t register_address = in_register_address;
	uint8_t register_value;

	l_success = *inout_success;

	// return if previous operations were failed
	if(!l_success)
		return;

	// start I2C communications
	drvIMUStartWriteAndReadBlock(in_i2c_address, &register_address, 1, &register_value, 1, imuCommunicationFinished);

	// wait until communication finished
	sysTaskNotifyTake(l_task_notify, imuCommunication_Timeout);

	*out_register_value = register_value;
	*inout_success = l_success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Writes one byte register of the IMU sensor
/// @param in_i2c_address I2C bus address of the sensor
/// @param in_register_address Register address to read
/// @param in_register_value Register value to write
/// @param inout_success Operation result (true - success, false - failed) Must be true before calling the function, otherwise the function immediatelly returns.
void imuWriteByteRegister(uint8_t in_i2c_address, uint8_t in_register_address, uint8_t in_register_value, bool* inout_success)
{
	uint8_t register_address = in_register_address;
	uint8_t register_value = in_register_value;

	l_success = *inout_success;

	// return if previous operations were failed
	if(!l_success)
		return;

	// start I2C communications
	drvIMUStartWriteAndWriteBlock(in_i2c_address, &register_address, 1, &register_value, 1, imuCommunicationFinished);

	// wait until communication finished
	sysTaskNotifyTake(l_task_notify, imuCommunication_Timeout);

	*inout_success = l_success;
}


void imuReadRegisterBlock(uint8_t in_i2c_address, uint8_t in_register_address, uint8_t* out_register_block, uint8_t in_register_block_length, bool* inout_success)
{
	uint8_t register_address = in_register_address;

	l_success = *inout_success;

	// return if previous operations were failed
	if(!l_success)
		return;

	// start I2C communications
	drvIMUStartWriteAndReadBlock(in_i2c_address, &register_address, 1, out_register_block, in_register_block_length, imuCommunicationFinished);

	// wait until communication finished
	sysTaskNotifyTake(l_task_notify, imuCommunication_Timeout);

	*inout_success = l_success;
}

void imuWriteRegisterBlock(uint8_t in_i2c_address, uint8_t in_register_address, uint8_t* in_register_block, uint8_t in_register_block_length, bool* inout_success)
{
	uint8_t register_address = in_register_address;

	l_success = *inout_success;

	// return if previous operations were failed
	if(!l_success)
		return;

	// start I2C communications
	drvIMUStartWriteAndWriteBlock(in_i2c_address, &register_address, 1, in_register_block, in_register_block_length, imuCommunicationFinished);

	// wait until communication finished
	sysTaskNotifyTake(l_task_notify, imuCommunication_Timeout);

	*inout_success = l_success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Communication finished interrupt handler
static void imuCommunicationFinished(bool in_success, void* in_interrupt_param)
{
	l_success = in_success;

	sysTaskNotifyGiveFromISR(l_task_notify, in_interrupt_param);
}
