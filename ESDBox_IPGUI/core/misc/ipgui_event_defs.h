#ifndef IPGUI_EVENT_DEFS_H
#define IPGUI_EVENT_DEFS_H

#include "ipgui_utils.h"

IPGUI_HEADER_BEGIN _______________MARKER_______________

/* event types */
#define IPGUI_EVT_TYPE_SYNC         0x00        /* sync */
#define IPGUI_EVT_TYPE_KEY          0x01        /* keyboard or button */
#define IPGUI_EVT_TYPE_REL          0x02        /* rel position eg:mouse */
#define IPGUI_EVT_TYPE_ABS          0x03        /* abs position eg:touch */
#define IPGUI_EVT_TYPE_MISC		    0x04        /* misc(杂项) */
#define IPGUI_EVT_TYPE_SW   	    0x05        /* switch */
#define IPGUI_EVT_TYPE_MAX          0x0f
#define IPGUI_EVT_TYPE_CNT          (IPGUI_EVT_TYPE_MAX + 1)

/* sync codes */
#define IPGUI_EVT_SYN_REPORT		0
// #define IPGUI_EVT_SYN_CONFIG		1
#define IPGUI_EVT_SYN_MT_REPORT	    2
// #define IPGUI_EVT_SYN_DROPPED	3
#define IPGUI_EVT_SYN_MAX			0x0f
#define IPGUI_EVT_SYN_CNT			(IPGUI_EVT_SYN_MAX + 1)

/* key codes */
#define IPGUI_EVT_KEY_RESERVED		0
#define IPGUI_EVT_KEY_ESC			1
#define IPGUI_EVT_KEY_1			    2
#define IPGUI_EVT_KEY_2			    3
#define IPGUI_EVT_KEY_3			    4
#define IPGUI_EVT_KEY_4			    5
#define IPGUI_EVT_KEY_5			    6
#define IPGUI_EVT_KEY_6			    7
#define IPGUI_EVT_KEY_7			    8
#define IPGUI_EVT_KEY_8			    9
#define IPGUI_EVT_KEY_9			    10
#define IPGUI_EVT_KEY_0			    11
#define IPGUI_EVT_KEY_MINUS		    12
#define IPGUI_EVT_KEY_EQUAL		    13
#define IPGUI_EVT_KEY_BACKSPACE		14
#define IPGUI_EVT_KEY_TAB			15
#define IPGUI_EVT_KEY_Q			    16
#define IPGUI_EVT_KEY_W			    17
#define IPGUI_EVT_KEY_E			    18
#define IPGUI_EVT_KEY_R			    19
#define IPGUI_EVT_KEY_T			    20
#define IPGUI_EVT_KEY_Y			    21
#define IPGUI_EVT_KEY_U			    22
#define IPGUI_EVT_KEY_I			    23
#define IPGUI_EVT_KEY_O			    24
#define IPGUI_EVT_KEY_P			    25
#define IPGUI_EVT_KEY_LEFTBRACE		26
#define IPGUI_EVT_KEY_RIGHTBRACE	27
#define IPGUI_EVT_KEY_ENTER		    28
#define IPGUI_EVT_KEY_LEFTCTRL		29
#define IPGUI_EVT_KEY_A			    30
#define IPGUI_EVT_KEY_S			    31
#define IPGUI_EVT_KEY_D			    32
#define IPGUI_EVT_KEY_F			    33
#define IPGUI_EVT_KEY_G			    34
#define IPGUI_EVT_KEY_H			    35
#define IPGUI_EVT_KEY_J			    36
#define IPGUI_EVT_KEY_K			    37
#define IPGUI_EVT_KEY_L			    38
#define IPGUI_EVT_KEY_SEMICOLON		39
#define IPGUI_EVT_KEY_APOSTROPHE	40
#define IPGUI_EVT_KEY_GRAVE		    41
#define IPGUI_EVT_KEY_LEFTSHIFT		42
#define IPGUI_EVT_KEY_BACKSLASH		43
#define IPGUI_EVT_KEY_Z			    44
#define IPGUI_EVT_KEY_X			    45
#define IPGUI_EVT_KEY_C			    46
#define IPGUI_EVT_KEY_V			    47
#define IPGUI_EVT_KEY_B			    48
#define IPGUI_EVT_KEY_N			    49
#define IPGUI_EVT_KEY_M			    50
#define IPGUI_EVT_KEY_COMMA		    51
#define IPGUI_EVT_KEY_DOT			52
#define IPGUI_EVT_KEY_SLASH		    53
#define IPGUI_EVT_KEY_RIGHTSHIFT	54
#define IPGUI_EVT_KEY_KPASTERISK	55
#define IPGUI_EVT_KEY_LEFTALT		56
#define IPGUI_EVT_KEY_SPACE		    57
#define IPGUI_EVT_KEY_CAPSLOCK		58
#define IPGUI_EVT_KEY_F1			59
#define IPGUI_EVT_KEY_F2			60
#define IPGUI_EVT_KEY_F3			61
#define IPGUI_EVT_KEY_F4			62
#define IPGUI_EVT_KEY_F5			63
#define IPGUI_EVT_KEY_F6			64
#define IPGUI_EVT_KEY_F7			65
#define IPGUI_EVT_KEY_F8			66
#define IPGUI_EVT_KEY_F9			67
#define IPGUI_EVT_KEY_F10			68
#define IPGUI_EVT_KEY_NUMLOCK		69
#define IPGUI_EVT_KEY_SCROLLLOCK	70
#define IPGUI_EVT_KEY_KP7			71
#define IPGUI_EVT_KEY_KP8			72
#define IPGUI_EVT_KEY_KP9			73
#define IPGUI_EVT_KEY_KPMINUS		74
#define IPGUI_EVT_KEY_KP4			75
#define IPGUI_EVT_KEY_KP5			76
#define IPGUI_EVT_KEY_KP6			77
#define IPGUI_EVT_KEY_KPPLUS		78
#define IPGUI_EVT_KEY_KP1			79
#define IPGUI_EVT_KEY_KP2			80
#define IPGUI_EVT_KEY_KP3			81
#define IPGUI_EVT_KEY_KP0			82
#define IPGUI_EVT_KEY_KPDOT		    83

