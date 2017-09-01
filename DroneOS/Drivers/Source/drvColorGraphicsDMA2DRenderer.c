/*****************************************************************************/
/* Color graphics driver STM32F429 DMA2D renderer                            */
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
#include <stm32f4xx_hal.h>
#include <guiTypes.h>
#include <drvResources.h>
#include <halColorGraphics.h>
#include <guiColorGraphics.h>
#include <guiColors.h>

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/
#define RED_MASK 0xF800;
#define GREEN_MASK 0x7E0;
#define BLUE_MASK 0x1F;

#define PIXEL_SIZE 2
#define ABS(X)	((X) > 0 ? (X) : -(X))
#define RGB888ToRGB565(x) (uint16_t)(((x & 0xf80000u) >> 8) | \
													 	 	 	 	 	 ((x & 0x00fc00u) >> 5) | \
													 	 	 	 	 	 ((x & 0x0000f8u) >> 3))
#define LAYER_ADDRESS(x) ((x == 0) ? guiLCD_FRAME_BUFFER : guiLCD_FRAME_BUFFER + guiSCREEN_WIDTH * guiSCREEN_HEIGHT * 2)
#define GET_ALPHA(x)	(x >> 24)

/*****************************************************************************/
/* Module local functions declarations                                       */
/*****************************************************************************/
static void drvColorGraphicsFillBuffer(void *in_destination, uint32_t in_xsize, uint32_t in_ysize, uint32_t in_offset, guiColor in_color);
static void drvBitBltFrom1BPP(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
		guiCoordinate in_destination_width, guiCoordinate in_destination_height,
		guiCoordinate in_source_x, guiCoordinate in_source_y,
		guiCoordinate in_source_width, guiCoordinate in_source_height,
		void* in_source_bitmap);
static void drvBitBltFromRGB565(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
		guiCoordinate in_destination_width, guiCoordinate in_destination_height,
		guiCoordinate in_source_x, guiCoordinate in_source_y,
		guiCoordinate in_source_width, guiCoordinate in_source_height,
		void* in_source_bitmap);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static DMA2D_HandleTypeDef l_dma2d_handle;

// selected background/foreground colors
static guiColor		l_background_color;
static guiColor		l_foreground_color;
static uint16_t		l_device_background_color;
static uint16_t		l_device_foreground_color;

static uint8_t l_current_layer;


