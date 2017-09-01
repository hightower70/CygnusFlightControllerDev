/*****************************************************************************/
/* Virtual keyboard codes (used for raw keyboard interface)                  */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __sysVirtualKeyboardCodes_h
#define __sysVirtualKeyboardCodes_h

#define sysVKC_SPECIAL_KEY_FLAG 0x8000
#define sysVKC_IS_SPECIAL_KEY(x) ((x & sysVKC_SPECIAL_KEY_FLAG) != 0)
#define sysVKC_KEY_CODE(x) ((x)&0x7fff)

#define sysVKC_NULL					0x00 // Invalid (NULL) code
#define sysVKC_BACK					0x08
#define sysVKC_TAB					0x09
#define sysVKC_CLEAR				0x0C
#define sysVKC_RETURN				0x0D
#define sysVKC_SHIFT				0x10
#define sysVKC_CONTROL			0x11 // CTRL key
#define sysVKC_MENU					0x12 // ALT key
#define sysVKC_PAUSE				0x13 // PAUSE key
#define sysVKC_CAPITAL			0x14 // CAPS LOCK key
#define sysVKC_KANA					0x15 // Input Method Editor (IME) Kana mode
#define sysVKC_HANGUL				0x15 // IME Hangul mode
#define sysVKC_JUNJA				0x17 // IME Junja mode
#define sysVKC_FINAL				0x18 // IME final mode
#define sysVKC_HANJA				0x19 // IME Hanja mode
#define sysVKC_KANJI				0x19 // IME Kanji mode
#define sysVKC_ESCAPE				0x1B // ESC key
#define sysVKC_CONVERT			0x1C // IME convert
#define sysVKC_NONCONVERT		0x1D // IME nonconvert
#define sysVKC_ACCEPT				0x1E // IME accept
#define sysVKC_MODECHANGE		0x1F // IME mode change request
#define sysVKC_SPACE				0x20 // SPACE key

#define sysVKC_PRIOR				0x21 // PAGE UP key
#define sysVKC_NEXT					0x22 // PAGE DOWN key
#define sysVKC_END					0x23 // END key
#define sysVKC_HOME					0x24 // HOME key
#define sysVKC_LEFT					0x25 // LEFT ARROW key
#define sysVKC_UP						0x26 // UP ARROW key
#define sysVKC_RIGHT				0x27 // RIGHT ARROW key
#define sysVKC_DOWN					0x28 // DOWN ARROW key
#define sysVKC_SELECT				0x29 // SELECT key
#define sysVKC_PRINT				0x2A // PRINT key
#define sysVKC_EXECUTE			0x2B // EXECUTE key
#define sysVKC_SNAPSHOT			0x2C // PRINT SCREEN key
#define sysVKC_INSERT				0x2D // INS key
#define sysVKC_DELETE				0x2E // DEL key
#define sysVKC_HELP					0x2F // HELP key

#define sysVKC_0 0x30
#define sysVKC_1 0x31
#define sysVKC_2 0x32
#define sysVKC_3 0x33
#define sysVKC_4 0x34
#define sysVKC_5 0x35
#define sysVKC_6 0x36
#define sysVKC_7 0x37
#define sysVKC_8 0x38
#define sysVKC_9 0x39
#define sysVKC_A 0x41
#define sysVKC_B 0x42
#define sysVKC_C 0x43
#define sysVKC_D 0x44
#define sysVKC_E 0x45
#define sysVKC_F 0x46
#define sysVKC_G 0x47
#define sysVKC_H 0x48
#define sysVKC_I 0x49
#define sysVKC_J 0x4A
#define sysVKC_K 0x4B
#define sysVKC_L 0x4C
#define sysVKC_M 0x4D
#define sysVKC_N 0x4E
#define sysVKC_O 0x4F
#define sysVKC_P 0x50
#define sysVKC_Q 0x51
#define sysVKC_R 0x52
#define sysVKC_S 0x53
#define sysVKC_T 0x54
#define sysVKC_U 0x55
#define sysVKC_V 0x56
#define sysVKC_W 0x57
#define sysVKC_X 0x58
#define sysVKC_Y 0x59
#define sysVKC_Z 0x5A

