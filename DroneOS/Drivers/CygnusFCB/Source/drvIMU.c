/*****************************************************************************/
/* Eight channel servo output driver                                         */
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
#include <drvIODefinitions.h>
#include <stm32f4xx_hal.h>
#include <drvHAL.h>
#include <drvI2CMaster.h>
#include <drvIMU.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/



/*****************************************************************************/
/* External functions                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static drvI2CMasterModule l_imu_i2c;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes IMU driver
void drvIMUInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /**I2C2 GPIO Configuration
  PB10     ------> I2C2_SCL
  PB11     ------> I2C2_SDA
  */

  GPIO_InitStruct.Pin = SCL_Pin|SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Peripheral clock enable
  __I2C2_CLK_ENABLE();

  // Peripheral interrupt init
  HAL_NVIC_SetPriority(I2C2_EV_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
  HAL_NVIC_SetPriority(I2C2_ER_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);

  // I2C init
  l_imu_i2c.I2CPort.Instance = I2C2;
  l_imu_i2c.I2CPort.Init.ClockSpeed = 400000;
  l_imu_i2c.I2CPort.Init.DutyCycle = I2C_DUTYCYCLE_2;
  l_imu_i2c.I2CPort.Init.OwnAddress1 = 0;
  l_imu_i2c.I2CPort.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  l_imu_i2c.I2CPort.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  l_imu_i2c.I2CPort.Init.OwnAddress2 = 0;
  l_imu_i2c.I2CPort.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  l_imu_i2c.I2CPort.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init(&l_imu_i2c.I2CPort);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief This function handles I2C2 event interrupt.
void I2C2_EV_IRQHandler(void)
{
	// call interrupt handler of HAL
	drvI2CMasterEventInterruptHandler(&l_imu_i2c);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief This function handles I2C2 error interrupt.
void I2C2_ER_IRQHandler(void)
{
	// call interrupt handler of HAL
	drvI2CMasterErrorInterruptHandler(&l_imu_i2c);
}

void drvIMUStartWriteAndReadBlock(uint8_t in_address, uint8_t* in_write_buffer, uint8_t in_write_buffer_length, uint8_t* in_read_buffer, uint8_t in_read_buffer_length, drvIMUCallbackFunction in_callback_function)
{
	l_imu_i2c.CallbackFunction = in_callback_function;

	drvI2CMasterStartWriteAndReadBlock(&l_imu_i2c, in_address, in_write_buffer, in_write_buffer_length, in_read_buffer, in_read_buffer_length);
}

void drvIMUStartWriteAndWriteBlock(uint8_t in_address, uint8_t* in_buffer1, uint8_t in_buffer1_length, uint8_t* in_buffer2, uint8_t in_buffer2_length, drvIMUCallbackFunction in_callback_function)
{
	l_imu_i2c.CallbackFunction = in_callback_function;

	drvI2CMasterStartWriteAndWriteBlock(&l_imu_i2c, in_address, in_buffer1, in_buffer1_length, in_buffer2, in_buffer2_length);
}

