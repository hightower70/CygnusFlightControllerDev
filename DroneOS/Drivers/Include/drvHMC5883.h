/*****************************************************************************/
/* HMC5883 Magnetic compass sensor driver                                    */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvHMC5883_h
#define __drvHMC5883_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <drvI2CMaster.h>
#include <drvIMU.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
uint32_t drvHMC5883Setup(drvI2CMasterModule* in_imu_i2c, drvIMUSetupFunction in_function);
void drvHMC5883Read(drvI2CMasterModule* in_imu_i2c);

#endif
