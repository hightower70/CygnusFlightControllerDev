/*****************************************************************************/
/* SLIP with CRC Protocol Implementation                                     */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <comSLIP.h>
#include <crcCITT16.h>

/*****************************************************************************/
/* Functions implementation                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores one byte in the packet buffer using SLIP encoding
/// @param in_byte Data to store
void slipEncodeByte(slipEncoderState* in_state, uint8_t in_byte)
{
	// check to see if is a special character
	switch (in_byte)
	{
		case slip_END:
			in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_ESC; // escape special character
			in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_ESC_END;
			break;

		case slip_ESC:
			in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_ESC; // escape special character
			in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_ESC_ESC;
			break;

		// send raw character
		default:
			in_state->TargetBuffer[in_state->TargetBufferPos++] = in_byte;
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores block of data in the packet buffer using SLIP encoding and updates CRC
/// @param in_state Pointer to the decoder state structure
/// @param in_buffer Pointer to the buffer
/// @param in_length Number of bytes to store
void slipEncodeBlock(slipEncoderState* in_state, uint8_t* in_buffer, uint8_t in_length)
{
	uint8_t data;

	while(in_length > 0)
	{
		data = *in_buffer;
		slipEncodeByte(in_state, data);
		in_state->CyclicRedundancyCheck = crc16Calculate(in_state->CyclicRedundancyCheck, data);

		in_buffer++;
		in_length--;
	}
}

void slipDecodeInitialize(slipDecoderState* in_state)
{
	in_state->Status = slip_ES_Idle;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief SLIP decodes one byte
/// @brief in_state SLIP decoder state
/// @brief in_byte Byte to decode
void slipDecodeByte(slipDecoderState* in_state, uint8_t in_byte)
{
	switch (in_state->Status)
	{
		// waiting for packet start
		case slip_ES_Idle:
			if(in_byte == slip_END)
			{
				in_state->Status = slip_ES_Data;
				in_state->TargetBufferPos = 0;
				in_state->CyclicRedundancyCheck = crc16_INIT_VALUE;
			}
			break;

		// processing packet data
		case slip_ES_Data:
			if(in_byte == slip_ESC)
			{
				in_state->Status = slip_ES_Escape;
			}
			else
			{
				in_state->TargetBuffer[in_state->TargetBufferPos++] = in_byte;
				in_state->CyclicRedundancyCheck = crc16Calculate(in_state->CyclicRedundancyCheck, in_byte);
			}
			break;

		// processing escape characters
		case slip_ES_Escape:
			switch(in_byte)
			{
				case slip_ESC_ESC:
					in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_ESC;
					in_state->CyclicRedundancyCheck = crc16Calculate(in_state->CyclicRedundancyCheck, slip_ESC);
					break;

				case slip_ESC_END:
					in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_END;
					in_state->CyclicRedundancyCheck = crc16Calculate(in_state->CyclicRedundancyCheck, slip_END);
					break;
			}
			break;
	}
}
