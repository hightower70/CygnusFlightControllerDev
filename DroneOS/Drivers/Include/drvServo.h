/*****************************************************************************/
/* Eight channel servo output driver                                         */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvServo_h
#define __drvServo_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvSERVO_CHANNEL_COUNT 8

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvServoInit(void);


#endif
