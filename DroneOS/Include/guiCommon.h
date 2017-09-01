/*****************************************************************************/
/* GUI Common (BW and Color Module) Definitions                              */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef _guiCommon_h
#define _guiCommon_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <sysString.h>
#include <guiTypes.h>

/*****************************************************************************/
/* Consts                                                                    */
/*****************************************************************************/
#define guiBITMAP_GET_ALIGNMENT_BYTE(x) ((x >> 6) & 0x03)
#define guiBITMAP_GET_BPP(x) (x & 0x3f)


///////////////////////////////////////////////////////////////////////////////
// Global variables
extern guiFontInfo g_gui_current_font;
extern uint8_t g_gui_text_align;
extern guiRect g_gui_clip_rect;


///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void guiSetTextAlign(uint8_t in_align);
void guiSetFont(sysResourceAddress in_font_handle);
guiSize guiGetTextExtent(sysString in_string);
guiSize guiGetResourceTextExtent(sysResourceAddress in_string_handle);
guiSize guiGetBitmapSize(sysResourceAddress in_bitmap_handle);
sysResourceAddress guiGetBitmapData(sysResourceAddress in_bitmap_handle);

void guiExtendRect(guiRect* in_rect_to_extend, const guiRect* in_rect_to_include);

#endif
