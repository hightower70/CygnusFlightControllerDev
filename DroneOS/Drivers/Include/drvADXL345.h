/*****************************************************************************/
/* ADXL345 Acceleration sensor driver                                        */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __drvADXL345_h
#define __drvADXL345_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <drvI2CMaster.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvADXL345Init(drvI2CMasterModule* in_imu_i2c);



#endif
