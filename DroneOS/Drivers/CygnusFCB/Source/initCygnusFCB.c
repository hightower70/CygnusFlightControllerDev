#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "semihosting.h"

#include <usb_device.h>
#include <usbd_desc.h>
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_CAN2_Init(void);
static void MX_DAC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM1_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);


extern void drvServoInit(void);
extern void drvStatLEDInit(void);
void drvStatLEDSetDim(uint8_t in_dim);
void drvIMUInit(void);

GPIO_InitTypeDef  GPIO_InitStruct;
USBD_HandleTypeDef USBD_Device;

uint8_t l_state_led = 0;

int main1(void)
{
  unsigned int _btn_count = 0;

  HAL_Init();

  SystemClock_Config();
  MX_GPIO_Init();


  //BSP_IO_Init();

/**********************************************************************************
*
* This enables the peripheral clock to the GPIOD module.  This is stated in
* the beginning of the stm32f4xx.gpio.c source file.
*
**********************************************************************************/

//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
//  __HAL_RCC_GPIOB_CLK_ENABLE();


/**********************************************************************************
*
* This block of code defines the properties of the GPIO port.
* The different options of each item can be found in the stm32f4xx_gpio.h header file
* Every GPIO configuration should have the following initialized
*
* GPIO_InitStruct.GPIO_Pin
* GPIO_InitStruct.GPIO_Mode
* GPIO_InitStruct.GPIO_Speed
* GPIO_InitStruct.GPIO_OType
* GPIO_InitStruct.GPIO_PuPd
* GPIO_Init(GPIOx, &GPIO_InitStruct); (x represents port - A, B, C, D, E, F, G, H, I)
*
**********************************************************************************/

//GPIO_InitStruct.GPIO_Pin configures the pins that will be used.
//In this case we will use the LED's off of the discovery board which are on
//PortD pins 12, 13, 14 and 15
//GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_12;
//GPIO_InitStruct.Pin = GPIO_PIN_15;

//PIO_InitStruct.GPIO_Mode configures the pin mode the options are as follows
// GPIO_Mode_IN (Input Mode)
// GPIO_Mode_OUT (Output Mode)
// GPIO_Mode_AF (Alternate Function)
// GPIO_Mode_AN (Analog Mode)
//GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

//GPIO_InitStruct.GPIO_Speed configures the clock speed, options are as follows
// GPIO_Speed_2MHz
// GPIO_Speed_25MHz
// GPIO_Speed_50MHz
// GPIO_Speed_100MHz
//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

//GPIO_InitStruct.GPIO_OType configures the pin type, options are as follows
// GPIO_OType_PP (Push/Pull)
// GPIO_OType_OD (Open Drain)
//GPIO_InitStruct.OType = GPIO_OType_PP;

//Configures pullup / pulldown resistors on pin, options are as follows
// GPIO_PuPd_NOPULL (Disables internal pullup and pulldown resistors)
// GPIO_PuPd_UP (Enables internal pullup resistors)
// GPIO_PuPd_DOWN (Enables internal pulldown resistors)
//GPIO_InitStruct.Pull = GPIO_PULLUP;

//This finally passes all the values to the GPIO_Init function
//which takes care of setting the corresponding bits.
//GPIO_Init(GPIOD, &GPIO_InitStruct);
//HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/**********************************************************************************
*
* This enables the peripheral clock to the GPIOA module.  This is stated in
* the beginning of the stm32f4xx.gpio.c source file.
*
**********************************************************************************/
//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

/**********************************************************************************
*
* This block of code defines the properties of the GPIOA port.
* We are defining Pin 0 as a digital input with a pulldown resistor
* to detect a high level.  Pin 0 is connected to the 3.3V source
*
**********************************************************************************/
//GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
//GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
//GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
//GPIO_Init(GPIOA, &GPIO_InitStruct);

/**********************************************************************************
*
* This block of code blinks all four LED's on initial startup
*
* **********************************************************************************/
//GPIO_SetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
//GPIO_SetBits(GPIOB, GPIO_PIN_15);
//Delay(0xFFFFF);
//GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
//GPIO_ResetBits(GPIOB, GPIO_PIN_15);
//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);

MX_USB_DEVICE_Init();

drvServoInit();
drvStatLEDInit();

drvIMUInit();

//SH_SendString("Hello\n\r");

    while(1)
    {
    	for(l_state_led=0;l_state_led < 9;l_state_led++)
    	{
    		drvStatLEDSetDim(l_state_led);
    	  HAL_Delay(50);
    	}

    	for(l_state_led=9;l_state_led > 0;l_state_led--)
    	{
    		drvStatLEDSetDim(l_state_led);
    	  HAL_Delay(50);
    	}

    	//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
  	/*
      _btn_count++;
          if (_btn_count > 0x8FFFF)
          {
              //GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
            //GPIO_ToggleBits(GPIOB, GPIO_Pin_15);
        	  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
              _btn_count = 0;
          }*/
    }
}



void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

}
