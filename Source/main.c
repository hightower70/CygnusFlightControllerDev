#include <sysConfig.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "semihosting.h"


/* Private function prototypes -----------------------------------------------*/


int main(void)
{
	initSystem();

	  while (1)
	  {
		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12, GPIO_PIN_SET);
		  HAL_Delay(100);
		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12, GPIO_PIN_RESET);

		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13, GPIO_PIN_SET);
		  HAL_Delay(100);
		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13, GPIO_PIN_RESET);

		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_SET);
		  HAL_Delay(100);
		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET);

		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15, GPIO_PIN_SET);
		  HAL_Delay(100);
		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15, GPIO_PIN_RESET);


	  }
}

