/*****************************************************************************/
/* Communication driver and GCS communication manager                        */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <comManager.h>

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static bool l_task_stop = false;

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize communication manager
void comManagerInit(void)
{

}


void comManagerTask(void)
{
	while(true)
	{

	}
}

void comSendPacket(void)
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stops communication manager task
void comTaskStop()
{
	l_task_stop = true;

}