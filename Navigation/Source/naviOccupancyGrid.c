/*****************************************************************************/
/* Occupancy Grid for 2D navigation                                          */
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
#include <sysTimer.h>
#include <crcMD5.h>

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

// occupancy gid
naviOGElementType g_navi_occupancy_grid[naviOG_GRID_SIZE][naviOG_GRID_SIZE];

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static sysTick l_navi_oc_row_timestamp[naviOG_GRID_SIZE]; // last modified timestamp of a row
static bool l_update_active;
static naviOGCoordinate l_update_region_left;
static naviOGCoordinate l_update_region_top;
static naviOGCoordinate l_update_region_right;
static naviOGCoordinate l_update_region_bottom;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize occupancy grid
void naviOGInitialize(void)
{
	naviOGCoordinate x, y;

	// clean occupancy grid
	for (y = 0; y < naviOG_GRID_SIZE; y++)
	{
		l_navi_oc_row_timestamp[y] = 0;

		for (x = 0; x < naviOG_GRID_SIZE; x++)
		{
			g_navi_occupancy_grid[x][y] = naviOG_UNKNOWN;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Opens occupancy grid for update
/// @param in_left Left coordinate of the region to be updated
/// @param in_top Top coordinate of the region to be updated
/// @param in_right Right coordinate of the region to be updated
/// @param in_bottom Bottom coordinate of the region to be updated
/// @retval Updated is opened successfuly. (false is it was already opened, therefore can't be opened again)
bool naviOGUpdateOpen(naviOGCoordinate in_left, naviOGCoordinate in_top, naviOGCoordinate in_right, naviOGCoordinate in_bottom)
{
	if (l_update_active)
		return false;

	// check coordinates
	sysASSERT(in_right < naviOG_GRID_SIZE && in_bottom < naviOG_GRID_SIZE && in_left <= in_right && in_top <= in_bottom);

	// flag update start
	l_update_active = true;

	// store update region
	l_update_region_left = in_left;
	l_update_region_top = in_top;
	l_update_region_right = in_right;
	l_update_region_bottom = in_bottom;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Closes grid update procedure
void naviOGUpdateClose(void)
{
	naviOGCoordinate y;
	sysTick current_timestamp = sysGetSystemTick();
	
	// update timstamps
	for (y = l_update_region_top; y <= l_update_region_bottom; y++)
		l_navi_oc_row_timestamp[y] = current_timestamp;

	// flag update finish
	l_update_active = false;
}

/*****************************************************************************/
/* File handling function implementation                                     */
/*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
/// @brief Handles file requests from the system file manager. The configuration value data file has double buffer therefore it needs a special access.
/// @param in_request File operation request type
/// @param in_buffer Buffer used for data transfer
/// @param in_buffer_length Length of the buffer in bytes used for data transfer
/// @param in_start_position The position within the file where operation must be started
/// @return True if file operation was success
bool naviOGFileHandler(fileCallbackRequest in_request, void* in_buffer, uint16_t in_buffer_length, uint16_t in_start_pos)
{
	switch (in_request)
	{
		// Get length of the file
		case fileCF_GetLength:
			*((uint32_t*)in_buffer) = naviOG_GRID_SIZE * naviOG_GRID_SIZE;
			return true;

			// Get MD5 checksum
		case fileCF_GetMD5:
		{
			crcMD5State md5_state;

			crcMD5Open(&md5_state);
			crcMD5Update(&md5_state, (uint8_t*)l_navi_oc_row_timestamp, sizeof(l_navi_oc_row_timestamp));
			crcMD5Close(&md5_state, (crcMD5Hash*)in_buffer);
		}
		return true;

		// gets content
		case fileCF_GetContent:
			return true;

			// reads data block
		case fileCF_ReadBlock:
			return false;
	}

}

