/*****************************************************************************/
/*                                                                           */
/*    Simple Operation System Driver Module                                  */
/*                                                                           */
/*    Copyright (C) 2008-2015 Laszlo Arvai                                   */
/*                                                                           */
/*    ------------------------------------------------------------------     */
/*    Resource data access routines (resource stored in C array)             */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes files
#include <drvResources.h>

///////////////////////////////////////////////////////////////////////////////
// Module local variables
extern const unsigned char g_resource_file[];

///////////////////////////////////////////////////////////////////////////////
// Init ROM Access
void drvResInit(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// Read byte from ROM
uint8_t drvResourceReadByte(sysResourceAddress in_address)
{
	return g_resource_file[in_address];
}

void* drvGetResourcePhysicalAddress(sysResourceAddress in_address)
{
	return (void*)&g_resource_file[in_address];
}

///////////////////////////////////////////////////////////////////////////////
// Read word from ROM
uint16_t drvResourceReadWord(sysResourceAddress in_address)
{
	return g_resource_file[in_address] + ((g_resource_file[in_address + 1]) << 8);
}

///////////////////////////////////////////////////////////////////////////////
// Read double word from ROM
uint32_t drvResourceReadDWord(sysResourceAddress in_address)
{
	return ((uint32_t)g_resource_file[in_address]) +
		(((uint32_t)g_resource_file[in_address + 1]) << 8) +
		(((uint32_t)g_resource_file[in_address + 2]) << 16) +
		(((uint32_t)g_resource_file[in_address + 3]) << 24);
}

///////////////////////////////////////////////////////////////////////////////
// Read MSB first word from ROM
uint16_t drvResourceReadReverseWord(sysResourceAddress in_address)
{
	return ((g_resource_file[in_address]) << 8) + g_resource_file[in_address + 1];
}

///////////////////////////////////////////////////////////////////////////////
// Get resource string length
sysStringLength drvResourceGetStringLength(sysResourceAddress in_string_address)
{
	sysStringLength length = 0;
	uint8_t data;
	sysResourceAddress address;

	address = /*l_string_resource + */in_string_address;

	do
	{
		data = drvResourceReadByte(address++);

		length = (length << 7) + (data & 0x7f);
	} while (data > 127);

	return length;
}

///////////////////////////////////////////////////////////////////////////////
// Get resource string
sysConstString drvResourceGetString(sysResourceAddress in_string_address)
{
	sysResourceAddress address;

	address = in_string_address;

	while (drvResourceReadByte(address++) > 127);

	return (sysConstString)&g_resource_file[address];
}

