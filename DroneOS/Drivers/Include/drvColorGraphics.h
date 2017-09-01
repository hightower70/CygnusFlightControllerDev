/*****************************************************************************/
/* Color graphics driver                                                     */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvColorGraphics_h
#define __drvColorGraphics_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void drvColorGraphicsRendererInitialize(void);
void drvColorGraphicsRefreshScreen(void);

void drvColorGraphicsFillArea(guiCoordinate in_x1, guiCoordinate in_y1, guiCoordinate in_x2, guiCoordinate in_y2);
void drvColorGraphicsBitBltFromResource(guiCoordinate in_destination_x, guiCoordinate in_destination_y, 
																				guiCoordinate in_destination_width, guiCoordinate in_destination_height,
																				guiCoordinate in_source_x, guiCoordinate in_source_y,
																				guiCoordinate in_source_width, guiCoordinate in_source_height, 
																				sysResourceAddress in_source_bitmap, uint8_t in_source_bit_per_pixel);

void drvColorGraphicsBitBlt(guiCoordinate in_destination_x, guiCoordinate in_destination_y, 
														guiCoordinate in_destination_width, guiCoordinate in_destination_height,
														guiCoordinate in_source_x, guiCoordinate in_source_y,
														guiCoordinate in_source_width, guiCoordinate in_source_height, 
														void* in_source_bitmap, uint8_t in_source_bit_per_pixel);

#endif