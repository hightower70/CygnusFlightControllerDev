/*****************************************************************************/
/* Communication manager functions                                           */
/*                                                                           */
/* Copyright (C) 20152016 Laszlo Arvai                                       */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __comManager_h
#define __comManager_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <comManager.h>
#include <comInterfaces.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void comManagerInit(void);
void comAddInterface(comInterfaceDescription* in_interface);


#endif