/*****************************************************************************/
/* MD5 Hash Calculation Routines                                             */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __crcMD5_h
#define __crcMD5_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define crcMD5_HASH_SIZE 16

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
	uint32_t BitCount[2];			/* message length in bits, lsw first */
  uint32_t DigestBuffer[4];	/* digest buffer */
  uint8_t InputBuffer[64];	/* accumulate block */
} crcMD5State;

typedef struct
{
	uint8_t Hash[crcMD5_HASH_SIZE];
} crcMD5Hash;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void crcMD5Open(crcMD5State* in_state);
void crcMD5Update(crcMD5State* in_state, const uint8_t* in_data, uint32_t in_length);
void crcMD5Close(crcMD5State* in_state, crcMD5Hash* out_hash);

bool crcMD5IsEqual(crcMD5Hash* in_hash1, crcMD5Hash* in_hash2);

#endif
