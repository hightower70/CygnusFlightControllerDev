/*****************************************************************************/
/* Color graphics functions                                                  */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __guiColorGraphics_h
#define __guiColorGraphics_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <guiTypes.h>
#include <guiCommon.h>

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void guiColorGraphicsInitialize(void);
void guiColorGraphicsCleanup(void);

void guiOpenCanvas(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_width, guiCoordinate in_height);
void guiCloseCanvas(void);
void guiRefreshScreen(void);

void guiSetClipping(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom);
void guiSetClippingRect(guiRect in_clipping_rect);
void guiResetClipping(void);

void guiSetForegroundColor(guiColor in_color);
void guiSetBackgroundColor(guiColor in_color);
void guiFillRectangle(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom);
void guiDrawColorPixel(guiCoordinate in_x, guiCoordinate in_y, guiColor in_color);
void guiDrawHorizontalLine(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right);
void guiDrawVerticalLine(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_bottom);
void guiDrawText(guiCoordinate in_x, guiCoordinate in_y, sysString in_string);
void guiDrawRectangle(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom);
void guiDrawBitmapFromResource(guiCoordinate in_x, guiCoordinate in_y, sysResourceAddress in_bitmap_address);
void guiBitblt(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
	guiCoordinate in_destination_width, guiCoordinate in_destination_height,
	guiCoordinate in_source_x, guiCoordinate in_source_y,
	guiCoordinate in_source_width, guiCoordinate in_source_height,
	void* in_source_bitmap, uint8_t in_source_bit_per_pixel);

void guiDrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

#endif