#define IPGUI_EVT_KEY_ZENKAKUHANKAKU 85
#define IPGUI_EVT_KEY_102ND		    86
#define IPGUI_EVT_KEY_F11			87
#define IPGUI_EVT_KEY_F12			88
#define IPGUI_EVT_KEY_RO			89
#define IPGUI_EVT_KEY_KATAKANA		90
#define IPGUI_EVT_KEY_HIRAGANA		91
#define IPGUI_EVT_KEY_HENKAN		92
#define IPGUI_EVT_KEY_KATAKANAHIRAGANA 93
#define IPGUI_EVT_KEY_MUHENKAN		94
#define IPGUI_EVT_KEY_KPJPCOMMA		95
#define IPGUI_EVT_KEY_KPENTER		96
#define IPGUI_EVT_KEY_RIGHTCTRL		97
#define IPGUI_EVT_KEY_KPSLASH		98
#define IPGUI_EVT_KEY_SYSRQ		    99
#define IPGUI_EVT_KEY_RIGHTALT		100
#define IPGUI_EVT_KEY_LINEFEED		101
#define IPGUI_EVT_KEY_HOME		    102
#define IPGUI_EVT_KEY_UP			103
#define IPGUI_EVT_KEY_PAGEUP		104
#define IPGUI_EVT_KEY_LEFT		    105
#define IPGUI_EVT_KEY_RIGHT		    106
#define IPGUI_EVT_KEY_END			107
#define IPGUI_EVT_KEY_DOWN		    108
#define IPGUI_EVT_KEY_PAGEDOWN		109
#define IPGUI_EVT_KEY_INSERT		110
#define IPGUI_EVT_KEY_DELETE		111
#define IPGUI_EVT_KEY_MACRO		    112
#define IPGUI_EVT_KEY_MUTE		    113
#define IPGUI_EVT_KEY_VOLUMEDOWN	114
#define IPGUI_EVT_KEY_VOLUMEUP		115
#define IPGUI_EVT_KEY_POWER		    116
#define IPGUI_EVT_KEY_KPEQUAL		117
#define IPGUI_EVT_KEY_KPPLUSMINUS	118
#define IPGUI_EVT_KEY_PAUSE		    119
#define IPGUI_EVT_KEY_SCALE		    120//0x78

#define IPGUI_EVT_BTN	            0x80
#define IPGUI_EVT_BTN_0			    0x80
#define IPGUI_EVT_BTN_1			    0x81
#define IPGUI_EVT_BTN_2			    0x82
#define IPGUI_EVT_BTN_3			    0x83
#define IPGUI_EVT_BTN_4			    0x84
#define IPGUI_EVT_BTN_5			    0x85
#define IPGUI_EVT_BTN_6			    0x86
#define IPGUI_EVT_BTN_7			    0x87
#define IPGUI_EVT_BTN_8			    0x88
#define IPGUI_EVT_BTN_9			    0x89
#define IPGUI_EVT_BTN_10			0x8a
#define IPGUI_EVT_BTN_11			0x8b
#define IPGUI_EVT_BTN_12			0x8c
#define IPGUI_EVT_BTN_13			0x8d
#define IPGUI_EVT_BTN_14			0x8e
#define IPGUI_EVT_BTN_15			0x8f
#define IPGUI_EVT_BTN_16			0x90
#define IPGUI_EVT_BTN_17			0x91
#define IPGUI_EVT_BTN_18			0x92
#define IPGUI_EVT_BTN_19			0x93

