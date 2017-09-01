/*****************************************************************************/
/* Color graphics HAL layer                                                  */
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
#include <Windows.h>
#include <halGraphicsDisplay.h>
#include "halIODefinitions.h"

/*****************************************************************************/
/*  Module global variables                                                  */
/*****************************************************************************/
static BITMAPINFO  l_bitmapinfo;
static int         l_bitmapwidth;       // Width of bitmap in pixels
static int         l_bitmapheight;      // Height of bitmap in pixels
static HANDLE      l_hbm;               // Bitmap handle
static HDC         l_hdc;								// Device context handle


/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
void*	g_gui_screen_pixels;       // Pointer to the (device independent) bitmap data
int   g_gui_screen_line_size;    // Size in bytes of a bitmap scanline

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize color graphics HAL system
void halColorGraphicsInitialize(void)
{
	LPVOID  pb;

	// Create memory device context
	l_hdc = CreateCompatibleDC(0);

	// Create DIB section to hold bitmap data
	ZeroMemory(&l_bitmapinfo, sizeof(l_bitmapinfo));
	l_bitmapinfo.bmiHeader.biSize = sizeof(l_bitmapinfo.bmiHeader);
	l_bitmapinfo.bmiHeader.biWidth = guiSCREEN_WIDTH;
	l_bitmapinfo.bmiHeader.biHeight = -guiSCREEN_HEIGHT; // Create top-down bitmap
	l_bitmapinfo.bmiHeader.biPlanes = 1;
	l_bitmapinfo.bmiHeader.biBitCount = 24;
	l_bitmapinfo.bmiHeader.biCompression = BI_RGB;
	l_bitmapinfo.bmiHeader.biSizeImage = 0;
	l_bitmapinfo.bmiHeader.biXPelsPerMeter = 1;
	l_bitmapinfo.bmiHeader.biYPelsPerMeter = 1;
	l_bitmapinfo.bmiHeader.biClrUsed = 0;
	l_bitmapinfo.bmiHeader.biClrImportant = 0;

	l_hbm = CreateDIBSection(l_hdc,
		&l_bitmapinfo,
		DIB_RGB_COLORS,
		&pb,
		0,
		0);

	// Associate the bitmap to the device context
	SelectObject(l_hdc, l_hbm);

	g_gui_screen_pixels = (LPBYTE)pb;
	g_gui_screen_line_size = 4 * ((guiSCREEN_WIDTH * 3 + 3) / 4); // Length of a scan line 
	l_bitmapwidth = guiSCREEN_WIDTH;
	l_bitmapheight = guiSCREEN_HEIGHT;
}

void halColorGraphicsCleanup(void)
{
	DeleteDC(l_hdc);
	DeleteObject(l_hbm);
}

void guiClearScreen(void)
{
	ZeroMemory(g_gui_screen_pixels, g_gui_screen_line_size * l_bitmapheight);
}

void drvGraphicsRefreshScreen(HDC in_hdc)
{
	StretchDIBits(in_hdc,
		0, 0,
		guiSCREEN_WIDTH*guiemuZOOM, guiSCREEN_HEIGHT*guiemuZOOM,
		0, 0,
		guiSCREEN_WIDTH, guiSCREEN_HEIGHT,
		g_gui_screen_pixels,
		&l_bitmapinfo,
		DIB_RGB_COLORS,
		SRCCOPY);
}
