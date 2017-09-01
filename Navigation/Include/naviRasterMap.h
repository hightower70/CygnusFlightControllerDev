/*****************************************************************************/
/* Raster map for 2D navigation emulation functions                          */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __naviRasterMap_h
#define __naviRasterMap_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void naviRasterMapInitialize(void);
void naviRasterMapLoad(sysString in_file_name);


#endif
