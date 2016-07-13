/*****************************************************************************/
/* USB HID Communication HAL routines                                        */
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
#include <halHID.h>
#include <sysRTOS.h>
#include <comHID.h>

/*****************************************************************************/
/* Local Function prototypes                                                 */
/*****************************************************************************/
static int8_t CUSTOM_HID_Init_FS     (void);
static int8_t CUSTOM_HID_DeInit_FS   (void);
static int8_t CUSTOM_HID_OutEvent_FS (uint8_t event_idx, uint8_t state);
static void CUSTOM_HID_InEvent_FS  (void);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
extern USBD_CUSTOM_HID_ItfTypeDef  USBD_CustomHID_fops_FS;

__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
		0x06, 0x00, 0xFF,       // Usage Page = 0xFF00 (Vendor Defined Page 1)
		0x09, 0x01,             // Usage (Vendor Usage 1)
		0xA1, 0x01,             // Collection (Application)
		0x19, 0x01,             //      Usage Minimum
		0x29, 0x40,             //      Usage Maximum   //64 input usages total (0x01 to 0x40)
		0x15, 0x01,             //      Logical Minimum (data bytes in the report may have minimum value = 0x00)
		0x25, 0x40,            	//      Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
		0x75, 0x08,             //      Report Size: 8-bit field size
		0x95, 0x40,             //      Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
		0x81, 0x00,             //      Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
		0x19, 0x01,             //      Usage Minimum
		0x29, 0x40,             //      Usage Maximum   //64 output usages total (0x01 to 0x40)
		0x91, 0x00,             //      Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
		0xC0					// End Collection
}; 

/* USB handler declaration */
/* Handle for USB Full Speed IP */
USBD_HandleTypeDef  *hUsbDevice_0;
USBD_HandleTypeDef hUsbDeviceFS;

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS = 
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS,
  CUSTOM_HID_InEvent_FS
};

static halHIDConfigInfo l_hid_config;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes USB HID interface
void halHIDInit(void)
{
  /* Init Device Library,Add Supported Class and Start the library*/
  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);

  USBD_RegisterClass(&hUsbDeviceFS, &USBD_CUSTOM_HID);

  USBD_CUSTOM_HID_RegisterInterface(&hUsbDeviceFS, &USBD_CustomHID_fops_FS);

  USBD_Start(&hUsbDeviceFS);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Send the report to the Host
/// @param  in_report Pointer to the report data content
/// @param  in_length Length of the report in bytes
/// @retval True if packet sending was started, false when packet sending failed
bool halHIDSendReport( uint8_t *in_report, uint16_t in_length)
{
  return (USBD_CUSTOM_HID_SendReport(hUsbDevice_0, in_report, in_length) == USBD_OK);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns true if device is connected to the host and enumerated
bool halHIDIsConnected(void)
{
  return (hUsbDevice_0->dev_state == USBD_STATE_CONFIGURED);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Changes HID configuration
void halHIDConfig(halHIDConfigInfo* in_config_info)
{
	l_hid_config = *in_config_info;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes halHIDConfigInfo struct
void halHIDConfigInfoInit(halHIDConfigInfo* in_config_info)
{
	sysMemZero(in_config_info, sizeof(halHIDConfigInfo));
}
/*****************************************************************************/
/* Custom HID handling functions                                             */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief  DeInitializes the CUSTOM HID media low layer
/// @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */ 
  return (0);
  /* USER CODE END 5 */ 
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Manage the CUSTOM HID class events
/// @param  event_idx: event index
/// @param  state: event state
/// @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
static int8_t CUSTOM_HID_OutEvent_FS  (uint8_t event_idx, uint8_t state)
{ 
	USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDevice_0->pClassData;

  sysBeginInterruptRoutine();

	 if(l_hid_config.PacketReceivedCallback != sysNULL)
		 l_hid_config.PacketReceivedCallback(hhid->Report_buf, sysInterruptParam() );

	sysEndInterruptRoutine();

  return (0);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the CUSTOM HID media low layer
/// @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
static int8_t CUSTOM_HID_Init_FS(void)
{
  hUsbDevice_0 = &hUsbDeviceFS;

  return (0);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Callback for transmitter ready event
static void CUSTOM_HID_InEvent_FS(void)
{
  sysBeginInterruptRoutine();

	 if(l_hid_config.TransmitterEmptyCallback!= sysNULL)
		 l_hid_config.TransmitterEmptyCallback(sysInterruptParam());

  sysEndInterruptRoutine();
}

