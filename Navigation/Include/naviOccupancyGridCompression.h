/*****************************************************************************/
/* Occupancy Grid Compression functions                                      */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __naviOccupancyGridCompression_h
#define __naviOccupancyGridCompression_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
naviOGCoordinate naviOGCompress(naviOGCoordinate in_row, naviOGCoordinate in_first_colunm, naviOGCoordinate in_last_column, uint8_t* in_buffer, uint16_t in_buffer_length);

#endif