/*****************************************************************************/
/* External global variables                                                 */
/*****************************************************************************/
extern void*	g_gui_screen_pixels;
extern int		g_gui_screen_line_size;    // Size in bytes of a bitmap scanline

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes color graphics renderer
void drvColorGraphicsRendererInitialize(void)
{
	// init dma2d clock
  __HAL_RCC_DMA2D_CLK_ENABLE();

  // init driver
  halColorGraphicsInitialize();

	// init  variables
	l_current_layer = 0;
	guiSetForegroundColor(guiCOLOR_WHITE);
	guiSetBackgroundColor(guiCOLOR_BLACK);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets foreground color for drawing operations
/// @param in_color Color for foreground
void guiSetForegroundColor(guiColor in_color)
{
	l_foreground_color = in_color;
	l_device_foreground_color = RGB888ToRGB565(in_color);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets background color for drawing operations
/// @param in_color Color for background
void guiSetBackgroundColor(guiColor in_color)
{
	l_background_color = in_color;
	l_device_background_color = RGB888ToRGB565(in_color);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the foreground color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void guiSetForegroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	uint16_t* pixel;

	if (GET_ALPHA(l_foreground_color) == 0 || in_x < 0 || in_x >= guiSCREEN_WIDTH || in_y < 0 || in_y >= guiSCREEN_HEIGHT)
		return;

	pixel = (uint16_t*)(g_gui_screen_pixels + in_y * g_gui_screen_line_size + in_x * PIXEL_SIZE);
	*pixel = l_device_foreground_color;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the background color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void guiSetBackgroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	uint16_t* pixel;

	if ( GET_ALPHA(l_background_color) == 0 || in_x < 0 || in_x >= guiSCREEN_WIDTH || in_y < 0 || in_y >= guiSCREEN_HEIGHT)
		return;

	pixel = (uint16_t*)(g_gui_screen_pixels + in_y * g_gui_screen_line_size + in_x * PIXEL_SIZE);
	*pixel = l_device_background_color;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Fill rectangle with foreground color
/// @param in_left Left-Top corner X coordinate
/// @param in_top Left-Top corner Y coordinate
/// @param in_width Width of the rectangle
/// @param in_height height of the rectangle
void drvColorGraphicsFillArea(guiCoordinate in_x1, guiCoordinate in_y1, guiCoordinate in_x2, guiCoordinate in_y2)
{
	void* pixel_address;
	uint16_t* pixel;
  guiCoordinate x, y;
	uint32_t width, height;

	// check coordinate
	if (in_x1 > guiSCREEN_WIDTH)
		in_x1 = guiSCREEN_WIDTH - 1;

	if (in_y1 > guiSCREEN_HEIGHT)
		in_y1 = guiSCREEN_HEIGHT - 1;

	// check size
	if (in_x2 >= guiSCREEN_WIDTH)
		in_x2 = guiSCREEN_WIDTH - 1;

	if (in_y2 >= guiSCREEN_HEIGHT)
		in_y2 = guiSCREEN_HEIGHT - 1;

	width = in_x2 - in_x1 + 1;
	height = in_y2 - in_y1 + 1;
/*
	for (y = in_y1; y <= in_y2; y++)
	{
		pixel = (uint16_t*)(g_gui_screen_pixels + y * g_gui_screen_line_size + in_x1 * 2);
		for (x = in_x1; x <= in_x2; x++)
		{
			*pixel++ = 0xf000;
		}
	}
*/
	pixel_address = LAYER_ADDRESS(l_current_layer) + PIXEL_SIZE * guiSCREEN_WIDTH * in_y1 + in_x1 * PIXEL_SIZE;
	drvColorGraphicsFillBuffer(pixel_address, width, height, guiSCREEN_WIDTH - width, l_foreground_color);
}

void guiDrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
		yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
		curpixel = 0;

	uint16_t* pixel;

	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1)
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1)
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay)
	{
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	}
	else
	{
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		pixel = (uint16_t*)(g_gui_screen_pixels + y * g_gui_screen_line_size + x * 2);
		*pixel = 0xf;

		num += numadd;
		if (num >= den)
		{
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}


void drvColorGraphicsBitBltFromResource(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
	guiCoordinate in_destination_width, guiCoordinate in_destination_height,
	guiCoordinate in_source_x, guiCoordinate in_source_y,
	guiCoordinate in_source_width, guiCoordinate in_source_height,
	sysResourceAddress in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	switch(in_source_bit_per_pixel)
	{
		case 1:
			drvBitBltFrom1BPP(in_destination_x, in_destination_y,
					in_destination_width, in_destination_height,
					in_source_x, in_source_y,
					in_source_width, in_source_height,
					drvGetResourcePhysicalAddress(in_source_bitmap));
			break;

		case 16:
			drvBitBltFromRGB565(in_destination_x, in_destination_y,
					in_destination_width, in_destination_height,
					in_source_x, in_source_y,
					in_source_width, in_source_height,
					drvGetResourcePhysicalAddress(in_source_bitmap));
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a monochrome bitmap from the memory
/// @param in_x Top-Left X coordinate of the bitmap target area
/// @param in_y Top-Left Y coordinate of the bitmap target area
/// @param in_width Width of the bitmap to display (can be smaller than the real width of the bitmap)
/// @param in_height Height of the bitmap to display (can be smaller than the real height of the bitmap)
/// @param in_resource_address Resource address of the bitmap to display
void drvColorGraphicsBitBlt(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
	guiCoordinate in_destination_width, guiCoordinate in_destination_height,
	guiCoordinate in_source_x, guiCoordinate in_source_y,
	guiCoordinate in_source_width, guiCoordinate in_source_height,
	void* in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	switch(in_source_bit_per_pixel)
	{
		case 1:
			drvBitBltFrom1BPP(in_destination_x, in_destination_y,
					in_destination_width, in_destination_height,
					in_source_x, in_source_y,
					in_source_width, in_source_height,
					in_source_bitmap);
			break;
	}
}


/*****************************************************************************/
/* Local function implementation                                             */
/*****************************************************************************/

static void drvColorGraphicsFillBuffer(void *in_destination, uint32_t in_xsize, uint32_t in_ysize, uint32_t in_offset, guiColor in_color)
{
  // Init register to memory mode
  l_dma2d_handle.Instance						= DMA2D;
	l_dma2d_handle.Init.Mode        	= DMA2D_R2M;
	l_dma2d_handle.Init.ColorMode   	= DMA2D_RGB565;
	l_dma2d_handle.Init.OutputOffset	= in_offset;

  // DMA2D Initialization
	if(HAL_DMA2D_Init(&l_dma2d_handle) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&l_dma2d_handle, l_current_layer) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&l_dma2d_handle, in_color, (uint32_t)in_destination, in_xsize, in_ysize) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&l_dma2d_handle, 10);
      }
    }
  }
}

