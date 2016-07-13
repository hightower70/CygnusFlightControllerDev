/*****************************************************************************/
/* External I2C bus driver                                                   */
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
#include <halIODefinitions.h>
#include <stm32f4xx_hal.h>
#include <halHelpers.h>
#include <drvI2CMaster.h>
#include <drvEXT.h>

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
static drvI2CMasterModule l_ext_i2c;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes External I2C driver
void drvEXTInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
	
	/**I2C1 GPIO Configuration
	PB8     ------> I2C1_SCL
	PB9     ------> I2C1_SDA
	*/

  GPIO_InitStruct.Pin = SCL_EXT_Pin | SDA_EXT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Peripheral clock enable
  __I2C2_CLK_ENABLE();

  // Peripheral interrupt init
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

  // I2C init
  l_ext_i2c.I2CPort.Instance = I2C1;
  l_ext_i2c.I2CPort.Init.ClockSpeed = 400000;
  l_ext_i2c.I2CPort.Init.DutyCycle = I2C_DUTYCYCLE_2;
  l_ext_i2c.I2CPort.Init.OwnAddress1 = 0;
  l_ext_i2c.I2CPort.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  l_ext_i2c.I2CPort.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  l_ext_i2c.I2CPort.Init.OwnAddress2 = 0;
  l_ext_i2c.I2CPort.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  l_ext_i2c.I2CPort.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init(&l_ext_i2c.I2CPort);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief This function handles I2C1 event interrupt.
void I2C1_EV_IRQHandler(void)
{
	// call interrupt handler of HAL
	drvI2CMasterEventInterruptHandler(&l_ext_i2c);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief This function handles I2C1 error interrupt.
void I2C1_ER_IRQHandler(void)
{
	// call interrupt handler of HAL
	drvI2CMasterErrorInterruptHandler(&l_ext_i2c);
}

void drvEXTStartWriteAndReadBlock(uint8_t in_address, uint8_t* in_write_buffer, uint8_t in_write_buffer_length, uint8_t* in_read_buffer, uint8_t in_read_buffer_length, drvEXTCallbackFunction in_callback_function)
{
	l_ext_i2c.CallbackFunction = in_callback_function;

	drvI2CMasterStartWriteAndReadBlock(&l_ext_i2c, in_address, in_write_buffer, in_write_buffer_length, in_read_buffer, in_read_buffer_length);
}

void drvEXTStartWriteAndWriteBlock(uint8_t in_address, uint8_t* in_buffer1, uint8_t in_buffer1_length, uint8_t* in_buffer2, uint8_t in_buffer2_length, drvEXTCallbackFunction in_callback_function)
{
	l_ext_i2c.CallbackFunction = in_callback_function;

	drvI2CMasterStartWriteAndWriteBlock(&l_ext_i2c, in_address, in_buffer1, in_buffer1_length, in_buffer2, in_buffer2_length);
}

