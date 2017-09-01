/*****************************************************************************/
/* Common (BW, Color independent) graphics routines                          */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiCommon.h>
#include <drvResources.h>


///////////////////////////////////////////////////////////////////////////////
// Variables defined in other sources
guiFontInfo g_gui_current_font;
uint8_t g_gui_text_align = 0;
guiRect g_gui_clip_rect;

///////////////////////////////////////////////////////////////////////////////
// Set text align
void guiSetTextAlign(uint8_t in_align)
{
	g_gui_text_align = in_align;
}

///////////////////////////////////////////////////////////////////////////////
// Set font
void guiSetFont(sysResourceAddress in_font_handle)
{
	sysResourceAddress address;

	// get the font address
	address = /*l_font_resource_address + */in_font_handle;
	g_gui_current_font.Type = drvResourceReadByte(address++);
	g_gui_current_font.Flag = drvResourceReadByte(address++);
	g_gui_current_font.Width = drvResourceReadByte(address++);
	g_gui_current_font.Height = drvResourceReadByte(address++);
	g_gui_current_font.Baseline = drvResourceReadByte(address++);
	g_gui_current_font.HorizontalCharacterGap = drvResourceReadByte(address++);
	g_gui_current_font.Minascii = drvResourceReadByte(address++);
	g_gui_current_font.Maxascii = drvResourceReadByte(address++);
	g_gui_current_font.DefaultCharacter = drvResourceReadByte(address++);
	g_gui_current_font.UnicodeCount = drvResourceReadWord(address);
	address += sizeof(uint16_t);

	g_gui_current_font.AsciiTableAddress = address;
	g_gui_current_font.FontAddress = in_font_handle;
}

///////////////////////////////////////////////////////////////////////////////
// Get text extent
guiSize guiGetTextExtent(sysString in_string)
{
	guiSize size;

	sysResourceAddress chardata;
	int16_t i;
	sysStringLength len;

	// check font
	size.Width = 0;
	size.Height = 0;
	if (g_gui_current_font.AsciiTableAddress == 0)
		return size;

	// height
	size.Height = g_gui_current_font.Height;

	len = sysGetStringLength(in_string);

	// if the font is fixed
	if ((g_gui_current_font.Flag & guiFF_FIXED) != 0)
	{
		size.Width = g_gui_current_font.Width * len;
	}
	else
	{
		size.Width = 0;
		for (i = 0; i < len; i++)
		{
			if (in_string[i] >= g_gui_current_font.Minascii && in_string[i] <= g_gui_current_font.Maxascii)
			{
				chardata = drvResourceReadWord((in_string[i] - g_gui_current_font.Minascii) * sizeof(uint16_t) + g_gui_current_font.AsciiTableAddress);
				size.Width += drvResourceReadByte(chardata);
			}
		}
	}

	return size;
}

///////////////////////////////////////////////////////////////////////////////
// Get resource text extent
guiSize guiGetResourceTextExtent(sysResourceAddress in_string_handle)
{
	guiSize size;
	sysResourceAddress chardata;
	sysStringLength i;
	sysStringLength len;
	sysResourceAddress string;
	sysChar ch;

	size.Width = 0;
	size.Height = 0;

	// check handle
	if (in_string_handle == drvRESOURCE_INVALID_ADDRESS)
		return size;

	// check font
	size.Width = 0;
	size.Height = 0;
	if (g_gui_current_font.AsciiTableAddress == 0)
		return size;

	// height
	size.Height = g_gui_current_font.Height;

	len = drvResourceGetStringLength(in_string_handle);
	string = (sysResourceAddress)drvResourceGetString(in_string_handle);

	// if the font is fixed
	if ((g_gui_current_font.Flag & guiFF_FIXED) != 0)
	{
		size.Width = g_gui_current_font.Width * len;
	}
	else
	{
		size.Width = 0;
		for (i = 0; i < len; i++)
		{
			ch = drvResourceReadByte(string + i);

			if (ch >= g_gui_current_font.Minascii && ch <= g_gui_current_font.Maxascii)
			{
				chardata = drvResourceReadWord((ch - g_gui_current_font.Minascii) * sizeof(uint16_t) + g_gui_current_font.AsciiTableAddress);
				size.Width += drvResourceReadByte(chardata);
			}
		}
	}

	return size;
}

///////////////////////////////////////////////////////////////////////////////
// Get bitmap size
guiSize guiGetBitmapSize(sysResourceAddress in_bitmap_handle)
{
	sysResourceAddress resource_address;
	guiCoordinate width;
	guiCoordinate height;
	guiSize size;

	// get the bitmap address
	resource_address = in_bitmap_handle;

	// get bitmap data
	width = drvResourceReadWord(resource_address);
	resource_address += sizeof(uint16_t);

	height = drvResourceReadWord(resource_address);
	resource_address += sizeof(uint16_t);

	// store size
	size.Width = width;
	size.Height = height;

	return size;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets bitmap data pointer
/// @param in_bitmap_handle Handle of the bitmap 
/// @return Resource address of the bitmap data
sysResourceAddress guiGetBitmapData(sysResourceAddress in_bitmap_handle)
{
	uint8_t bpp;
	sysResourceAddress pos;

	pos = in_bitmap_handle + sizeof(uint16_t) + sizeof(uint16_t); // skip width, height
	bpp = drvResourceReadByte(pos);

	pos += sizeof(uint8_t) + guiBITMAP_GET_ALIGNMENT_BYTE(bpp); // skip bpp and alignment bytes

	return pos;
}

///////////////////////////////////////////////////////////////////////////////
// Extend rect to include the specified rectangle
void guiExtendRect(guiRect* in_rect_to_extend, const guiRect* in_rect_to_include)
{
	// left
	if (in_rect_to_extend->Left == guiINVALID_COORDINATE)
		in_rect_to_extend->Left = in_rect_to_include->Left;
	else
	{
		if (in_rect_to_include->Left < in_rect_to_extend->Left)
			in_rect_to_extend->Left = in_rect_to_include->Left;
	}

	// top
	if (in_rect_to_extend->Top == guiINVALID_COORDINATE)
		in_rect_to_extend->Top = in_rect_to_include->Top;
	else
	{
		if (in_rect_to_include->Top < in_rect_to_extend->Top)
			in_rect_to_extend->Top = in_rect_to_include->Top;
	}

	// right
	if (in_rect_to_extend->Right == guiINVALID_COORDINATE)
		in_rect_to_extend->Right = in_rect_to_include->Right;
	else
	{
		if (in_rect_to_include->Right > in_rect_to_extend->Right)
			in_rect_to_extend->Right = in_rect_to_include->Right;
	}

	// bottom
	if (in_rect_to_extend->Bottom == guiINVALID_COORDINATE)
		in_rect_to_extend->Bottom = in_rect_to_include->Bottom;
	else
	{
		if (in_rect_to_include->Bottom > in_rect_to_extend->Bottom)
			in_rect_to_extend->Bottom = in_rect_to_include->Bottom;
	}
}