static void drvBitBltFromRGB565(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
		guiCoordinate in_destination_width, guiCoordinate in_destination_height,
		guiCoordinate in_source_x, guiCoordinate in_source_y,
		guiCoordinate in_source_width, guiCoordinate in_source_height,
		void* in_source_bitmap)
{
	void* destination_address = LAYER_ADDRESS(l_current_layer) + in_destination_y * g_gui_screen_line_size + in_destination_x * PIXEL_SIZE;
	int x,y;

  // Init register to memory mode
	l_dma2d_handle.Init.Mode        	= DMA2D_M2M;
	l_dma2d_handle.Init.ColorMode   	= DMA2D_RGB565;
	l_dma2d_handle.Init.OutputOffset	= 0;

	l_dma2d_handle.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	l_dma2d_handle.LayerCfg[1].InputAlpha = 0;//0xff;
	l_dma2d_handle.LayerCfg[1].InputColorMode = CM_RGB565;
	l_dma2d_handle.LayerCfg[1].InputOffset = 0;

	l_dma2d_handle.Instance						= DMA2D;

  // DMA2D Initialization
	if(HAL_DMA2D_Init(&l_dma2d_handle) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&l_dma2d_handle, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&l_dma2d_handle, (uint32_t)(in_source_bitmap), (uint32_t)destination_address, in_destination_width, in_destination_height) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&l_dma2d_handle, 10);
      }
    }
  }
}

static void drvBitBltFrom1BPP(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
		guiCoordinate in_destination_width, guiCoordinate in_destination_height,
		guiCoordinate in_source_x, guiCoordinate in_source_y,
		guiCoordinate in_source_width, guiCoordinate in_source_height,
		void* in_source_bitmap)
{
	uint16_t row_byte_count;
	uint8_t bitmap_data;
	uint16_t bitmap_x;
	uint16_t bitmap_y;
	uint8_t* source_pixel;

	row_byte_count = (in_source_width + 7) / 8;
	for (bitmap_y = 0; bitmap_y < in_destination_height; bitmap_y++)
	{
		source_pixel = (uint8_t*)in_source_bitmap + row_byte_count * (bitmap_y + in_source_y) + in_source_x / 8;
		bitmap_data = *source_pixel;
		bitmap_data <<= (in_source_x % 8);

		for (bitmap_x = 0; bitmap_x < in_destination_width; bitmap_x++)
		{
			if (((bitmap_x + in_source_x) % 8) == 0)
				bitmap_data = *source_pixel++;

			if ((bitmap_data & 0x80) == 0)
			{
				guiSetBackgroundPixel(in_destination_x + bitmap_x, in_destination_y + bitmap_y);
			}
			else
			{
				guiSetForegroundPixel(in_destination_x + bitmap_x, in_destination_y + bitmap_y);
			}

			bitmap_data <<= 1;
		}
	}
}

