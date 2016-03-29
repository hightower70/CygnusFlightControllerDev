/*****************************************************************************/
/* System (flash) files content and information table                        */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <cfgSettingsConstants.h>
#include <fileSystemFiles.h>


/*****************************************************************************/
/* File storage                                                              */
/*****************************************************************************/

const uint8_t g_settings_xml[cfgSETTINGS_XML_FILE_LENGTH] =
{
#include "cfgSettingsXML.inl"
};

const uint8_t g_settings_default_data[cfgSETTINGS_BINARY_FILE_LENGTH] =
{
#include "cfgSettingsDefaultData.inl"
};

/*****************************************************************************/
/* File information table                                                    */
/*****************************************************************************/
fileInternalFileTableEntry g_system_files_info_table[] =
{
	{ "SettingsXML", g_settings_xml, cfgSETTINGS_XML_FILE_LENGTH },
	{ "SettingsData", 	g_settings_default_data, cfgSETTINGS_BINARY_FILE_LENGTH },
	{ sysNULL, 0, 0 }
};
