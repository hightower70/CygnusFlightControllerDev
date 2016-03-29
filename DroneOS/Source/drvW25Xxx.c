/*****************************************************************************/
/*                                                                           */
/*    Domino Operation System Kernel Module                                  */
/*                                                                           */
/*    Copyright (C) 2008 Laszlo Arvai                                        */
/*                                                                           */
/*    ------------------------------------------------------------------     */
/*    W25X16, W25X32, W25X64 FLASH driver module                             */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <krnlSystemTimer.h>
#include <drvEEPROM.h>
#include <drvSPI.h>
#include "drvIOConfig.h"

///////////////////////////////////////////////////////////////////////////////
// Constants

// EEPROM commands
#define SEE_WRSR					1			// write status register
#define SEE_WRITE					2			// write command
#define SEE_READ					3			// read command
#define SEE_WDI						4			// write disable
#define SEE_STAT					5			// read status register
#define SEE_WEN						6			// write enable
#define SEE_SECTOR_ERASE	0x20	// sector erase

///////////////////////////////////////////////////////////////////////////////
// Module global variables
dosByte l_address_highest;

///////////////////////////////////////////////////////////////////////////////
// Initialize EEPROM
void drvInitEEPROM( void)
{
	l_address_highest = 0;
	InitEEPROMSPI();
	InitEEPROMCS();
}

///////////////////////////////////////////////////////////////////////////////
// Read status
dosByte drvEEPROMReadStatus( void)
{
	// Check the Serial EEPROM status register
	dosByte status;
	
	EEPROMCS(PIN_LOW);									// select the Serial EEPROM
	EEPROMSendReceiveByte( SEE_STAT );	// send a READ STATUS COMMAND
	status = EEPROMSendReceiveByte(0);	// send/receive
	EEPROMCS(PIN_HIGH);									// deselect terminate command
	
	return status;
}

///////////////////////////////////////////////////////////////////////////////
// Wait for EEPROM busy
void drvEEPROMWaitForBusy(dosWord in_max_wait_in_ms)
{
	dosWord start_time = krnlGetSystemTimer();
	dosWord diff_time;

	// wait (with timeout) if display is busy
	do
	{
		diff_time = krnlGetSystemTimer() - start_time;
	}	while( (drvEEPROMReadStatus() & 0x1) && diff_time < in_max_wait_in_ms);
}	


///////////////////////////////////////////////////////////////////////////////
// Write enable
void drvEEPROMWriteEnable(void)
{
	// send a Write Enable command
	EEPROMCS(PIN_LOW);									// select the Serial EEPROM
	EEPROMSendReceiveByte(SEE_WEN);			// write enable command
	EEPROMCS(PIN_HIGH);									// deselect terminate command
}

///////////////////////////////////////////////////////////////////////////////
// read block from eeprom
void drvEEPROMReadBlock( dosWord in_address, dosByte* out_buffer, dosWord in_length)
{
	dosWord i;
	
	// select eeprom and send read command and address
	EEPROMCS(PIN_LOW);									// select the Serial EEPROM
	EEPROMSendReceiveByte(SEE_READ);		// read command
	
	EEPROMSendReceiveByte( l_address_highest ); // address highest first
	EEPROMSendReceiveByte( HIGH(in_address) ); // address MSB first
	EEPROMSendReceiveByte( LOW(in_address) ); // address LSB (word aligned)
	
	// read block data
	for( i = 0; i < in_length; i++ )
	{
		out_buffer[i] = EEPROMSendReceiveByte(0); // send dummy, read byte
	}
	
	// relase CS
	EEPROMCS(PIN_HIGH);									// deselect terminate command
}

///////////////////////////////////////////////////////////////////////////////
// Sector erase
void drvEEPROMSectorErase(dosWord in_address)
{
	// Set the Write Enable Latch
	drvEEPROMWriteEnable();
	
	// erase block
	EEPROMCS(PIN_LOW);									// select the Serial EEPROM

	EEPROMSendReceiveByte(SEE_SECTOR_ERASE);		// read command
	EEPROMSendReceiveByte( l_address_highest );	// address highest first
	EEPROMSendReceiveByte( HIGH(in_address) );	// address MSB first
	EEPROMSendReceiveByte( LOW(in_address) );		// address LSB (word aligned)

	EEPROMCS(PIN_HIGH);									// deselect and terminate command

	// wait for block erase
	drvEEPROMWaitForBusy( 400 );
}	

///////////////////////////////////////////////////////////////////////////////
// write block to eeprom
void drvEEPROMWriteBlock( dosWord in_address, dosByte* in_buffer, dosWord in_length)
{
	dosWord i;
	
	// erase sector
	drvEEPROMSectorErase( in_address );
			
	// Set the Write Enable Latch
	drvEEPROMWriteEnable();
	
	// perform the write sequence
	EEPROMCS(PIN_LOW);												// select the Serial EEPROM
	EEPROMSendReceiveByte(SEE_WRITE);					// write command
	EEPROMSendReceiveByte( l_address_highest );	// address highest first
	EEPROMSendReceiveByte(HIGH(in_address));	// address MSB first
	EEPROMSendReceiveByte(LOW(in_address));		// address LSB (word aligned)

	// read block data
	for( i = 0; i < in_length; i++ )
	{
		EEPROMSendReceiveByte(in_buffer[i]); // read dummy, write byte
	}

	// relase CS
	EEPROMCS(PIN_HIGH);										// deselect terminate command

	// wait for busy	
	drvEEPROMWaitForBusy( 20 );
}
