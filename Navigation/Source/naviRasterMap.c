/*****************************************************************************/
/* Raster map for 2D navigation emulation functions                          */
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
#include <sysRTOS.h>
#include <naviOccupancyGrid.h>
#include <naviRasterMap.h>
#include <stdio.h>

/*****************************************************************************/
/* Windows bitmap related data structs and constants                         */
/*****************************************************************************/

#include <sysPackedStructStart.h>

# define naviBF_TYPE 0x4D42             /* "MB" */

// BMP file header structure
typedef struct                       
{
	unsigned short bfType;           /* Magic number for file */
	unsigned int   bfSize;           /* Size of file */
	unsigned short bfReserved1;      /* Reserved */
	unsigned short bfReserved2;      /* ... */
	unsigned int   bfOffBits;        /* Offset to bitmap data */
} naviBITMAPFILEHEADER;

// BMP file info structure
typedef struct                       
{
	unsigned int   biSize;           /* Size of info header */
	int            biWidth;          /* Width of image */
	int            biHeight;         /* Height of image */
	unsigned short biPlanes;         /* Number of color planes */
	unsigned short biBitCount;       /* Number of bits per pixel */
	unsigned int   biCompression;    /* Type of compression to use */
	unsigned int   biSizeImage;      /* Size of image data */
	int            biXPelsPerMeter;  /* X pixels per meter */
	int            biYPelsPerMeter;  /* Y pixels per meter */
	unsigned int   biClrUsed;        /* Number of colors used */
	unsigned int   biClrImportant;   /* Number of important colors */
} naviBITMAPINFOHEADER;

// Constants for the biCompression field...
#  define naviBI_RGB       0             /* No compression - straight BGR data */
#  define naviBI_RLE8      1             /* 8-bit run-length compression */
#  define naviBI_RLE4      2             /* 4-bit run-length compression */
#  define naviBI_BITFIELDS 3             /* RGB bitmap with RGB masks */

// Colormap entry structure
typedef struct                       
{
	unsigned char  rgbBlue;          /* Blue value */
	unsigned char  rgbGreen;         /* Green value */
	unsigned char  rgbRed;           /* Red value */
	unsigned char  rgbReserved;      /* Reserved */
} naviRGBQUAD;

// Bitmap information structure
typedef struct                       
{
	naviBITMAPINFOHEADER bmiHeader;      /* Image header */
	naviRGBQUAD          bmiColors[256]; /* Image colormap */
} naviBITMAPINFO;

#include <sysPackedStructEnd.h>

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
uint8_t g_raster_map[naviOG_GRID_SIZE][naviOG_GRID_SIZE];

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize raster map
void naviRasterMapInitialize(void)
{
	naviRasterMapLoad("d:\\Projects\\SLAM\\alaprajz.bmp ");
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Loads bitmap as raster map
/// @param in_file_name Bitmap file name
void naviRasterMapLoad(sysString in_file_name)
{
	bool success = true;
	FILE* bitmap_file = NULL;
	naviBITMAPFILEHEADER file_header;
	naviBITMAPINFOHEADER info_header;
	naviRGBQUAD palette[256];
	uint8_t red, green, blue;
	uint8_t byte_buffer;
	uint8_t byte_buffer_length;
	uint8_t palette_index;
	int x, y;
	uint8_t grey;
	int stride_length;
	int padding_length;
	uint32_t padding_buffer;

	// init map
	for (y = 0; y < naviOG_GRID_SIZE; y++)
	{
		for (x = 0; x < naviOG_GRID_SIZE; x++)
		{
			g_raster_map[x][y] = 1;
		}
	}

	sysMemZero(&file_header, sizeof(file_header));
	sysMemZero(&info_header, sizeof(info_header));

	// open bitmap file
	bitmap_file = fopen(in_file_name, "rb");

	if (bitmap_file == NULL)
		success = false;

	// load bitmap file header
	if (success)
	{
		if (fread(&file_header, sizeof(file_header), 1, bitmap_file) != 1)
			success = false;
	}

	if (success && file_header.bfType != naviBF_TYPE)
		success = false;

	// load info header
	if (success)
	{
		if (fread(&info_header, sizeof(info_header), 1, bitmap_file) != 1)
			success = false;
	}

	// check size
	if (success && info_header.biSize != sizeof(info_header))
		success = false;

	// only uncompressed bitmaps are supported
	if (info_header.biCompression != BI_RGB)
		success = false;

	// read palette
	if (success && info_header.biClrUsed > 0)
	{
		if (fread(&palette, sizeof(RGBQUAD), info_header.biClrUsed, bitmap_file) != info_header.biClrUsed)
			success = false;
	}

	// locate and load pixel data in the file
	if (success)
	{
		fseek(bitmap_file, file_header.bfOffBits, SEEK_SET);

		stride_length = info_header.biBitCount * info_header.biWidth / 8; // row length in bytes
		padding_length = stride_length % 4;

		for (y = 0; y < info_header.biHeight; y++)
		{
			for (x = 0; x < info_header.biWidth; x++)
			{
				byte_buffer = 0;
				byte_buffer_length = 0;

				switch (info_header.biBitCount)
				{
					case 1:
					case 2:
					case 4:
					case 8:
						// read data into buffer
						if (byte_buffer_length == 0)
						{
							if (fread(&byte_buffer, sizeof(byte_buffer), 1, bitmap_file) != 1)
								success = false;
							else
							{
								byte_buffer_length = 8;
							}
						}

						// get palette index
						switch (info_header.biBitCount)
						{
							case 1:
								palette_index = (byte_buffer & 0x80) >> 7;
								byte_buffer <<= 1;
								byte_buffer_length -= 1;
								break;

							case 2:
								palette_index = (byte_buffer & 0xC0) >> 6;
								byte_buffer <<= 2;
								byte_buffer_length -= 2;
								break;

							case 4:
								palette_index = (byte_buffer & 0xF0) >> 4;
								byte_buffer <<= 4;
								byte_buffer_length -= 4;
								break;

							case 8:
							default:
								palette_index = byte_buffer;
								byte_buffer_length = 0;
								break;
						}

						// get pixel color
						red = palette[palette_index].rgbRed;
						blue = palette[palette_index].rgbBlue;
						green = palette[palette_index].rgbGreen;
						break;

					case 24:
						if (success && fread(&blue, sizeof(blue), 1, bitmap_file) != 1)
							success = false;
						if (success && fread(&green, sizeof(green), 1, bitmap_file) != 1)
							success = false;
						if (success && fread(&red, sizeof(red), 1, bitmap_file) != 1)
							success = false;
						break;
				}

				// calculate grey value
				// grey = red * 0.3 + green * 0.59 + blue * 0.11
				grey = (uint8_t)(((int)red * 100 * 30 + green * 100 * 59 + blue * 100 * 11) / 10000);

				if (grey > 127)
					g_raster_map[x][info_header.biHeight - y - 1] = 1;
				else
					g_raster_map[x][info_header.biHeight - y - 1] = 0;
			}
		}

		// skip padding
		if (success && padding_length > 0)
			fread(&padding_buffer, sizeof(uint8_t), padding_length, bitmap_file);
	}

	if (bitmap_file != NULL)
		fclose(bitmap_file);
}