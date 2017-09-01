/*****************************************************************************/
/* System (flash) files content and information table                        */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <cfgConstants.h>
#include <fileSystemFiles.h>
#include <cfgStorage.h>

/*****************************************************************************/
/* File storage                                                              */
/*****************************************************************************/

static const uint8_t l_configuration_xml[cfg_XML_DATA_FILE_LENGTH] =
{
#include "cfgXML.inl"
};

static const uint8_t l_configuration_default_data[cfg_VALUE_DATA_FILE_LENGTH] =
{
#include "cfgDefault.inl"
};

static const uint8_t l_configuration_value_info_data[cfg_VALUE_INFO_DATA_FILE_LENGTH] =
{
#include "cfgValueInfo.inl"
};

/*****************************************************************************/
/* File information table                                                    */
/*****************************************************************************/
fileInternalFileTableEntry g_system_files_info_table[] =
{
  { "ConfigurationXML",         (uint8_t*)l_configuration_xml,              sysNULL,                  cfg_XML_DATA_FILE_LENGTH,         fileSFF_READ_ONLY },
  { "DefaultConfigurationData", (uint8_t*)l_configuration_default_data,     sysNULL,                  cfg_VALUE_DATA_FILE_LENGTH,       fileSFF_READ_ONLY },
  { "ConfigurationData",        sysNULL,                                    cfgValueDataFileHandler,  cfg_VALUE_DATA_FILE_LENGTH,       fileSFF_READ_WRITE },
  { "ConfigurationValueInfo",   (uint8_t*)l_configuration_value_info_data,  sysNULL,                  cfg_VALUE_INFO_DATA_FILE_LENGTH,  fileSFF_READ_ONLY },
  { sysNULL,                    sysNULL,                                    sysNULL,                  0,                                0 }
};
