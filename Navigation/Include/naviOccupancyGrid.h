/*****************************************************************************/
/* Occupancy Grid for 2D navigation                                          */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __naviOccupancyGrid_h
#define __naviOccupancyGrid_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <fileSystemFiles.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define naviOG_GRID_SIZE 1024
#define naviOG_WALL 65500u
#define naviOG_FREE 0u
#define naviOG_UNKNOWN 65535u

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef uint16_t naviOGElementType;
typedef uint16_t naviOGCoordinate;

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
extern naviOGElementType g_navi_occupancy_grid[naviOG_GRID_SIZE][naviOG_GRID_SIZE];

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
bool naviOGFileHandler(fileCallbackRequest in_request, void* in_buffer, uint16_t in_buffer_length, uint16_t in_start_pos);


#endif
