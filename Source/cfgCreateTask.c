/*****************************************************************************/
/* Creates all task                                                          */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>

/*****************************************************************************/
/* Task function prototypes                                                  */
/*****************************************************************************/
extern void tskHeartbeat(void* in_argument);


/*****************************************************************************/
/* Task creation function                                                    */
/*****************************************************************************/
void cfgCreateTasks(void)
{
  sysTaskCreate(tskHeartbeat, "HeartBeat", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
}
