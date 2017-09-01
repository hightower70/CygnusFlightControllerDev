/*****************************************************************************/
/* Occupancy Grid Compression functions                                      */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <naviOccupancyGrid.h>
#include <naviOccupancyGridCompression.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
	uint8_t NumberOfBits;
	uint8_t CompressedValue;
} naviOGCTableEntry;


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////
//   bit           compressed value
//    1.              0 /      \ 1
//                  Unknown    /\
//                            /  \
//    2.                   0 /    \ 1
//                         free   /\
//    3.                       0 /  \ 1
//                              /    \
//                             /      \
//                            /        \
//                           /          \
//    4.                 0 /  \ 1    0 /  \ 1
//                     wall    n.a. path  n.a.
static naviOGCTableEntry l_compression_table[] =
{
	{1, 0x00 }, // naviOG_UNKNOWN
	{2, 0x02 }, // naviOG_FREE
	{4, 0x0c }, // naviOG_WALL
	{4, 0x0e }  // naviOG_PATH
};

///////////////////////////////////////////////////////////////////////////////
/// @brief Compresses one row of occupancy grid using fixed Huffman tree
/// @param in_row Row of the grid to compress
/// @param in_first_column First column index to compress in the given row
/// @param in_last_column Last column index to compress
/// @param in_buffer Buffer to put binary compressed data
/// @param in_buffer_length Size of the buffer in bytes
/// @retval Last colunm index which was compressed
naviOGCoordinate naviOGCompress(naviOGCoordinate in_row, naviOGCoordinate in_first_colunm, naviOGCoordinate in_last_column, uint8_t* in_buffer, uint16_t in_buffer_length)
{
	naviOGElementType entry;
	naviOGCoordinate current_column;
	naviOGElementType* current_row;
	uint16_t compressed_buffer;
	uint8_t compressed_buffer_index;
	uint16_t buffer_index;
	naviOGCTableEntry* compressed_value;

	// initialize
	compressed_buffer = 0;
	compressed_buffer_index = 0;
	buffer_index = 0;
	current_row = &g_navi_occupancy_grid[in_row][0];
	current_column = in_first_colunm;

	while (buffer_index < in_buffer_length && current_column <= in_last_column)
	{
		// determine compressed value (table index)
		entry = current_row[current_column];

		switch (entry)
		{
			case naviOG_UNKNOWN:
				compressed_value = &l_compression_table[0];
				break;

			case naviOG_FREE:
				compressed_value = &l_compression_table[1];
				break;

			case naviOG_WALL:
				compressed_value = &l_compression_table[2];
				break;

			default:
				compressed_value = &l_compression_table[0];
				break;
		}

		// store compressed value
		compressed_buffer <<= compressed_value->NumberOfBits;
		compressed_buffer_index += compressed_value->NumberOfBits;

		// store value if eigth bits are collected
		if (compressed_buffer_index >= 8)
		{
			compressed_buffer_index -= 8;
			in_buffer[buffer_index++] = (compressed_buffer >> compressed_buffer_index) & 0xff;
		}

		// increment column (if there is space in the buffer)
		if (buffer_index < in_buffer_length)
		{
			current_column++;
		}
	}

	// store remaining bits
	if (compressed_buffer_index > 0 && buffer_index < in_buffer_length)
	{
		in_buffer[buffer_index] = (compressed_buffer << (8 - compressed_buffer_index)) & 0xff;
	}

	return current_column;
}
