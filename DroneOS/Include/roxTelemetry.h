/*****************************************************************************/
/* Real time object storage telemetry functions                              */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __roxTelemetry_h
#define __roxTelemetry_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <roxStorage.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void roxTelemetryInitialize(void);
void roxTelemetryTaskStop(void);
void roxTelemetrySetObjectChanged(roxObjectIndex in_object_index);

#endif