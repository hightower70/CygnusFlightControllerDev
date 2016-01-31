/*****************************************************************************/
/* String handling functions                                                 */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __String_h
#define __String_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

// string conversion options
#define TS_NO_ZERO_BLANKING (1<<1)
#define TS_RIGHT_ADJUSTMENT (1<<2)
#define TS_DISPLAY_MINUS_SIGN (1<<3)

// parser options
#define TS_ACCEPT_MINUS_SIGN (1<<1)

#define charIsDigit(x) ((x)>='0' && ((x))<='9')
#define charIsHexDigit(x) (((x)>='0' && ((x))<='9') || ((x)>='a' && ((x))<='f') || ((x)>='A' && ((x))<='F'))

// line end types
typedef enum
{
	rtosString_LE_CR,
	rtosString_LE_LF,
	rtosString_LE_CRLF,
	rtosString_LE_LFCR
}  rtosStringLineEnd;

///////////////////////////////////////////////////////////////////////////////
// ASCII control codes
#define rtosASCII_NUL	0
#define rtosASCII_SOH	1
#define rtosASCII_STX	2
#define rtosASCII_ETX	3
#define rtosASCII_EOT 4
#define rtosASCII_ENQ	5
#define rtosASCII_0K	6
#define rtosASCII_BEL	7
#define rtosASCII_BS	8
#define rtosASCII_HT	9
#define rtosASCII_LF	10
#define rtosASCII_VT	11
#define rtosASCII_FF	12
#define rtosASCII_CR	13
#define rtosASCII_SO	14
#define rtosASCII_SI	15
#define rtosASCII_DLE 16
#define rtosASCII_DC1	17
#define rtosASCII_DC2	18
#define rtosASCII_DC3	19
#define rtosASCII_DC4	20
#define rtosASCII_NAK	21
#define rtosASCII_SYN	22
#define rtosASCII_ETB	23
#define rtosASCII_CAN	24
#define rtosASCII_EM	25
#define rtosASCII_SUB	26
#define rtosASCII_ESC	27
#define rtosASCII_FS	28
#define rtosASCII_GS	29
#define rtosASCII_RS	30
#define rtosASCII_US	31

///////////////////////////////////////////////////////////////////////////////
// Functions
//rtosStringLength strGetLength( rtosString in_string );
//rtosStringLength strGetConstLength( rtosConstString in_string );
//dosASCIIChar strUnicodeToASCIIChar( dosWord in_unicode );
//
rtosChar strCharToUpper( rtosChar in_char );
rtosChar strCharToLower( rtosChar in_char );

// comparision
int strCompareConstString(rtosString in_string1, rtosConstString in_string2);
int strCompareConstStringNoCase(rtosString in_string1, rtosConstString in_string2);
//int strCompareString(rtosString in_string1, rtosString in_string2);
//int strCompareStringNoCase(rtosString in_string1, rtosString in_string2);


// String concatenation, copy, fill functions
rtosStringLength strCopyString( rtosString in_destination, rtosStringLength in_destination_size, rtosStringLength in_destination_pos, rtosString in_source );
rtosStringLength strCopyConstString( rtosString in_destination, rtosStringLength in_destination_size, rtosStringLength in_destination_pos, rtosConstString in_source );
//void strInt16ToString( rtosString in_buffer, dosByte in_buffer_length, dosInt16 in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );
//void strFillString( rtosString in_destination, rtosStringLength in_destination_size, rtosChar in_char, rtosStringLength in_start_pos, dosWord in_requierd_length );
//dosInt16 strFindChar( rtosString in_buffer, rtosChar in_char );
//void strSetLength( rtosString in_destination, rtosStringLength in_destination_size, rtosChar in_char, rtosStringLength in_required_length );
//void strAppendInt16ToString( rtosString in_buffer, dosByte in_buffer_length, dosInt16 in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );
rtosStringLength strAppendConstString( rtosString in_destination, rtosStringLength in_destination_size, rtosConstString in_source );
//void strAppendString( rtosString in_destination, rtosStringLength in_destination_size, rtosString in_source );
//rtosStringLength strCopyCharacter(rtosString in_destination, rtosStringLength in_destination_size, rtosStringLength in_pos, rtosChar in_char);

