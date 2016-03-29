/*****************************************************************************/
/* I/O definitions                                                           */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Used system resources:                                                    */
/*                                                                           */
/*   Servo:        TIM3 - CH4,                                               */
/*                 PC0, PC1, PC2                                             */
/*                                                                           */
/*   Status LED:   TIM12 - CH12                                              */
/*                 PB15                                                      */
/*****************************************************************************/


#ifndef __drvIODefinitions_h
#define __drvIODefinitions_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>



#define SPI_CS_Pin GPIO_PIN_13
#define SPI_CS_GPIO_Port GPIOC
#define SERVO_SEL_A_Pin GPIO_PIN_0
#define SERVO_SEL_A_GPIO_Port GPIOC
#define SERVO_SEL_B_Pin GPIO_PIN_1
#define SERVO_SEL_B_GPIO_Port GPIOC
#define SERVO_SEL_C_Pin GPIO_PIN_2
#define SERVO_SEL_C_GPIO_Port GPIOC
#define MDM_Pin GPIO_PIN_4
#define MDM_GPIO_Port GPIOA
#define AIN3_Pin GPIO_PIN_5
#define AIN3_GPIO_Port GPIOA
#define AIn2_Pin GPIO_PIN_6
#define AIn2_GPIO_Port GPIOA
#define AIN1_Pin GPIO_PIN_7
#define AIN1_GPIO_Port GPIOA
#define VBAT_Pin GPIO_PIN_0
#define VBAT_GPIO_Port GPIOB
#define SERVO_Pin GPIO_PIN_1
#define SERVO_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_10
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_11
#define SDA_GPIO_Port GPIOB
#define STAT_Pin GPIO_PIN_15
#define STAT_GPIO_Port GPIOB
#define PPM_Pin GPIO_PIN_8
#define PPM_GPIO_Port GPIOA
#define SPI_SCK_Pin GPIO_PIN_3
#define SPI_SCK_GPIO_Port GPIOB
#define SPI_MISO_Pin GPIO_PIN_4
#define SPI_MISO_GPIO_Port GPIOB
#define SPI_MOSI_Pin GPIO_PIN_5
#define SPI_MOSI_GPIO_Port GPIOB
#define SCL_EXT_Pin GPIO_PIN_8
#define SCL_EXT_GPIO_Port GPIOB
#define SDA_EXT_Pin GPIO_PIN_9
#define SDA_EXT_GPIO_Port GPIOB

#endif
