/*****************************************************************************/
/* SLIP with CRC Protocol Implementation                                     */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __SLIP_h
#define __SLIP_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define slip_END 0xc0
#define slip_ESC 0xdb
#define slip_ESC_END 0xdc
#define slip_ESC_ESC 0xdd

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

// Slip encoder state
typedef struct 
{
	uint8_t* TargetBuffer;
	uint16_t TargetBufferSize;
	uint16_t TargetBufferPos;
} slipEncoderState;

// Status of the last operation of SLIP encoder
typedef enum
{
	slip_ES_Idle,
	slip_ES_Data,
	slip_ES_Escape
} slipEncoderStatus;

// Slip decoder state
typedef struct
{
	slipEncoderStatus Status;
	uint8_t* TargetBuffer;
	uint16_t TargetBufferPos;
	uint16_t TargetBufferSize;
	uint16_t LastPacketLength;
} slipDecoderState;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

void slipEncodeByte(slipEncoderState* in_state, uint8_t in_byte);
bool slipEncodeBlock(slipEncoderState* in_state, uint8_t* in_buffer, uint8_t in_length);

void slipDecodeInitialize(slipDecoderState* in_state);
bool slipDecodeByte(slipDecoderState* in_state, uint8_t in_byte);


#endif