#define sysVKC_LWIN 0x5B // Left Windows key (Microsoft Natural keyboard)

#define sysVKC_RWIN 0x5C // Right Windows key (Natural keyboard)

#define sysVKC_APPS 0x5D // Applications key (Natural keyboard)

#define sysVKC_SLEEP 0x5F // Computer Sleep key

// Num pad keys
#define sysVKC_NUMPAD0 0x60
#define sysVKC_NUMPAD1 0x61
#define sysVKC_NUMPAD2 0x62
#define sysVKC_NUMPAD3 0x63
#define sysVKC_NUMPAD4 0x64
#define sysVKC_NUMPAD5 0x65
#define sysVKC_NUMPAD6 0x66
#define sysVKC_NUMPAD7 0x67
#define sysVKC_NUMPAD8 0x68
#define sysVKC_NUMPAD9 0x69
#define sysVKC_MULTIPLY 0x6A
#define sysVKC_ADD 0x6B
#define sysVKC_SEPARATOR 0x6C
#define sysVKC_SUBTRACT 0x6D
#define sysVKC_DECIMAL 0x6E
#define sysVKC_DIVIDE 0x6F

#define sysVKC_F1 0x70
#define sysVKC_F2 0x71
#define sysVKC_F3 0x72
#define sysVKC_F4 0x73
#define sysVKC_F5 0x74
#define sysVKC_F6 0x75
#define sysVKC_F7 0x76
#define sysVKC_F8 0x77
#define sysVKC_F9 0x78
#define sysVKC_F10 0x79
#define sysVKC_F11 0x7A
#define sysVKC_F12 0x7B
#define sysVKC_F13 0x7C
#define sysVKC_F14 0x7D
#define sysVKC_F15 0x7E
#define sysVKC_F16 0x7F
#define sysVKC_F17 0x80
#define sysVKC_F18 0x81
#define sysVKC_F19 0x82
#define sysVKC_F20 0x83
#define sysVKC_F21 0x84
#define sysVKC_F22 0x85
#define sysVKC_F23 0x86
#define sysVKC_F24 0x87

#define sysVKC_NUMLOCK 0x90
#define sysVKC_SCROLL 0x91
#define sysVKC_LSHIFT 0xA0
#define sysVKC_RSHIFT 0xA1
#define sysVKC_LCONTROL 0xA2
#define sysVKC_RCONTROL 0xA3
#define sysVKC_LMENU 0xA4
#define sysVKC_RMENU 0xA5

#define sysVKC_BROWSER_BACK 0xA6 // Windows 2000/XP: Browser Back key
#define sysVKC_BROWSER_FORWARD 0xA7 // Windows 2000/XP: Browser Forward key
#define sysVKC_BROWSER_REFRESH 0xA8 // Windows 2000/XP: Browser Refresh key
#define sysVKC_BROWSER_STOP 0xA9 // Windows 2000/XP: Browser Stop key
#define sysVKC_BROWSER_SEARCH 0xAA // Windows 2000/XP: Browser Search key
#define sysVKC_BROWSER_FAVORITES 0xAB // Windows 2000/XP: Browser Favorites key
#define sysVKC_BROWSER_HOME 0xAC // Windows 2000/XP: Browser Start and Home key
#define sysVKC_VOLUME_MUTE 0xAD // Windows 2000/XP: Volume Mute key
#define sysVKC_VOLUME_DOWN 0xAE // Windows 2000/XP: Volume Down key
#define sysVKC_VOLUME_UP 0xAF // Windows 2000/XP: Volume Up key
#define sysVKC_MEDIA_NEXT_TRACK 0xB0 // Windows 2000/XP: Next Track key
#define sysVKC_MEDIA_PREV_TRACK 0xB1 // Windows 2000/XP: Previous Track key
#define sysVKC_MEDIA_STOP 0xB2 // Windows 2000/XP: Stop Media key
#define sysVKC_MEDIA_PLAY_PAUSE 0xB3 // Windows 2000/XP: Play/Pause Media key
#define sysVKC_MEDIA_LAUNCH_MAIL 0xB4 // Windows 2000/XP: Start Mail key
#define sysVKC_MEDIA_LAUNCH_MEDIA_SELECT 0xB5 // Windows 2000/XP: Select Media key
#define sysVKC_MEDIA_LAUNCH_APP1 0xB6 // sysVKC_LAUNCH_APP1 (B6) Windows 2000/XP: Start Application 1 key
#define sysVKC_MEDIA_LAUNCH_APP2 0xB7 // sysVKC_LAUNCH_APP2 (B7) Windows 2000/XP: Start Application 2 key

