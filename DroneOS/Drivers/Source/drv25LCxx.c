/*****************************************************************************/
/* AT25LCxx (SPI) EEPROM driver                                              */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <drvEEPROM.h>
#include <halEEPROM.h>
#include <sysRTOS.h>
#include <sysHighresTimer.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define drv25LCxx_COMMAND_BUFFER_LENGTH 3
#define drv25LCxx_PAGE_WRITE_TIMEOUT 30 // page write timeout in ms
#define drv25LCxx_PAGE_SIZE 32

// 25LCxx Serial EEPROM commands
#define drv25LCXX_SEE_WRSR	1			// write status register
#define drv25LCXX_SEE_WRITE	2			// write command
#define drv25LCXX_SEE_READ	3			// read command
#define drv25LCXX_SEE_WRDI	4			// write disable
#define drv25LCXX_SEE_RDSR	5			// read status register
#define drv25LCXX_SEE_WREN	6			// write enable

//  Constants for the status register.
#define drv25LCXX_SR_BLOCK_PROTECT_NONE      0x00
#define drv25LCXX_SR_BLOCK_PROTECT_QUARTER   0x04
#define drv25LCXX_SR_BLOCK_PROTECT_HALF      0x08
#define drv25LCXX_SR_BLOCK_PROTECT_ALL       0x0C
#define drv25LCXX_SR_WRITE_PROTECT_ENABLE    0x80
#define drv25LCXX_SR_WRITE_ENABLE            0x02
#define drv25LCXX_SR_WRITE_CYCLE_IN_PROGRESS 0x01


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint8_t l_command_buffer[drv25LCxx_COMMAND_BUFFER_LENGTH];


/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
bool drvEEPROMWaitForBusy(void);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes EEPROM subsystem
void drvEEPROMInit(void)
{
	halEEPROMInit();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Reads block from EEPROM
/// @param in_address Read from address
/// @param out_buffer Buffer will receive data
/// @param in_length Number of bytes to read
/// @return True if operation was success
bool drvEEPROMReadBlock(uint16_t in_address, uint8_t* out_buffer, uint16_t in_length)
{
	bool success;

	// prepare command
	l_command_buffer[0] = drv25LCXX_SEE_READ;
	l_command_buffer[1] = sysHIGH(in_address);
	l_command_buffer[2] = sysLOW(in_address);

	success = halEEPROMWriteAndReadBlock(l_command_buffer, 3, out_buffer, in_length);

	// wait for CS high
	sysHighresTimerDelay(2);

	return success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Writes block to the EEPROM
/// @param in_address Write to address
/// @param in_buffer Buffer containing data to write
/// @param in_length Number of bytes to write
/// @return True if operation was success
bool drvEEPROMWriteBlock(uint16_t in_address, uint8_t* in_buffer, uint16_t in_length)
{
	bool success = true;
	uint16_t remaining_byte_count;
	uint16_t current_block_length;
	uint16_t address;
	uint8_t* buffer;

	// init
	remaining_byte_count = in_length;
	address = in_address;
	buffer = in_buffer;

	// write pages
	while(success && remaining_byte_count > 0)
	{
		// determine block length
		current_block_length = drv25LCxx_PAGE_SIZE - (address % drv25LCxx_PAGE_SIZE);
		if(current_block_length > remaining_byte_count)
			current_block_length = remaining_byte_count;

		// unlock EEPROM
		l_command_buffer[0] = drv25LCXX_SEE_WREN;
		success = halEEPROMWriteAndWriteBlock(l_command_buffer, 1, sysNULL, 0);

		// wait for CS high
		sysHighresTimerDelay(2);

		// prepare command
		l_command_buffer[0] = drv25LCXX_SEE_WRITE;
		l_command_buffer[1] = sysHIGH(address);
		l_command_buffer[2] = sysLOW(address);

		// write page
		success = halEEPROMWriteAndWriteBlock(l_command_buffer, 3, buffer, current_block_length);
		buffer += current_block_length;
		address += current_block_length;
		remaining_byte_count -= current_block_length;

		// wait for CS high
		sysHighresTimerDelay(2);

		// wait for write operation
		if(success)
			success = drvEEPROMWaitForBusy();
	}

	return success;
}

/*****************************************************************************/
/* Local function implementation                                             */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Waits while EEPROM is busy
/// @return True if EEPROM write is accessible
bool drvEEPROMWaitForBusy(void)
{
	uint8_t status = 0;
	sysTick start_time;

	l_command_buffer[0] = drv25LCXX_SEE_RDSR;

	start_time = sysGetSystemTick();
	do
	{
		// read status register
		halEEPROMWriteAndReadBlock(l_command_buffer, 1, &status, 1);

		// If EEPROM is busy wait a little
		if((status & drv25LCXX_SR_WRITE_CYCLE_IN_PROGRESS ) != 0 )
			sysHighresTimerDelay(1000);

	}	while((status & drv25LCXX_SR_WRITE_CYCLE_IN_PROGRESS ) != 0 && sysGetSystemTickSince(start_time) < drv25LCxx_PAGE_WRITE_TIMEOUT);

	// wait for CS high
	sysHighresTimerDelay(2);

	return ((status & drv25LCXX_SR_WRITE_CYCLE_IN_PROGRESS ) == 0);
}

