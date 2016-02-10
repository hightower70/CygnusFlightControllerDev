/*****************************************************************************/
/* High resolution (1us resolution) timer routines                           */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __sysHighresTimer_h
#define __sysHighresTimer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef uint32_t sysHighresTimestamp;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void sysHighresTimerInit(void);
sysHighresTimestamp sysHighresTimerGetTimestamp(void);
void sysHighresTimerDelay(uint32_t in_delay_us);
sysHighresTimestamp sysHighresTimerGetTimeSince(sysHighresTimestamp in_start_time);

#endif