#define IPGUI_EVT_BTN_MOUSE		    0xa0
#define IPGUI_EVT_BTN_LEFT		    0xa0
#define IPGUI_EVT_BTN_RIGHT		    0xa1
#define IPGUI_EVT_BTN_MIDDLE        0xa2
#define IPGUI_EVT_BTN_SIDE		    0xa3
#define IPGUI_EVT_BTN_EXTRA		    0xa4
#define IPGUI_EVT_BTN_FORWARD		0xa5
#define IPGUI_EVT_BTN_BACK		    0xa6
#define IPGUI_EVT_BTN_TASK		    0xa7

#define IPGUI_EVT_BTN_TOUCH		    0xa8

#define IPGUI_EVT_KEY_MAX           0xff
#define IPGUI_EVT_KEY_CNT           (IPGUI_EVT_KEY_MAX + 1)

/* rel event codes */
#define IPGUI_EVT_REL_X			    0x00
#define IPGUI_EVT_REL_Y			    0x01
#define IPGUI_EVT_REL_Z			    0x02
#define IPGUI_EVT_REL_RX			0x03
#define IPGUI_EVT_REL_RY			0x04
#define IPGUI_EVT_REL_RZ			0x05
#define IPGUI_EVT_REL_HWHEEL		0x06
#define IPGUI_EVT_REL_DIAL		    0x07
#define IPGUI_EVT_REL_WHEEL		    0x08
#define IPGUI_EVT_REL_MISC		    0x09
#define IPGUI_EVT_REL_MAX           0x0f
#define IPGUI_EVT_REL_CNT           (IPGUI_EVT_REL_MAX + 1)

/* abs event codes */
#define IPGUI_EVT_ABS_X			    0x00
#define IPGUI_EVT_ABS_Y			    0x01
#define IPGUI_EVT_ABS_Z			    0x02
#define IPGUI_EVT_ABS_RX			0x03
#define IPGUI_EVT_ABS_RY			0x04
#define IPGUI_EVT_ABS_RZ			0x05
#define IPGUI_EVT_ABS_THROTTLE		0x06
#define IPGUI_EVT_ABS_RUDDER		0x07
#define IPGUI_EVT_ABS_WHEEL		    0x08
#define IPGUI_EVT_ABS_GAS			0x09
#define IPGUI_EVT_ABS_BRAKE		    0x0a
#define IPGUI_EVT_ABS_HAT0X		    0x10
#define IPGUI_EVT_ABS_HAT0Y		    0x11
#define IPGUI_EVT_ABS_HAT1X		    0x12
#define IPGUI_EVT_ABS_HAT1Y		    0x13
#define IPGUI_EVT_ABS_HAT2X		    0x14
#define IPGUI_EVT_ABS_HAT2Y		    0x15
#define IPGUI_EVT_ABS_HAT3X		    0x16
#define IPGUI_EVT_ABS_HAT3Y		    0x17
#define IPGUI_EVT_ABS_PRESSURE		0x18
#define IPGUI_EVT_ABS_DISTANCE		0x19
#define IPGUI_EVT_ABS_TILT_X		0x1a
#define IPGUI_EVT_ABS_TILT_Y		0x1b
#define IPGUI_EVT_ABS_TOOL_WIDTH	0x1c

#define IPGUI_EVT_ABS_VOLUME		0x20
#define IPGUI_EVT_ABS_MT		    0x21
#define IPGUI_EVT_ABS_MISC		    0x28
#define IPGUI_EVT_ABS_MAX			0x3f
#define IPGUI_EVT_ABS_CNT			(IPGUI_EVT_ABS_MAX + 1)

/* misc event codes */
#define IPGUI_EVT_MISC_SERIAL		0x00
#define IPGUI_EVT_MISC_PULSELED		0x01
#define IPGUI_EVT_MISC_GESTURE		0x02
#define IPGUI_EVT_MISC_RAW			0x03
#define IPGUI_EVT_MISC_SCAN		    0x04
#define IPGUI_EVT_MISC_TIMESTAMP	0x05
#define IPGUI_EVT_MISC_TIMER        0x06
#define IPGUI_EVT_MISC_MAX			0x0f
#define IPGUI_EVT_MISC_CNT			(IPGUI_EVT_MISC_MAX + 1)

/* sw event codes */
#define IPGUI_EVT_SW_MISC		    0x00
#define IPGUI_EVT_SW_MAX			0x0f
#define IPGUI_EVT_SW_CNT			(IPGUI_EVT_SW_MAX+1)

/* key values */
#define IPGUI_EVT_KEY_DOWN_VAL      0x01
#define IPGUI_EVT_KEY_UP_VAL        0x00

#define IPGUI_EVT_BTN_DOWN_VAL      0x01
#define IPGUI_EVT_BTN_UP_VAL        0x00

/* abs values */
#define IPGUI_EVT_ABS_VAL(x)        (x)


IPGUI_HEADER_END   _______________MARKER_______________

#endif