// hex conversion
//rtosChar strNibbleToHexDigit( dosByte in_nibble );
//rtosStringLength strWordToHexStringPos( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength in_pos, dosWord in_value );
//void strWordToHexString( rtosString in_buffer, rtosStringLength in_buffer_length, dosWord in_value );
//void strAppendWordToHexString( rtosString in_buffer, rtosStringLength in_buffer_length, dosWord in_value );

// byte conversion
//void strByteToHexStringPos( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength in_pos, dosWord in_value );
//void strByteToHexString( rtosString in_buffer, rtosStringLength in_buffer_length, dosWord in_value );
//void strAppendByteToHexString( rtosString in_buffer, rtosStringLength in_buffer_length, dosWord in_value );

// word conversion
//rtosStringLength strWordToStringPos( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength in_pos, dosWord in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );
//rtosStringLength strWordToString( rtosString in_buffer, rtosStringLength in_buffer_length, dosWord in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );
//rtosStringLength strAppendWordToString( rtosString in_buffer, rtosStringLength in_buffer_length, dosWord in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );

// dword conversion
//void strDWordToString( rtosString in_buffer, rtosStringLength in_buffer_length, dosDWord in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );
//rtosStringLength strDWordToHexStringPos( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength in_pos, dosDWord in_value );
//rtosStringLength strDWordToStringPos( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength in_pos, dosDWord in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );

// int32 conversion
//void strInt32ToString( rtosString in_buffer, rtosStringLength in_buffer_length, dosInt32 in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );
//void strAppendInt32ToString( rtosString in_buffer, rtosStringLength in_buffer_length, dosInt32 in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );
//rtosStringLength strInt32ToStringPos( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength in_pos, dosInt32 in_value, dosByte in_field_length, dosByte in_precision, dosByte in_options );

// parser routines
void strSkipWhitespaces(rtosString in_string, rtosStringLength in_buffer_length, rtosStringLength* in_index);
//void strSkipLineEnd(rtosString in_string, rtosStringLength in_buffer_length, rtosStringLength* in_index, rtosStringLineEnd in_line_end);
//int strCheckForTokenNoCase(rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* inout_index, dosBool* in_success, rtosConstString in_expected_tokens[]);
//void strCheckForConstStringNoCase(rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* inout_index, dosBool* in_success, rtosConstString in_string );
//void strCheckForConstString(rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* inout_index, dosBool* in_success, rtosConstString in_string );
void strCheckForSeparator( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* in_index, bool* in_success, rtosChar in_char );
//void strStringToInt16( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* in_index, dosBool* in_success, dosInt16* out_number );
void strStringToWord( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* in_index, bool* in_success, uint16_t* out_number );
//void strStringToDWord( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* in_index, dosBool* in_success, dosDWord* out_number );
void strStringToByte( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* in_index, bool* in_success, uint8_t* out_number );
//void strStringToFixedInt16( rtosString in_buffer, rtosStringLength* in_index, dosBool* in_success, dosInt16* out_number, dosByte in_fixed_point, dosWord in_divisor );
//void strStringToFixedWord( rtosString in_buffer, rtosStringLength* in_index, dosBool* in_success, dosWord* out_number, dosByte in_fixed_point, dosWord in_divisor );
//void strStringToFixedDWord( rtosString in_buffer, rtosStringLength* in_index, dosBool* in_success, dosDWord* out_number, dosByte in_fixed_point, dosDWord in_divisor );
//
//dosByte strHexDigitToNibble( rtosChar in_digit, dosBool* in_success );
//void strHexStringToWord( rtosString in_buffer, rtosStringLength* in_index, dosBool* in_success, dosWord* out_number );
//void strHexStringToDWord( rtosString in_buffer, rtosStringLength in_buffer_length, rtosStringLength* in_index, dosBool* in_success, dosDWord* out_number );

#endif
