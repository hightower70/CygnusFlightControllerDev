/*****************************************************************************/
/* Lidar Driver                                                              */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvLidar_h
#define __drvLidar_h

#include <halIODefinitions.h>

#if defined(halLIDAR_NUMBER_OF_DISTANCES_PER_SCAN)
#define drvLIDAR_NUMBER_OF_DISTANCES_PER_SCAN halLIDAR_NUMBER_OF_DISTANCES_PER_SCAN
#else
#define drvLIDAR_NUMBER_OF_DISTANCES_PER_SCAN 360
#endif


#define drvLIDAR_INVALID_DISTANCE 0xffff

void drvLidarInitialize(void);
uint16_t drvLidarGetDistance(uint16_t in_angle);

extern void drvLidarScanIsCompleteCallback(void);

#endif