// sysVKC_OEM_1 (BA) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ';:' key
#define sysVKC_OEM_1 0xBA

// Windows 2000/XP: For any country/region, the '+' key
#define sysVKC_OEM_PLUS 0xBB

// Windows 2000/XP: For any country/region, the ',' key
#define sysVKC_OEM_COMMA 0xBC

// Windows 2000/XP: For any country/region, the '-' key
#define sysVKC_OEM_MINUS 0xBD

// Windows 2000/XP: For any country/region, the '.' key
#define sysVKC_OEM_PERIOD 0xBE

// sysVKC_OEM_2 (BF) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '/?' key
#define sysVKC_OEM_2 0xBF

// sysVKC_OEM_3 (C0) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '`~' key
#define sysVKC_OEM_3 0xC0

// sysVKC_OEM_4 (DB) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '[{' key
#define sysVKC_OEM_4 0xDB

// sysVKC_OEM_5 (DC) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '\|' key
#define sysVKC_OEM_5 0xDC

// sysVKC_OEM_6 (DD) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ']}' key
#define sysVKC_OEM_6 0xDD

// sysVKC_OEM_7 (DE) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the 'single-quote/double-quote' key
#define sysVKC_OEM_7 0xDE

// sysVKC_OEM_8 (DF) Used for miscellaneous characters; it can vary by keyboard.
#define sysVKC_OEM_8 0xDF

// sysVKC_OEM_102 (E2) Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard
#define sysVKC_OEM_102 0xE2

// Windows 95/98/Me, Windows NT 4.0, Windows 2000/XP: IME PROCESS key
#define sysVKC_PROCESSKEY 0xE5

// Windows 2000/XP: Used to pass Unicode characters as if they were keystrokes. The sysVKC_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT,SendInput, WM_KEYDOWN, and WM_KEYUP
#define sysVKC_PACKET 0xE7

#define sysVKC_ATTN 0xF6 // Attn key
#define sysVKC_CRSEL 0xF7 // CrSel key
#define sysVKC_EXSEL 0xF8 // ExSel key
#define sysVKC_EREOF 0xF9 // Erase EOF key
#define sysVKC_PLAY 0xFA // Play key
#define sysVKC_ZOOM 0xFB // Zoom key

#define sysVKC_NONAME 0xFC // Reserved for future use

#define sysVKC_PA1 0xFD // sysVKC_PA1 (FD) PA1 key

#define sysVKC_OEM_CLEAR 0xFE // Clear key

#define sysVKC_INVALID 0xffff


// modifier flags
#define sysVKC_MF_LEFT_SHIFT     (1<<0)
#define sysVKC_MF_RIGHT_SHIFT    (1<<1)
#define sysVKC_MF_LEFT_CONTROL   (1<<2)
#define sysVKC_MF_RIGHT_CONTROL  (1<<3)
#define sysVKC_MF_LEFT_ALT       (1<<4)
#define sysVKC_MF_RIGHT_ALT      (1<<5)
#define sysVKC_MF_LEFT_GUI       (1<<6)
#define sysVKC_MF_RIGHT_GUI      (1<<7)

// lock status
#define sysVKC_LS_NUM_LOCK			(1<<0)
#define sysVKC_LS_CAPS_LOCK			(1<<1)
#define sysVKC_LS_SCROLL_LOCK		(1<<2)


#endif