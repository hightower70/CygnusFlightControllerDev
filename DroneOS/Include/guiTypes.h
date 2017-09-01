#ifndef __guiTypes
#define __guiTypes

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <drvResources.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

#define guiINVALID_COORDINATE -32768

// Gui drawing mode
#define guiDM_Inverse      (1<<1)
#define guiDM_Transparent  (1<<2)

// Horizontal Text Align
#define guiHA_LEFT					(0)
#define guiHA_CENTER				(1)
#define guiHA_RIGHT					(2)
#define guiHA_MASK					(3)

// Vertical text Align
#define guiVA_TOP						(0)
#define guiVA_CENTER				(1<<2)
#define guiVA_BOTTOM				(2<<2)
#define guiVA_MASK					(3<<2)

///////////////////////////////////////////////////////////////////////////////
// GUI font flags
#define	guiFF_DONTCARE			1				// Don't care or don't know.
#define guiFF_ROMAN					2				// Proportionally spaced fonts with serifs.
#define guiFF_SWISS					3				// Proportionally spaced fonts without serifs.
#define guiFF_MODERN				4				// Fixed-pitch fonts.
#define guiFF_SCRIPT				5				// Script
#define guiFF_DECORATIVE		6				// Decorative
#define guiFF_FIXED					(1<<7)	// Set if the font is fixed
#define guiFF_INVALID				0				// invalid family type

///////////////////////////////////////////////////////////////////////////////
// Types
typedef int16_t guiCoordinate;

#define guiRGBToColor(r,g,b) (((uint32_t)r << 16) + ((uint32_t)g << 8) + b)

#if !defined(guiCOLOR_DEPTH)
#error The color depth of the graphics display (guiCOLOR_DEPTH) must be defined.
#endif

#if (guiCOLOR_DEPTH > 16)
typedef uint32_t guiColor;
#elif (guiCOLOR_DEPTH <= 16) && (guiCOLOR_DEPTH > 8)
typedef uint16_t guiColor;
#else
typedef uint8_t guiColor;
#endif


///////////////////////////////////////////////////////////////////////////////
// GUI rectangle type
typedef struct
{
	guiCoordinate Left;
	guiCoordinate Top;
	guiCoordinate Right;
	guiCoordinate Bottom;
} guiRect;

///////////////////////////////////////////////////////////////////////////////
// GUI size type
typedef struct
{
	guiCoordinate Width;
	guiCoordinate Height;
} guiSize;

///////////////////////////////////////////////////////////////////////////////
// GUI font info
typedef struct
{
	uint8_t							Type;
	uint8_t							Flag;
	uint8_t							Width;
	uint8_t							Height;
	uint8_t							Baseline;
	uint8_t							HorizontalCharacterGap;
	uint8_t							Minascii;
	uint8_t							Maxascii;
	uint16_t						UnicodeCount;
	uint8_t							DefaultCharacter;
	sysResourceAddress	AsciiTableAddress;
	sysResourceAddress	FontAddress;
} guiFontInfo;


#endif
