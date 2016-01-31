/*****************************************************************************/
/* MD5 Hash Calculation Routines                                             */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __crcMD5_h
#define __crcMD5_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define crcMD5_CHECKSUM_SIZE 16

///////////////////////////////////////////////////////////////////////////////
// Types
typedef struct 
{
	uint32_t BitCount[2];			/* message length in bits, lsw first */
  uint32_t DigestBuffer[4];	/* digest buffer */
  uint8_t InputBuffer[64];	/* accumulate block */
} crcMD5StateType;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void crcOpenMD5(crcMD5StateType* in_state);
void crcUpdateMD5(crcMD5StateType* in_state, const uint8_t* in_data, uint32_t in_length);
void crcCloseMD5(crcMD5StateType* in_state, uint8_t out_hash[]);

#endif
