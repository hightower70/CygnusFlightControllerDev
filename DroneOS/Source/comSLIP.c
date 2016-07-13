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
/// @return True if if was success, False when buffer is too small
bool slipEncodeBlock(slipEncoderState* in_state, uint8_t* in_buffer, uint8_t in_length)
{
	uint8_t data;
	bool success = true;

	// slip packet start
	in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_END;
	
	while(in_length > 0 && in_state->TargetBufferPos < in_state->TargetBufferSize)
	{
		data = *in_buffer;
		slipEncodeByte(in_state, data);

		in_buffer++;
		in_length--;
	}

	// store SLIP END
	if (in_state->TargetBufferPos < in_state->TargetBufferSize - 1)
	{
		in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_END;
	}
	else
	{
		success = false;
	}

	return success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes SLIP decoder 
/// @param in_state Decoder state
void slipDecodeInitialize(slipDecoderState* in_state)
{
	in_state->Status = slip_ES_Idle;
	in_state->TargetBufferPos = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief SLIP decodes one byte
/// @brief in_state SLIP decoder state
/// @brief in_byte Byte to decode
/// @return True if block end was found
bool slipDecodeByte(slipDecoderState* in_state, uint8_t in_byte)
{
	bool retval = false;

	// buffer is full, there is no full packet in the buffer ->clear buffer content
	if (in_state->TargetBufferPos == in_state->TargetBufferSize)
	{
		in_state->Status = slip_ES_Idle;
		in_state->TargetBufferPos = 0;
	}

	switch (in_state->Status)
	{
		// waiting for packet start
		case slip_ES_Idle:
			if(in_byte == slip_END)
			{
				in_state->Status = slip_ES_Data;

				// report only packet with non-zero length
				if (in_state->TargetBufferPos != 0)
				{
					retval = true;
					in_state->LastPacketLength = in_state->TargetBufferPos;
				}

				// prepare to receive next packet
				in_state->TargetBufferPos = 0;
			}
			break;

		// processing packet data
		case slip_ES_Data:
			switch (in_byte)
			{
				// escape code
				case slip_ESC:
					in_state->Status = slip_ES_Escape;
					break;

				// packet end
				case slip_END:
					// report only packet with non-zero length
					if (in_state->TargetBufferPos != 0)
					{
						retval = true;
						in_state->LastPacketLength = in_state->TargetBufferPos;
					}

					// prepare to receive next packet
					in_state->TargetBufferPos = 0;
					break;

				// normal data
				default:
					in_state->TargetBuffer[in_state->TargetBufferPos++] = in_byte;
					break;
			}
			break;

		// processing escape characters
		case slip_ES_Escape:
			switch(in_byte)
			{
				case slip_ESC_ESC:
					in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_ESC;
					break;

				case slip_ESC_END:
					in_state->TargetBuffer[in_state->TargetBufferPos++] = slip_END;
					break;
			}
			in_state->Status = slip_ES_Data;
			break;
	}

	return retval;
}
