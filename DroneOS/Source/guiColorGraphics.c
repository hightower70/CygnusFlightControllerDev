/*****************************************************************************/
/* Color graphics functions                                                  */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <guiColorGraphics.h>
#include <drvColorGraphics.h>
#include <halColorGraphics.h>
#include <halGraphicsDisplay.h>
#include <guiCommon.h>
#include <sysString.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static guiRect l_clip_rect;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes color graphics
void guiColorGraphicsInitialize()
{
	halColorGraphicsInitialize();
	halGraphicsDisplayInitialize();
	drvColorGraphicsRendererInitialize();

	l_clip_rect.Left = 0;
	l_clip_rect.Top = 0;
	l_clip_rect.Right = guiSCREEN_WIDTH - 1;
	l_clip_rect.Bottom = guiSCREEN_HEIGHT - 1;

	guiSetForegroundColor(0);
	drvColorGraphicsFillArea(0, 0, guiSCREEN_WIDTH - 1, guiSCREEN_HEIGHT - 1);
	guiSetForegroundColor(0xffffffff);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cleans up color graphics
void guiColorGraphicsCleanup(void)
{
	halColorGraphicsCleanup();
}

void guiOpenCanvas(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
}

void guiCloseCanvas(void)
{
}

void guiRefreshScreen(void)
{
	drvColorGraphicsRefreshScreen();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets clipping rectangle for graphics operation
/// @param in_clipping_rect
void guiSetClippingRect(guiRect in_clipping_rect)
{
	// Left
	if (in_clipping_rect.Left < 0)
		l_clip_rect.Left = 0;
	else
		l_clip_rect.Left = in_clipping_rect.Left;

	// Top
	if (in_clipping_rect.Top < 0)
		l_clip_rect.Top = 0;
	else
		l_clip_rect.Top = in_clipping_rect.Top;

	// Right
	if (in_clipping_rect.Right >= guiSCREEN_WIDTH)
		l_clip_rect.Right = guiSCREEN_WIDTH - 1;
	else
		l_clip_rect.Right = in_clipping_rect.Right;

	// Bottom
	if (in_clipping_rect.Bottom > guiSCREEN_HEIGHT)
		l_clip_rect.Bottom = guiSCREEN_HEIGHT - 1;
	else
		l_clip_rect.Bottom = in_clipping_rect.Bottom;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets clipping rectangle for graphics operation
/// @param in_left Left edge X coordinate
/// @param in_top Top edge Y coordinate
/// @param in_right Right edge X coordinate
/// @param in_bottom Bottom edge Y coordinate
void guiSetClipping(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
	// Left
	if (in_left < 0)
		l_clip_rect.Left = 0;
	else
		l_clip_rect.Left = in_left;

	// Top
	if (in_top < 0)
		l_clip_rect.Top = 0;
	else
		l_clip_rect.Top = in_top;

	// Right
	if (in_right >= guiSCREEN_WIDTH)
		l_clip_rect.Right = guiSCREEN_WIDTH - 1;
	else
		l_clip_rect.Right = in_right;

	// Bottom
	if (in_bottom > guiSCREEN_HEIGHT)
		l_clip_rect.Bottom = guiSCREEN_HEIGHT - 1;
	else
		l_clip_rect.Bottom = in_bottom;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Resets clipping rectangle to include the whole screen
void guiResetClipping(void)
{
	l_clip_rect.Left = 0;
	l_clip_rect.Top = 0;
	l_clip_rect.Right = guiSCREEN_WIDTH - 1;
	l_clip_rect.Bottom = guiSCREEN_HEIGHT - 1;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws text at the given coordinate with the selected font
/// @param in_x X coordinate of the text (position depends on the alignment)
/// @param in_y Y coordinate of the text (position depends on the alignment)
/// @param in_string Text to write
void guiDrawText(guiCoordinate in_x, guiCoordinate in_y, sysString in_string)
{
	sysResourceAddress chardata;
	int16_t i;
	sysStringLength len;
	guiCoordinate width;
	guiSize size;
	guiCoordinate offset_x, offset_y;
	guiCoordinate destination_width, destination_height;

	// check font
	if (g_gui_current_font.AsciiTableAddress == 0)
		return;

	// check align
	if (g_gui_text_align != 0)
	{
		// get text extent
		size = guiGetTextExtent(in_string);

		// horizontal align
		switch (g_gui_text_align & guiHA_MASK)
		{
			// left align
			case guiHA_LEFT:
				break;

				// center align
			case guiHA_CENTER:
				in_x -= size.Width / 2;
				break;

				// right align
			case guiHA_RIGHT:
				in_x -= size.Width;
				break;
		}

		// vertical align
		switch (g_gui_text_align & guiVA_MASK)
		{
			// top align
			case guiVA_TOP:
				break;

				// center align
			case guiVA_CENTER:
				in_y -= size.Height / 2;
				break;

				// bottom align
			case guiVA_BOTTOM:
				in_y -= size.Height;
				break;
		}
	}

	// draw characters
	len = sysGetStringLength(in_string);
	for (i = 0; i < len; i++)
	{
		if (in_string[i] >= g_gui_current_font.Minascii && in_string[i] <= g_gui_current_font.Maxascii)
		{
			chardata = drvResourceReadWord((in_string[i] - g_gui_current_font.Minascii) * sizeof(uint16_t) + g_gui_current_font.AsciiTableAddress) + g_gui_current_font.FontAddress;
			width = drvResourceReadByte(chardata++);

			if (in_x <= l_clip_rect.Right && in_y <= l_clip_rect.Bottom &&
				in_x + width >= l_clip_rect.Left && in_y + g_gui_current_font.Height >= l_clip_rect.Top)
			{
				offset_x = 0;
				offset_y = 0;

				destination_width = l_clip_rect.Right - in_x + 1;
				destination_height = l_clip_rect.Bottom - in_y + 1;

				if (destination_width > width)
					destination_width = width;

				if (destination_height > g_gui_current_font.Height)
					destination_height = g_gui_current_font.Height;

				if (in_x < l_clip_rect.Left)
				{
					offset_x = l_clip_rect.Left - in_x;
					destination_width -= offset_x;
				}

				if (in_y < l_clip_rect.Top)
				{
					offset_y = l_clip_rect.Top - in_y;
					destination_height -= offset_y;
				}

				drvColorGraphicsBitBltFromResource(in_x + offset_x, in_y + offset_y, destination_width, destination_height, offset_x, offset_y, width, g_gui_current_font.Height, chardata, 1);
			}

			in_x += width;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a rectangle with the current foreground color
/// @param in_left X coordinate of the left side
/// @param in_top Y coordinate of the top side
/// @param in_right X coordinate of the right side
/// @param in_bottom Y coordinate of the bottom side
void guiDrawRectangle(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
	drvColorGraphicsFillArea(in_left, in_top, in_right, in_top);
	drvColorGraphicsFillArea(in_left, in_top, in_left, in_bottom);
	drvColorGraphicsFillArea(in_right, in_top, in_right, in_bottom);
	drvColorGraphicsFillArea(in_left, in_bottom, in_right, in_bottom);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Fill a rectangle with the current foreground color (including rectangle border)
/// @param in_left X coordinate of the left side
/// @param in_top Y coordinate of the top side
/// @param in_right X coordinate of the right side
/// @param in_bottom Y coordinate of the bottom side
void guiFillRectangle(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
	//TODO: clipping
	drvColorGraphicsFillArea(in_left, in_top, in_right, in_bottom);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a horizontal line with the current foreground color
/// @param in_left X coordinate of the left point
/// @param in_top Y coordinate of the line
/// @param in_right X coordinate of the right point
void guiDrawHorizontalLine(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right)
{
	//TODO: clipping
	drvColorGraphicsFillArea(in_left, in_top, in_right, in_top);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a vertical line with the current foreground color
/// @param in_left X coordinate of the line
/// @param in_top Y coordinate of the top point
/// @param in_bottom Y coordinate of the bottom point
void guiDrawVerticalLine(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_bottom)
{
	//TODO: clipping
	drvColorGraphicsFillArea(in_left, in_top, in_left, in_bottom);
}

///////////////////////////////////////////////////////////////////////////////
// Draw bitmap
void guiDrawBitmapFromResource(guiCoordinate in_x, guiCoordinate in_y, sysResourceAddress in_bitmap_address)
{
	sysResourceAddress resource_address;
	guiCoordinate width;
	guiCoordinate height;
	uint8_t bpp;

	// get the bitmap address
	resource_address = /*l_bitmap_resource_address + */in_bitmap_address;

	// get bitmap data
	width = drvResourceReadWord(resource_address);
	resource_address += sizeof(uint16_t);

	height = drvResourceReadWord(resource_address);
	resource_address += sizeof(uint16_t);

	bpp = drvResourceReadByte(resource_address);
	resource_address += sizeof(uint8_t) + guiBITMAP_GET_ALIGNMENT_BYTE(bpp);
	bpp = guiBITMAP_GET_BPP(bpp);

	drvColorGraphicsBitBltFromResource(in_x, in_y, width, height, 0, 0, width, height, resource_address, bpp);
}

///////////////////////////////////////////////////////////////////////////////
// Draw bitmap
void guiBitblt(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
	guiCoordinate in_destination_width, guiCoordinate in_destination_height,
	guiCoordinate in_source_x, guiCoordinate in_source_y,
	guiCoordinate in_source_width, guiCoordinate in_source_height,
	void* in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	//TODO: clipping
	drvColorGraphicsBitBlt(in_destination_x, in_destination_y,
		in_destination_width, in_destination_height,
		in_source_x, in_source_y,
		in_source_width, in_source_height,
		in_source_bitmap, in_source_bit_per_pixel);
}
