/*****************************************************************************/
/* 25LCxx (SPI) EEPROM driver                                                */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <drvEEPROM.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

// 25LCxx Serial EEPROM commands
#define drv25LCXX_SEE_WRSR	1			// write status register
#define drv25LCXX_SEE_WRITE	2			// write command
#define drv25LCXX_SEE_READ	3			// read command
#define drv25LCXX_SEE_WDI		4			// write disable
#define drv25LCXX_SEE_STAT	5			// read status register
#define drv25LCXX_SEE_WEN		6			// write enable

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/
extern void drvSPIInit(void);

extern void drvSPIWriteAndReadBlock(uint8_t* in_write_block, uint16_t in_write_block_length, uint8_t* out_read_block, uint16_t in_read_block_length);


uint8_t buffer[10];
uint8_t buffer2[10];

///////////////////////////////////////////////////////////////////////////////
/// @brief nitializes EEPROM subsystem
void drvEEPROMInit(void)
{
	drvSPIInit();

  buffer[0] = drv25LCXX_SEE_READ;
  buffer[1] = 0x00;
  buffer[2] = 0x00;

  drvSPIWriteAndReadBlock(buffer, 3, buffer2, 10);
}

void drvEEPROMReadBlock(uint16_t in_address, uint8_t* out_buffer, uint16_t in_length)
{
  //drvHAL_SetPinLow(SPI_CS_GPIO_Port, SPI_CS_Pin);


  //drvHAL_SetPinHigh(SPI_CS_GPIO_Port, SPI_CS_Pin);
}

void drvEEPROMWriteBlock(uint16_t in_address, uint8_t* in_buffer, uint16_t in_length)
{

}

bool drvEEPROMVerifyBlock(uint16_t in_address, uint8_t* in_buffer, uint16_t in_length)
{

	return false;
}
