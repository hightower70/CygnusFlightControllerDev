/*****************************************************************************/
/* GUI resource data access functions                                        */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __drvResources_h
#define __drvResources_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <halIODefinitions.h>

#define drvRESOURCE_INVALID_ADDRESS 0

#ifndef drvResourceInit
void drvResourceInit(void);
#endif

#ifndef drvResourceReadByte
uint8_t drvResourceReadByte(sysResourceAddress in_address);
#endif

#ifndef drvResourceReadWord
uint16_t drvResourceReadWord(sysResourceAddress  in_address);
#endif 

#ifndef drvResourceReadDWord
uint32_t drvResourceReadDWord(sysResourceAddress in_address);
#endif

#ifndef drvResourceReadReverseWord
uint16_t drvResourceReadReverseWord(sysResourceAddress in_address);
#endif

#ifndef drvGetResourcePhysicalAddress
void* drvGetResourcePhysicalAddress(sysResourceAddress in_address);
#endif

#ifndef drvResourceGetStringLength
sysStringLength drvResourceGetStringLength(sysResourceAddress in_string_address);
#endif

#ifndef drvResourceGetString
sysConstString drvResourceGetString(sysResourceAddress in_string_address);
#endif

#endif

