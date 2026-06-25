/*
 * Open Sans 5px 字体实现
 * 字体: Open Sans 2
 * 字符: ASCII 0-127
 */

#include "ipgui_draw_builtin_font.h"

/* 字符   0: NUL  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_000_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   1: SOH  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_001_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   2: STX  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_002_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   3: ETX  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_003_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   4: EOT  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_004_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   5: ENQ  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_005_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   6: ACK  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_006_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   7: BEL  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_007_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   8: BS   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_008_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符   9: HT   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_009_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  10: LF   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_010_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  11: VT   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_011_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  12: FF   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_012_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  13: CR   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_013_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  14: SO   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_014_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  15: SI   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_015_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  16: DLE  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_016_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  17: DC1  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_017_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  18: DC2  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_018_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  19: DC3  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_019_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  20: DC4  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_020_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  21: NAK  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_021_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  22: SYN  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_022_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  23: ETB  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_023_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  24: CAN  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_024_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  25: EM   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_025_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  26: SUB  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_026_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  27: ESC  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_027_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  28: FS   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_028_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  29: GS   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_029_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  30: RS   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_030_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  31: US   */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_031_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字符  33: !    */
/*
 * ▒▒
 * ▒▒
 * ▒▒
 * ▒▒
 * ░░
 */
static const unsigned char open_sans_5px_char_033_bitmap[] = {
    0x44, /* 行 0 */
    0x66, /* 行 1 */
    0x4d, /* 行 2 */
    0x46, /* 行 3 */
    0x03, /* 行 4 */
};

/* 字符  34: "    */
/*
 * ░░░░
 * ░░░░
 */
static const unsigned char open_sans_5px_char_034_bitmap[] = {
    0x3b, 0x3c, /* 行 0 */
    0x3a, 0x39, /* 行 1 */
};

/* 字符  35: #    */
/*
 *   ░░░░  
 * ░░▓▓▓▓░░
 * ▒▒▒▒▒▒  
 * ▒▒▒▒░░  
 */
static const unsigned char open_sans_5px_char_035_bitmap[] = {
    0x00, 0x2f, 0x2f, 0x00, /* 行 0 */
    0x38, 0x8a, 0x89, 0x07, /* 行 1 */
    0x5b, 0x7d, 0x7b, 0x00, /* 行 2 */
    0x51, 0x47, 0x11, 0x00, /* 行 3 */
};

/* 字符  36: $    */
/*
 * ░░▒▒░░
 * ▒▒▒▒░░
 * ░░▓▓▒▒
 * ░░▓▓░░
 *   ░░  
 */
static const unsigned char open_sans_5px_char_036_bitmap[] = {
    0x11, 0x74, 0x17, /* 行 0 */
    0x6f, 0x5f, 0x09, /* 行 1 */
    0x0c, 0x90, 0x48, /* 行 2 */
    0x3f, 0x8b, 0x2e, /* 行 3 */
    0x00, 0x17, 0x00, /* 行 4 */
};

/* 字符  37: %    */
/*
 * ░░░░░░░░
 * ▒▒▒▒▒▒░░
 * ░░▒▒▒▒▒▒
 * ░░▒▒▒▒▒▒
 *       ░░
 */
static const unsigned char open_sans_5px_char_037_bitmap[] = {
    0x3f, 0x3a, 0x27, 0x0c, /* 行 0 */
    0x55, 0x54, 0x64, 0x12, /* 行 1 */
    0x3c, 0x76, 0x7a, 0x5d, /* 行 2 */
    0x02, 0x59, 0x50, 0x6c, /* 行 3 */
    0x00, 0x00, 0x00, 0x02, /* 行 4 */
};

/* 字符  38: &    */
/*
 * ░░▒▒░░  
 * ▒▒▒▒░░  
 * ▒▒▒▒▒▒░░
 * ▒▒▒▒▓▓░░
 *   ░░    
 */
static const unsigned char open_sans_5px_char_038_bitmap[] = {
    0x1b, 0x5f, 0x17, 0x00, /* 行 0 */
    0x42, 0x6e, 0x31, 0x00, /* 行 1 */
    0x58, 0x77, 0x49, 0x35, /* 行 2 */
    0x6b, 0x5a, 0xa4, 0x33, /* 行 3 */
    0x00, 0x02, 0x00, 0x00, /* 行 4 */
};

/* 字符  39: '    */
/*
 * ░░
 * ░░
 */
static const unsigned char open_sans_5px_char_039_bitmap[] = {
    0x3b, /* 行 0 */
    0x3a, /* 行 1 */
};

/* 字符  40: (    */
/*
 * ░░░░
 * ▒▒  
 * ▒▒  
 * ▒▒  
 * ░░░░
 */
static const unsigned char open_sans_5px_char_040_bitmap[] = {
    0x1f, 0x1a, /* 行 0 */
    0x6b, 0x00, /* 行 1 */
    0x6b, 0x00, /* 行 2 */
    0x6b, 0x00, /* 行 3 */
    0x38, 0x1a, /* 行 4 */
};

/* 字符  41: )    */
/*
 * ░░  
 * ▒▒░░
 * ░░░░
 * ▒▒░░
 * ▒▒  
 */
static const unsigned char open_sans_5px_char_041_bitmap[] = {
    0x3a, 0x00, /* 行 0 */
    0x57, 0x16, /* 行 1 */
    0x2f, 0x3d, /* 行 2 */
    0x4a, 0x23, /* 行 3 */
    0x54, 0x00, /* 行 4 */
};

/* 字符  42: *    */
/*
 * ░░▒▒░░
 * ▒▒▓▓░░
 * ░░░░░░
 */
static const unsigned char open_sans_5px_char_042_bitmap[] = {
    0x05, 0x50, 0x05, /* 行 0 */
    0x55, 0xb6, 0x2f, /* 行 1 */
    0x21, 0x27, 0x09, /* 行 2 */
};

/* 字符  43: +    */
/*
 *   ▒▒  
 * ▒▒▓▓░░
 *   ░░  
 */
static const unsigned char open_sans_5px_char_043_bitmap[] = {
    0x00, 0x55, 0x00, /* 行 0 */
    0x42, 0x91, 0x34, /* 行 1 */
    0x00, 0x27, 0x00, /* 行 2 */
};

/* 字符  44: ,    */
/*
 * ▒▒
 * ░░
 */
static const unsigned char open_sans_5px_char_044_bitmap[] = {
    0x43, /* 行 0 */
    0x3a, /* 行 1 */
};

/* 字符  45: -    */
/*
 * ▒▒░░
 */
static const unsigned char open_sans_5px_char_045_bitmap[] = {
    0x4c, 0x27, /* 行 0 */
};

/* 字符  46: .    */
/*
 * ▒▒
 * ░░
 */
static const unsigned char open_sans_5px_char_046_bitmap[] = {
    0x45, /* 行 0 */
    0x04, /* 行 1 */
};

/* 字符  47: /    */
/*
 *   ░░
 * ░░▒▒
 * ▒▒░░
 * ▒▒  
 */
static const unsigned char open_sans_5px_char_047_bitmap[] = {
    0x00, 0x3a, /* 行 0 */
    0x0e, 0x58, /* 行 1 */
    0x5b, 0x0d, /* 行 2 */
    0x68, 0x00, /* 行 3 */
};

/* 字符  48: 0    */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_048_bitmap[] = {
    0x2d, 0x60, 0x17, /* 行 0 */
    0x68, 0x00, 0x68, /* 行 1 */
    0x66, 0x00, 0x6a, /* 行 2 */
    0x56, 0x5e, 0x48, /* 行 3 */
    0x00, 0x02, 0x00, /* 行 4 */
};

/* 字符  49: 1    */
/*
 * ░░▒▒
 * ░░▒▒
 *   ▒▒
 *   ▒▒
 */
static const unsigned char open_sans_5px_char_049_bitmap[] = {
    0x0b, 0x59, /* 行 0 */
    0x26, 0x6b, /* 行 1 */
    0x00, 0x68, /* 行 2 */
    0x00, 0x68, /* 行 3 */
};

/* 字符  50: 2    */
/*
 * ░░▒▒░░
 * ░░░░▒▒
 * ░░▒▒░░
 * ▓▓▒▒░░
 */
static const unsigned char open_sans_5px_char_050_bitmap[] = {
    0x39, 0x63, 0x1b, /* 行 0 */
    0x01, 0x0d, 0x5c, /* 行 1 */
    0x01, 0x6b, 0x0a, /* 行 2 */
    0x81, 0x71, 0x39, /* 行 3 */
};

/* 字符  51: 3    */
/*
 * ░░▒▒░░
 * ░░░░▒▒
 * ░░▒▒▒▒
 * ▒▒▒▒▒▒
 * ░░░░  
 */
static const unsigned char open_sans_5px_char_051_bitmap[] = {
    0x3e, 0x5b, 0x1d, /* 行 0 */
    0x01, 0x2d, 0x55, /* 行 1 */
    0x12, 0x5a, 0x44, /* 行 2 */
    0x48, 0x5e, 0x54, /* 行 3 */
    0x02, 0x04, 0x00, /* 行 4 */
};

/* 字符  52: 4    */
/*
 *   ░░░░
 * ░░▒▒░░
 * ▒▒▒▒▒▒
 * ░░▒▒▒▒
 */
static const unsigned char open_sans_5px_char_052_bitmap[] = {
    0x00, 0x3b, 0x23, /* 行 0 */
    0x12, 0x78, 0x3c, /* 行 1 */
    0x7c, 0x51, 0x55, /* 行 2 */
    0x27, 0x4d, 0x53, /* 行 3 */
};

/* 字符  53: 5    */
/*
 * ░░▒▒░░
 * ▒▒░░  
 * ░░░░▒▒
 * ░░▒▒▒▒
 * ░░░░  
 */
static const unsigned char open_sans_5px_char_053_bitmap[] = {
    0x3e, 0x5c, 0x1c, /* 行 0 */
    0x66, 0x25, 0x00, /* 行 1 */
    0x1d, 0x3e, 0x68, /* 行 2 */
    0x3f, 0x5f, 0x50, /* 行 3 */
    0x01, 0x04, 0x00, /* 行 4 */
};

/* 字符  54: 6    */
/*
 * ░░▒▒░░
 * ▒▒░░░░
 * ▓▓░░▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_054_bitmap[] = {
    0x11, 0x5d, 0x1f, /* 行 0 */
    0x68, 0x2c, 0x04, /* 行 1 */
    0x81, 0x2c, 0x68, /* 行 2 */
    0x53, 0x60, 0x56, /* 行 3 */
    0x00, 0x03, 0x00, /* 行 4 */
};

/* 字符  55: 7    */
/*
 * ▒▒▒▒▒▒
 *   ░░▒▒
 *   ▒▒  
 * ░░▒▒  
 */
static const unsigned char open_sans_5px_char_055_bitmap[] = {
    0x46, 0x5c, 0x4d, /* 行 0 */
    0x00, 0x28, 0x43, /* 行 1 */
    0x00, 0x6d, 0x00, /* 行 2 */
    0x18, 0x56, 0x00, /* 行 3 */
};

/* 字符  56: 8    */
/*
 * ░░▒▒░░
 * ▒▒░░▒▒
 * ▒▒▒▒▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_056_bitmap[] = {
    0x31, 0x56, 0x24, /* 行 0 */
    0x63, 0x34, 0x52, /* 行 1 */
    0x50, 0x69, 0x43, /* 行 2 */
    0x67, 0x56, 0x58, /* 行 3 */
    0x00, 0x03, 0x00, /* 行 4 */
};

/* 字符  57: 9    */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 * ░░▒▒░░
 * ░░░░  
 */
static const unsigned char open_sans_5px_char_057_bitmap[] = {
    0x34, 0x61, 0x14, /* 行 0 */
    0x6d, 0x00, 0x64, /* 行 1 */
    0x40, 0x54, 0x72, /* 行 2 */
    0x26, 0x61, 0x2c, /* 行 3 */
    0x03, 0x01, 0x00, /* 行 4 */
};

/* 字符  58: :    */
/*
 * ▒▒
 *   
 * ▒▒
 * ░░
 */
static const unsigned char open_sans_5px_char_058_bitmap[] = {
    0x4a, /* 行 0 */
    0x00, /* 行 1 */
    0x45, /* 行 2 */
    0x04, /* 行 3 */
};

/* 字符  59: ;    */
/*
 * ▒▒
 *   
 * ▒▒
 * ░░
 */
static const unsigned char open_sans_5px_char_059_bitmap[] = {
    0x4c, /* 行 0 */
    0x00, /* 行 1 */
    0x43, /* 行 2 */
    0x3a, /* 行 3 */
};

/* 字符  60: <    */
/*
 *       
 * ░░▒▒░░
 * ▒▒▒▒░░
 *   ░░░░
 */
static const unsigned char open_sans_5px_char_060_bitmap[] = {
    0x00, 0x00, 0x00, /* 行 0 */
    0x0d, 0x52, 0x35, /* 行 1 */
    0x57, 0x53, 0x0c, /* 行 2 */
    0x00, 0x06, 0x29, /* 行 3 */
};

/* 字符  61: =    */
/*
 * ░░▒▒░░
 * ░░▒▒░░
 */
static const unsigned char open_sans_5px_char_061_bitmap[] = {
    0x3b, 0x54, 0x2f, /* 行 0 */
    0x3d, 0x58, 0x31, /* 行 1 */
};

/* 字符  62: >    */
/*
 *       
 * ▒▒▒▒░░
 * ░░▒▒▒▒
 * ░░░░  
 */
static const unsigned char open_sans_5px_char_062_bitmap[] = {
    0x00, 0x00, 0x00, /* 行 0 */
    0x43, 0x4d, 0x06, /* 行 1 */
    0x15, 0x59, 0x44, /* 行 2 */
    0x2d, 0x01, 0x00, /* 行 3 */
};

/* 字符  63: ?    */
/*
 * ▒▒▒▒  
 *   ▒▒  
 * ░░░░  
 * ░░░░  
 * ░░    
 */
static const unsigned char open_sans_5px_char_063_bitmap[] = {
    0x49, 0x57, 0x00, /* 行 0 */
    0x00, 0x70, 0x00, /* 行 1 */
    0x2e, 0x36, 0x00, /* 行 2 */
    0x35, 0x11, 0x00, /* 行 3 */
    0x03, 0x00, 0x00, /* 行 4 */
};

/* 字符  64: @    */
/*
 *   ▒▒▒▒░░  
 * ▒▒▒▒▒▒▒▒░░
 * ▒▒▒▒▒▒▒▒░░
 * ▒▒░░▒▒▒▒  
 * ░░▒▒▒▒░░  
 */
static const unsigned char open_sans_5px_char_064_bitmap[] = {
    0x00, 0x49, 0x53, 0x2c, 0x00, /* 行 0 */
    0x48, 0x47, 0x61, 0x5e, 0x0a, /* 行 1 */
    0x56, 0x5f, 0x58, 0x44, 0x23, /* 行 2 */
    0x59, 0x36, 0x40, 0x4c, 0x00, /* 行 3 */
    0x02, 0x47, 0x50, 0x0f, 0x00, /* 行 4 */
};

/* 字符  65: A    */
/*
 *   ▒▒    
 * ░░▓▓░░  
 * ▒▒▒▒▓▓  
 * ▒▒  ▒▒░░
 */
static const unsigned char open_sans_5px_char_065_bitmap[] = {
    0x00, 0x55, 0x00, 0x00, /* 行 0 */
    0x0e, 0x8a, 0x30, 0x00, /* 行 1 */
    0x63, 0x6b, 0x80, 0x00, /* 行 2 */
    0x6b, 0x00, 0x65, 0x09, /* 行 3 */
};

/* 字符  66: B    */
/*
 * ▒▒▒▒▒▒
 * ▒▒░░▒▒
 * ▒▒▒▒▒▒
 * ▒▒▒▒▓▓
 */
static const unsigned char open_sans_5px_char_066_bitmap[] = {
    0x45, 0x5f, 0x43, /* 行 0 */
    0x6d, 0x10, 0x75, /* 行 1 */
    0x73, 0x54, 0x71, /* 行 2 */
    0x74, 0x5e, 0x84, /* 行 3 */
};

/* 字符  67: C    */
/*
 * ░░▒▒▒▒
 * ▒▒░░  
 * ▒▒    
 * ▒▒▒▒▒▒
 *   ░░░░
 */
static const unsigned char open_sans_5px_char_067_bitmap[] = {
    0x08, 0x5d, 0x5a, /* 行 0 */
    0x6a, 0x08, 0x00, /* 行 1 */
    0x70, 0x00, 0x00, /* 行 2 */
    0x42, 0x69, 0x4c, /* 行 3 */
    0x00, 0x02, 0x01, /* 行 4 */
};

/* 字符  68: D    */
/*
 * ▒▒▒▒▒▒  
 * ▒▒  ▒▒░░
 * ▒▒  ░░▒▒
 * ▒▒▒▒▒▒░░
 */
static const unsigned char open_sans_5px_char_068_bitmap[] = {
    0x45, 0x5d, 0x47, 0x00, /* 行 0 */
    0x6c, 0x00, 0x49, 0x2f, /* 行 1 */
    0x6c, 0x00, 0x26, 0x49, /* 行 2 */
    0x74, 0x5d, 0x77, 0x07, /* 行 3 */
};

/* 字符  69: E    */
/*
 * ▒▒▒▒░░
 * ▒▒░░░░
 * ▒▒▒▒░░
 * ▒▒▒▒░░
 */
static const unsigned char open_sans_5px_char_069_bitmap[] = {
    0x45, 0x5c, 0x2c, /* 行 0 */
    0x6d, 0x0c, 0x04, /* 行 1 */
    0x73, 0x50, 0x1f, /* 行 2 */
    0x75, 0x60, 0x2e, /* 行 3 */
};

/* 字符  70: F    */
/*
 * ▒▒▒▒░░
 * ▒▒    
 * ▒▒▒▒░░
 * ▒▒    
 */
static const unsigned char open_sans_5px_char_070_bitmap[] = {
    0x45, 0x5c, 0x2c, /* 行 0 */
    0x6c, 0x00, 0x00, /* 行 1 */
    0x74, 0x5c, 0x23, /* 行 2 */
    0x6c, 0x00, 0x00, /* 行 3 */
};

/* 字符  71: G    */
/*
 * ░░▒▒▒▒░░
 * ▒▒░░    
 * ▒▒  ▒▒░░
 * ░░▒▒▒▒▒▒
 *   ░░░░  
 */
static const unsigned char open_sans_5px_char_071_bitmap[] = {
    0x06, 0x5a, 0x62, 0x0e, /* 行 0 */
    0x67, 0x0a, 0x00, 0x00, /* 行 1 */
    0x70, 0x00, 0x6c, 0x3f, /* 行 2 */
    0x3e, 0x6b, 0x6e, 0x40, /* 行 3 */
    0x00, 0x01, 0x04, 0x00, /* 行 4 */
};

/* 字符  72: H    */
/*
 * ░░  ░░░░
 * ▒▒░░▒▒░░
 * ▒▒▒▒▒▒░░
 * ▒▒  ░░░░
 */
static const unsigned char open_sans_5px_char_072_bitmap[] = {
    0x3c, 0x00, 0x1f, 0x1d, /* 行 0 */
    0x6d, 0x0c, 0x41, 0x34, /* 行 1 */
    0x73, 0x50, 0x76, 0x34, /* 行 2 */
    0x6c, 0x00, 0x38, 0x34, /* 行 3 */
};

/* 字符  73: I    */
/*
 * ░░
 * ▒▒
 * ▒▒
 * ▒▒
 */
static const unsigned char open_sans_5px_char_073_bitmap[] = {
    0x3c, /* 行 0 */
    0x6c, /* 行 1 */
    0x6c, /* 行 2 */
    0x6c, /* 行 3 */
};

/* 字符  74: J    */
/*
 *   ░░
 *   ▒▒
 *   ▒▒
 *   ▒▒
 * ░░▒▒
 */
static const unsigned char open_sans_5px_char_074_bitmap[] = {
    0x00, 0x3a, /* 行 0 */
    0x00, 0x68, /* 行 1 */
    0x00, 0x68, /* 行 2 */
    0x00, 0x67, /* 行 3 */
    0x24, 0x66, /* 行 4 */
};

/* 字符  75: K    */
/*
 * ░░  ▒▒  
 * ▒▒▒▒░░  
 * ▒▒▓▓░░  
 * ▒▒░░▒▒  
 */
static const unsigned char open_sans_5px_char_075_bitmap[] = {
    0x3c, 0x00, 0x45, 0x00, /* 行 0 */
    0x6c, 0x58, 0x21, 0x00, /* 行 1 */
    0x75, 0x81, 0x12, 0x00, /* 行 2 */
    0x6c, 0x05, 0x77, 0x00, /* 行 3 */
};

/* 字符  76: L    */
/*
 * ░░    
 * ▒▒    
 * ▒▒    
 * ▒▒▒▒░░
 */
static const unsigned char open_sans_5px_char_076_bitmap[] = {
    0x3c, 0x00, 0x00, /* 行 0 */
    0x6c, 0x00, 0x00, /* 行 1 */
    0x6c, 0x00, 0x00, /* 行 2 */
    0x75, 0x60, 0x2e, /* 行 3 */
};

/* 字符  77: M    */
/*
 * ▒▒░░  ▒▒░░
 * ▒▒▒▒░░▓▓░░
 * ▒▒▒▒▒▒▒▒░░
 * ▒▒░░▓▓▒▒░░
 */
static const unsigned char open_sans_5px_char_077_bitmap[] = {
    0x48, 0x1f, 0x00, 0x63, 0x04, /* 行 0 */
    0x63, 0x65, 0x0d, 0xb4, 0x08, /* 行 1 */
    0x64, 0x65, 0x5a, 0x6e, 0x08, /* 行 2 */
    0x64, 0x1f, 0x87, 0x64, 0x08, /* 行 3 */
};

/* 字符  78: N    */
/*
 * ▒▒░░░░░░
 * ▒▒▒▒░░▒▒
 * ▒▒░░▒▒▒▒
 * ▒▒  ▒▒▒▒
 */
static const unsigned char open_sans_5px_char_078_bitmap[] = {
    0x49, 0x14, 0x0f, 0x28, /* 行 0 */
    0x61, 0x73, 0x1c, 0x48, /* 行 1 */
    0x64, 0x2f, 0x64, 0x48, /* 行 2 */
    0x64, 0x00, 0x74, 0x48, /* 行 3 */
};

/* 字符  79: O    */
/*
 * ░░▒▒▒▒░░
 * ▒▒░░░░▒▒
 * ▒▒    ▒▒
 * ░░▒▒▒▒░░
 *   ░░░░  
 */
static const unsigned char open_sans_5px_char_079_bitmap[] = {
    0x11, 0x64, 0x61, 0x09, /* 行 0 */
    0x6f, 0x04, 0x0d, 0x66, /* 行 1 */
    0x6f, 0x00, 0x00, 0x6f, /* 行 2 */
    0x3f, 0x63, 0x6a, 0x2f, /* 行 3 */
    0x00, 0x02, 0x01, 0x00, /* 行 4 */
};

/* 字符  80: P    */
/*
 * ▒▒▒▒░░
 * ▒▒  ▒▒
 * ▒▒▒▒░░
 * ▒▒    
 */
static const unsigned char open_sans_5px_char_080_bitmap[] = {
    0x45, 0x62, 0x32, /* 行 0 */
    0x6c, 0x00, 0x7c, /* 行 1 */
    0x74, 0x5d, 0x2e, /* 行 2 */
    0x6c, 0x00, 0x00, /* 行 3 */
};

/* 字符  81: Q    */
/*
 * ░░▒▒▒▒░░
 * ▒▒░░░░▒▒
 * ▒▒    ▒▒
 * ░░▒▒▒▒░░
 *   ░░▒▒░░
 */
static const unsigned char open_sans_5px_char_081_bitmap[] = {
    0x11, 0x64, 0x61, 0x09, /* 行 0 */
    0x6f, 0x04, 0x0d, 0x66, /* 行 1 */
    0x6f, 0x00, 0x00, 0x70, /* 行 2 */
    0x3f, 0x63, 0x6e, 0x30, /* 行 3 */
    0x00, 0x02, 0x66, 0x13, /* 行 4 */
};

/* 字符  82: R    */
/*
 * ▒▒▒▒░░
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 * ▒▒  ▒▒
 */
static const unsigned char open_sans_5px_char_082_bitmap[] = {
    0x45, 0x61, 0x36, /* 行 0 */
    0x6c, 0x00, 0x75, /* 行 1 */
    0x74, 0x7e, 0x41, /* 行 2 */
    0x6c, 0x00, 0x77, /* 行 3 */
};

/* 字符  83: S    */
/*
 * ░░▒▒░░
 * ▒▒░░  
 * ░░▒▒░░
 * ▒▒▒▒▒▒
 * ░░░░  
 */
static const unsigned char open_sans_5px_char_083_bitmap[] = {
    0x35, 0x5f, 0x22, /* 行 0 */
    0x6f, 0x0a, 0x00, /* 行 1 */
    0x0d, 0x68, 0x3b, /* 行 2 */
    0x46, 0x61, 0x4c, /* 行 3 */
    0x01, 0x04, 0x00, /* 行 4 */
};

/* 字符  84: T    */
/*
 * ▒▒▒▒▒▒
 *   ▒▒  
 *   ▒▒  
 *   ▒▒  
 */
static const unsigned char open_sans_5px_char_084_bitmap[] = {
    0x57, 0x71, 0x42, /* 行 0 */
    0x00, 0x6c, 0x00, /* 行 1 */
    0x00, 0x6c, 0x00, /* 行 2 */
    0x00, 0x6c, 0x00, /* 行 3 */
};

/* 字符  85: U    */
/*
 * ░░  ░░░░
 * ▒▒  ░░░░
 * ▒▒  ░░░░
 * ▒▒▒▒▓▓░░
 *   ░░    
 */
static const unsigned char open_sans_5px_char_085_bitmap[] = {
    0x3c, 0x00, 0x21, 0x1b, /* 行 0 */
    0x6c, 0x00, 0x3c, 0x30, /* 行 1 */
    0x6d, 0x00, 0x3e, 0x2e, /* 行 2 */
    0x49, 0x6a, 0x86, 0x0a, /* 行 3 */
    0x00, 0x03, 0x00, 0x00, /* 行 4 */
};

/* 字符  86: V    */
/*
 * ░░  ░░
 * ▒▒  ▒▒
 * ▒▒▒▒░░
 * ░░▓▓  
 */
static const unsigned char open_sans_5px_char_086_bitmap[] = {
    0x3f, 0x00, 0x3f, /* 行 0 */
    0x70, 0x00, 0x6e, /* 行 1 */
    0x43, 0x61, 0x3a, /* 行 2 */
    0x02, 0xad, 0x00, /* 行 3 */
};

/* 字符  87: W    */
/*
 * ░░░░▒▒  ░░
 * ▒▒░░▓▓░░▒▒
 * ▒▒▒▒▒▒▓▓░░
 * ░░▓▓░░▓▓  
 */
static const unsigned char open_sans_5px_char_087_bitmap[] = {
    0x3e, 0x03, 0x53, 0x00, 0x3e, /* 行 0 */
    0x6c, 0x3d, 0x8c, 0x26, 0x46, /* 行 1 */
    0x5d, 0x70, 0x42, 0x84, 0x09, /* 行 2 */
    0x1e, 0x85, 0x04, 0xa0, 0x00, /* 行 3 */
};

/* 字符  88: X    */
/*
 * ▒▒  ░░
 * ░░▓▓░░
 * ░░▓▓░░
 * ▒▒░░▒▒
 */
static const unsigned char open_sans_5px_char_088_bitmap[] = {
    0x42, 0x00, 0x3d, /* 行 0 */
    0x38, 0x80, 0x21, /* 行 1 */
    0x17, 0xa0, 0x0a, /* 行 2 */
    0x6d, 0x0b, 0x6a, /* 行 3 */
};

/* 字符  89: Y    */
/*
 * ▒▒  ▒▒
 * ▒▒▒▒░░
 * ░░▓▓  
 *   ▒▒  
 */
static const unsigned char open_sans_5px_char_089_bitmap[] = {
    0x41, 0x00, 0x40, /* 行 0 */
    0x5f, 0x4a, 0x37, /* 行 1 */
    0x06, 0x95, 0x00, /* 行 2 */
    0x00, 0x6c, 0x00, /* 行 3 */
};

/* 字符  90: Z    */
/*
 * ▒▒▒▒▒▒
 *   ▒▒░░
 * ░░▒▒  
 * ▓▓▒▒░░
 */
static const unsigned char open_sans_5px_char_090_bitmap[] = {
    0x43, 0x5c, 0x4e, /* 行 0 */
    0x00, 0x54, 0x23, /* 行 1 */
    0x18, 0x5f, 0x00, /* 行 2 */
    0x94, 0x62, 0x3f, /* 行 3 */
};

/* 字符  91: [    */
/*
 * ▒▒░░
 * ▒▒  
 * ▒▒  
 * ▒▒  
 * ▒▒░░
 */
static const unsigned char open_sans_5px_char_091_bitmap[] = {
    0x4b, 0x2d, /* 行 0 */
    0x68, 0x00, /* 行 1 */
    0x68, 0x00, /* 行 2 */
    0x68, 0x00, /* 行 3 */
    0x64, 0x2f, /* 行 4 */
};

/* 字符  92: \    */
/*
 * ░░  
 * ▒▒  
 * ░░░░
 *   ▒▒
 */
static const unsigned char open_sans_5px_char_092_bitmap[] = {
    0x38, 0x00, /* 行 0 */
    0x64, 0x00, /* 行 1 */
    0x2b, 0x3a, /* 行 2 */
    0x00, 0x67, /* 行 3 */
};

/* 字符  93: ]    */
/*
 * ▒▒░░
 * ░░░░
 * ░░░░
 * ░░░░
 * ▒▒░░
 */
static const unsigned char open_sans_5px_char_093_bitmap[] = {
    0x56, 0x21, /* 行 0 */
    0x2c, 0x3c, /* 行 1 */
    0x2c, 0x3c, /* 行 2 */
    0x2c, 0x3c, /* 行 3 */
    0x63, 0x2f, /* 行 4 */
};

/* 字符  94: ^    */
/*
 * ░░▒▒  
 * ▒▒▒▒░░
 * ░░░░░░
 */
static const unsigned char open_sans_5px_char_094_bitmap[] = {
    0x03, 0x4b, 0x00, /* 行 0 */
    0x49, 0x58, 0x0e, /* 行 1 */
    0x3b, 0x03, 0x38, /* 行 2 */
};

/* 字符  95: _    */
/*
 * ░░▒▒▒▒░░
 */
static const unsigned char open_sans_5px_char_095_bitmap[] = {
    0x01, 0x50, 0x50, 0x14, /* 行 0 */
};

/* 字符  96: `    */
/*
 *   ▒▒
 */
static const unsigned char open_sans_5px_char_096_bitmap[] = {
    0x00, 0x57, /* 行 0 */
};

/* 字符  97: a    */
/*
 * ░░▒▒░░
 * ░░▒▒▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_097_bitmap[] = {
    0x25, 0x66, 0x24, /* 行 0 */
    0x39, 0x53, 0x5f, /* 行 1 */
    0x71, 0x61, 0x5c, /* 行 2 */
    0x00, 0x02, 0x00, /* 行 3 */
};

/* 字符  98: b    */
/*
 * ▒▒    
 * ▒▒▒▒▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_098_bitmap[] = {
    0x4f, 0x00, 0x00, /* 行 0 */
    0x73, 0x61, 0x42, /* 行 1 */
    0x6c, 0x00, 0x6d, /* 行 2 */
    0x75, 0x5b, 0x6a, /* 行 3 */
    0x00, 0x02, 0x00, /* 行 4 */
};

/* 字符  99: c    */
/*
 * ░░▒▒░░
 * ▒▒    
 * ▒▒▒▒░░
 *   ░░  
 */
static const unsigned char open_sans_5px_char_099_bitmap[] = {
    0x38, 0x5e, 0x0f, /* 行 0 */
    0x6e, 0x00, 0x00, /* 行 1 */
    0x60, 0x60, 0x0f, /* 行 2 */
    0x00, 0x02, 0x00, /* 行 3 */
};

/* 字符 100: d    */
/*
 *     ▒▒
 * ░░▒▒▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_100_bitmap[] = {
    0x00, 0x00, 0x52, /* 行 0 */
    0x3d, 0x5d, 0x7e, /* 行 1 */
    0x74, 0x00, 0x70, /* 行 2 */
    0x65, 0x60, 0x7d, /* 行 3 */
    0x00, 0x02, 0x00, /* 行 4 */
};

/* 字符 101: e    */
/*
 * ░░▒▒░░
 * ▓▓▒▒▒▒
 * ▒▒▒▒░░
 *   ░░  
 */
static const unsigned char open_sans_5px_char_101_bitmap[] = {
    0x36, 0x64, 0x29, /* 行 0 */
    0x85, 0x54, 0x5c, /* 行 1 */
    0x5e, 0x62, 0x29, /* 行 2 */
    0x00, 0x05, 0x00, /* 行 3 */
};

/* 字符 102: f    */
/*
 * ░░▒▒
 * ▓▓░░
 * ▒▒  
 * ▒▒  
 */
static const unsigned char open_sans_5px_char_102_bitmap[] = {
    0x32, 0x4d, /* 行 0 */
    0x8a, 0x33, /* 行 1 */
    0x68, 0x00, /* 行 2 */
    0x68, 0x00, /* 行 3 */
};

/* 字符 103: g    */
/*
 * ░░▒▒▒▒
 * ▒▒▒▒░░
 * ▒▒▒▒░░
 * ▒▒░░▒▒
 * ░░░░░░
 */
static const unsigned char open_sans_5px_char_103_bitmap[] = {
    0x3b, 0x66, 0x40, /* 行 0 */
    0x65, 0x5e, 0x2d, /* 行 1 */
    0x6b, 0x60, 0x1b, /* 行 2 */
    0x7c, 0x29, 0x66, /* 行 3 */
    0x0e, 0x28, 0x01, /* 行 4 */
};

/* 字符 104: h    */
/*
 * ▒▒    
 * ▒▒▒▒▒▒
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 */
static const unsigned char open_sans_5px_char_104_bitmap[] = {
    0x4f, 0x00, 0x00, /* 行 0 */
    0x73, 0x5d, 0x43, /* 行 1 */
    0x69, 0x00, 0x69, /* 行 2 */
    0x64, 0x00, 0x68, /* 行 3 */
};

/* 字符 105: i    */
/*
 * ░░
 * ▒▒
 * ▒▒
 * ▒▒
 */
static const unsigned char open_sans_5px_char_105_bitmap[] = {
    0x31, /* 行 0 */
    0x43, /* 行 1 */
    0x64, /* 行 2 */
    0x64, /* 行 3 */
};

/* 字符 106: j    */
/*
 *   ░░
 *   ▒▒
 *   ▒▒
 *   ▒▒
 * ░░▒▒
 * ░░░░
 */
static const unsigned char open_sans_5px_char_106_bitmap[] = {
    0x00, 0x31, /* 行 0 */
    0x00, 0x43, /* 行 1 */
    0x00, 0x64, /* 行 2 */
    0x00, 0x64, /* 行 3 */
    0x0b, 0x6f, /* 行 4 */
    0x0b, 0x13, /* 行 5 */
};

/* 字符 107: k    */
/*
 * ▒▒    
 * ▒▒░░░░
 * ▒▒▒▒  
 * ▒▒▒▒░░
 */
static const unsigned char open_sans_5px_char_107_bitmap[] = {
    0x4f, 0x00, 0x00, /* 行 0 */
    0x64, 0x31, 0x21, /* 行 1 */
    0x74, 0x7c, 0x00, /* 行 2 */
    0x64, 0x46, 0x36, /* 行 3 */
};

/* 字符 108: l    */
/*
 * ▒▒
 * ▒▒
 * ▒▒
 * ▒▒
 */
static const unsigned char open_sans_5px_char_108_bitmap[] = {
    0x4f, /* 行 0 */
    0x64, /* 行 1 */
    0x64, /* 行 2 */
    0x64, /* 行 3 */
};

/* 字符 109: m    */
/*
 * ▒▒▒▒▒▒▒▒░░
 * ▒▒  ▒▒░░░░
 * ▒▒  ▒▒░░▒▒
 */
static const unsigned char open_sans_5px_char_109_bitmap[] = {
    0x4f, 0x5f, 0x6c, 0x68, 0x11, /* 行 0 */
    0x69, 0x00, 0x72, 0x2a, 0x3e, /* 行 1 */
    0x64, 0x00, 0x68, 0x28, 0x40, /* 行 2 */
};

/* 字符 110: n    */
/*
 * ▒▒▒▒▒▒
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 */
static const unsigned char open_sans_5px_char_110_bitmap[] = {
    0x4f, 0x5d, 0x43, /* 行 0 */
    0x69, 0x00, 0x69, /* 行 1 */
    0x64, 0x00, 0x68, /* 行 2 */
};

/* 字符 111: o    */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_111_bitmap[] = {
    0x36, 0x5b, 0x37, /* 行 0 */
    0x6d, 0x00, 0x74, /* 行 1 */
    0x5b, 0x57, 0x60, /* 行 2 */
    0x00, 0x02, 0x00, /* 行 3 */
};

/* 字符 112: p    */
/*
 * ▒▒▒▒▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 * ▒▒░░  
 * ░░    
 */
static const unsigned char open_sans_5px_char_112_bitmap[] = {
    0x4f, 0x5e, 0x43, /* 行 0 */
    0x6b, 0x00, 0x75, /* 行 1 */
    0x79, 0x5c, 0x6c, /* 行 2 */
    0x63, 0x02, 0x00, /* 行 3 */
    0x14, 0x00, 0x00, /* 行 4 */
};

/* 字符 113: q    */
/*
 * ░░▒▒▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▓▓
 *   ░░▒▒
 *     ░░
 */
static const unsigned char open_sans_5px_char_113_bitmap[] = {
    0x3b, 0x5c, 0x57, /* 行 0 */
    0x74, 0x00, 0x70, /* 行 1 */
    0x65, 0x5a, 0x81, /* 行 2 */
    0x00, 0x02, 0x68, /* 行 3 */
    0x00, 0x00, 0x15, /* 行 4 */
};

/* 字符 114: r    */
/*
 * ▒▒▒▒
 * ▒▒  
 * ▒▒  
 */
static const unsigned char open_sans_5px_char_114_bitmap[] = {
    0x4a, 0x57, /* 行 0 */
    0x72, 0x00, /* 行 1 */
    0x64, 0x00, /* 行 2 */
};

/* 字符 115: s    */
/*
 * ▒▒▒▒░░
 * ▒▒▒▒  
 * ░░▒▒░░
 *   ░░  
 */
static const unsigned char open_sans_5px_char_115_bitmap[] = {
    0x47, 0x55, 0x02, /* 行 0 */
    0x4d, 0x5c, 0x00, /* 行 1 */
    0x3b, 0x77, 0x12, /* 行 2 */
    0x00, 0x02, 0x00, /* 行 3 */
};

/* 字符 116: t    */
/*
 * ░░  
 * ▓▓░░
 * ▒▒  
 * ▒▒░░
 *   ░░
 */
static const unsigned char open_sans_5px_char_116_bitmap[] = {
    0x15, 0x00, /* 行 0 */
    0x8b, 0x33, /* 行 1 */
    0x68, 0x00, /* 行 2 */
    0x5c, 0x34, /* 行 3 */
    0x00, 0x03, /* 行 4 */
};

/* 字符 117: u    */
/*
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 *   ░░  
 */
static const unsigned char open_sans_5px_char_117_bitmap[] = {
    0x45, 0x00, 0x45, /* 行 0 */
    0x68, 0x00, 0x69, /* 行 1 */
    0x60, 0x5b, 0x7e, /* 行 2 */
    0x00, 0x02, 0x00, /* 行 3 */
};

/* 字符 118: v    */
/*
 * ▒▒░░░░
 * ▒▒▒▒░░
 * ░░▓▓  
 */
static const unsigned char open_sans_5px_char_118_bitmap[] = {
    0x4a, 0x0a, 0x3d, /* 行 0 */
    0x6b, 0x55, 0x13, /* 行 1 */
    0x2d, 0x8a, 0x00, /* 行 2 */
};

/* 字符 119: w    */
/*
 * ▒▒░░░░▒▒
 * ▒▒▒▒▒▒▒▒
 * ▒▒▒▒▒▒░░
 */
static const unsigned char open_sans_5px_char_119_bitmap[] = {
    0x47, 0x3c, 0x2f, 0x45, /* 行 0 */
    0x67, 0x63, 0x64, 0x63, /* 行 1 */
    0x56, 0x59, 0x75, 0x3b, /* 行 2 */
};

/* 字符 120: x    */
/*
 * ▒▒░░░░
 * ░░▓▓  
 * ▒▒▒▒░░
 */
static const unsigned char open_sans_5px_char_120_bitmap[] = {
    0x4d, 0x26, 0x27, /* 行 0 */
    0x1f, 0xa3, 0x00, /* 行 1 */
    0x67, 0x4f, 0x2f, /* 行 2 */
};

/* 字符 121: y    */
/*
 * ▒▒░░▒▒
 * ▒▒▒▒░░
 * ░░▓▓  
 * ▒▒▒▒  
 * ░░    
 */
static const unsigned char open_sans_5px_char_121_bitmap[] = {
    0x4a, 0x09, 0x40, /* 行 0 */
    0x6b, 0x56, 0x16, /* 行 1 */
    0x23, 0x90, 0x00, /* 行 2 */
    0x43, 0x45, 0x00, /* 行 3 */
    0x1f, 0x00, 0x00, /* 行 4 */
};

/* 字符 122: z    */
/*
 * ░░▒▒░░
 * ░░▒▒  
 * ▓▓▒▒░░
 */
static const unsigned char open_sans_5px_char_122_bitmap[] = {
    0x38, 0x77, 0x0a, /* 行 0 */
    0x0f, 0x63, 0x00, /* 行 1 */
    0x8a, 0x54, 0x0b, /* 行 2 */
};

/* 字符 123: {    */
/*
 * ░░▒▒
 * ░░░░
 * ▒▒░░
 * ▒▒░░
 * ░░▒▒
 */
static const unsigned char open_sans_5px_char_123_bitmap[] = {
    0x0a, 0x41, /* 行 0 */
    0x3e, 0x2b, /* 行 1 */
    0x7c, 0x0a, /* 行 2 */
    0x43, 0x25, /* 行 3 */
    0x12, 0x4e, /* 行 4 */
};

/* 字符 124: |    */
/*
 * ▒▒
 * ▒▒
 * ▒▒
 * ▒▒
 * ▒▒
 * ░░
 */
static const unsigned char open_sans_5px_char_124_bitmap[] = {
    0x46, /* 行 0 */
    0x58, /* 行 1 */
    0x58, /* 行 2 */
    0x58, /* 行 3 */
    0x58, /* 行 4 */
    0x13, /* 行 5 */
};

/* 字符 125: }    */
/*
 * ▒▒░░
 * ▒▒░░
 * ░░▒▒
 * ▒▒░░
 * ▒▒░░
 */
static const unsigned char open_sans_5px_char_125_bitmap[] = {
    0x47, 0x02, /* 行 0 */
    0x47, 0x22, /* 行 1 */
    0x1d, 0x6c, /* 行 2 */
    0x41, 0x27, /* 行 3 */
    0x5b, 0x05, /* 行 4 */
};

/* 字符 126: ~    */
/*
 * ░░░░░░
 * ░░▒▒░░
 */
static const unsigned char open_sans_5px_char_126_bitmap[] = {
    0x06, 0x02, 0x01, /* 行 0 */
    0x3d, 0x59, 0x32, /* 行 1 */
};

/* 字符 127: DEL  */
/*
 * ░░▒▒░░
 * ▒▒  ▒▒
 * ▒▒  ▒▒
 * ▒▒▒▒▒▒
 */
static const unsigned char open_sans_5px_char_127_bitmap[] = {
    0x36, 0x40, 0x37, /* 行 0 */
    0x40, 0x00, 0x44, /* 行 1 */
    0x40, 0x00, 0x44, /* 行 2 */
    0x52, 0x40, 0x55, /* 行 3 */
};

/* 字形结构体定义 */
static const  ipgui_glyph_t open_sans_5px_char_000 = {
    .cover_map = open_sans_5px_char_000_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_001 = {
    .cover_map = open_sans_5px_char_001_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_002 = {
    .cover_map = open_sans_5px_char_002_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_003 = {
    .cover_map = open_sans_5px_char_003_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_004 = {
    .cover_map = open_sans_5px_char_004_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_005 = {
    .cover_map = open_sans_5px_char_005_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_006 = {
    .cover_map = open_sans_5px_char_006_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_007 = {
    .cover_map = open_sans_5px_char_007_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_008 = {
    .cover_map = open_sans_5px_char_008_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_009 = {
    .cover_map = open_sans_5px_char_009_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 10
};

static const  ipgui_glyph_t open_sans_5px_char_010 = {
    .cover_map = open_sans_5px_char_010_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 0
};

static const  ipgui_glyph_t open_sans_5px_char_011 = {
    .cover_map = open_sans_5px_char_011_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_012 = {
    .cover_map = open_sans_5px_char_012_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_013 = {
    .cover_map = open_sans_5px_char_013_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 0
};

static const  ipgui_glyph_t open_sans_5px_char_014 = {
    .cover_map = open_sans_5px_char_014_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_015 = {
    .cover_map = open_sans_5px_char_015_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_016 = {
    .cover_map = open_sans_5px_char_016_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_017 = {
    .cover_map = open_sans_5px_char_017_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_018 = {
    .cover_map = open_sans_5px_char_018_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_019 = {
    .cover_map = open_sans_5px_char_019_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_020 = {
    .cover_map = open_sans_5px_char_020_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_021 = {
    .cover_map = open_sans_5px_char_021_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_022 = {
    .cover_map = open_sans_5px_char_022_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_023 = {
    .cover_map = open_sans_5px_char_023_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_024 = {
    .cover_map = open_sans_5px_char_024_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_025 = {
    .cover_map = open_sans_5px_char_025_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_026 = {
    .cover_map = open_sans_5px_char_026_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_027 = {
    .cover_map = open_sans_5px_char_027_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_028 = {
    .cover_map = open_sans_5px_char_028_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_029 = {
    .cover_map = open_sans_5px_char_029_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_030 = {
    .cover_map = open_sans_5px_char_030_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_031 = {
    .cover_map = open_sans_5px_char_031_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_032 = {
    .cover_map = 0,
    .width = 0,
    .height = 0,
    .bearing_x = 0,
    .bearing_y = 0,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_033 = {
    .cover_map = open_sans_5px_char_033_bitmap,
    .width = 1,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_034 = {
    .cover_map = open_sans_5px_char_034_bitmap,
    .width = 2,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_035 = {
    .cover_map = open_sans_5px_char_035_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_036 = {
    .cover_map = open_sans_5px_char_036_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_037 = {
    .cover_map = open_sans_5px_char_037_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_038 = {
    .cover_map = open_sans_5px_char_038_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_039 = {
    .cover_map = open_sans_5px_char_039_bitmap,
    .width = 1,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_040 = {
    .cover_map = open_sans_5px_char_040_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_041 = {
    .cover_map = open_sans_5px_char_041_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_042 = {
    .cover_map = open_sans_5px_char_042_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_043 = {
    .cover_map = open_sans_5px_char_043_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_044 = {
    .cover_map = open_sans_5px_char_044_bitmap,
    .width = 1,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 1,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_045 = {
    .cover_map = open_sans_5px_char_045_bitmap,
    .width = 2,
    .height = 1,
    .bearing_x = 0,
    .bearing_y = 2,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_046 = {
    .cover_map = open_sans_5px_char_046_bitmap,
    .width = 1,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 1,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_047 = {
    .cover_map = open_sans_5px_char_047_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_048 = {
    .cover_map = open_sans_5px_char_048_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_049 = {
    .cover_map = open_sans_5px_char_049_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_050 = {
    .cover_map = open_sans_5px_char_050_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_051 = {
    .cover_map = open_sans_5px_char_051_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_052 = {
    .cover_map = open_sans_5px_char_052_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_053 = {
    .cover_map = open_sans_5px_char_053_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_054 = {
    .cover_map = open_sans_5px_char_054_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_055 = {
    .cover_map = open_sans_5px_char_055_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_056 = {
    .cover_map = open_sans_5px_char_056_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_057 = {
    .cover_map = open_sans_5px_char_057_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_058 = {
    .cover_map = open_sans_5px_char_058_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_059 = {
    .cover_map = open_sans_5px_char_059_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_060 = {
    .cover_map = open_sans_5px_char_060_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_061 = {
    .cover_map = open_sans_5px_char_061_bitmap,
    .width = 3,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_062 = {
    .cover_map = open_sans_5px_char_062_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_063 = {
    .cover_map = open_sans_5px_char_063_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_064 = {
    .cover_map = open_sans_5px_char_064_bitmap,
    .width = 5,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 5
};

static const  ipgui_glyph_t open_sans_5px_char_065 = {
    .cover_map = open_sans_5px_char_065_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_066 = {
    .cover_map = open_sans_5px_char_066_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_067 = {
    .cover_map = open_sans_5px_char_067_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_068 = {
    .cover_map = open_sans_5px_char_068_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_069 = {
    .cover_map = open_sans_5px_char_069_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_070 = {
    .cover_map = open_sans_5px_char_070_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_071 = {
    .cover_map = open_sans_5px_char_071_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_072 = {
    .cover_map = open_sans_5px_char_072_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_073 = {
    .cover_map = open_sans_5px_char_073_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_074 = {
    .cover_map = open_sans_5px_char_074_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = -1,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_075 = {
    .cover_map = open_sans_5px_char_075_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_076 = {
    .cover_map = open_sans_5px_char_076_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_077 = {
    .cover_map = open_sans_5px_char_077_bitmap,
    .width = 5,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 5
};

static const  ipgui_glyph_t open_sans_5px_char_078 = {
    .cover_map = open_sans_5px_char_078_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_079 = {
    .cover_map = open_sans_5px_char_079_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_080 = {
    .cover_map = open_sans_5px_char_080_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_081 = {
    .cover_map = open_sans_5px_char_081_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_082 = {
    .cover_map = open_sans_5px_char_082_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_083 = {
    .cover_map = open_sans_5px_char_083_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_084 = {
    .cover_map = open_sans_5px_char_084_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_085 = {
    .cover_map = open_sans_5px_char_085_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_086 = {
    .cover_map = open_sans_5px_char_086_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_087 = {
    .cover_map = open_sans_5px_char_087_bitmap,
    .width = 5,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 5
};

static const  ipgui_glyph_t open_sans_5px_char_088 = {
    .cover_map = open_sans_5px_char_088_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_089 = {
    .cover_map = open_sans_5px_char_089_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_090 = {
    .cover_map = open_sans_5px_char_090_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_091 = {
    .cover_map = open_sans_5px_char_091_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_092 = {
    .cover_map = open_sans_5px_char_092_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_093 = {
    .cover_map = open_sans_5px_char_093_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_094 = {
    .cover_map = open_sans_5px_char_094_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_095 = {
    .cover_map = open_sans_5px_char_095_bitmap,
    .width = 4,
    .height = 1,
    .bearing_x = -1,
    .bearing_y = 0,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_096 = {
    .cover_map = open_sans_5px_char_096_bitmap,
    .width = 2,
    .height = 1,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_097 = {
    .cover_map = open_sans_5px_char_097_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_098 = {
    .cover_map = open_sans_5px_char_098_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_099 = {
    .cover_map = open_sans_5px_char_099_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_100 = {
    .cover_map = open_sans_5px_char_100_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_101 = {
    .cover_map = open_sans_5px_char_101_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_102 = {
    .cover_map = open_sans_5px_char_102_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_103 = {
    .cover_map = open_sans_5px_char_103_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_104 = {
    .cover_map = open_sans_5px_char_104_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_105 = {
    .cover_map = open_sans_5px_char_105_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_106 = {
    .cover_map = open_sans_5px_char_106_bitmap,
    .width = 2,
    .height = 6,
    .bearing_x = -1,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_107 = {
    .cover_map = open_sans_5px_char_107_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_108 = {
    .cover_map = open_sans_5px_char_108_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

static const  ipgui_glyph_t open_sans_5px_char_109 = {
    .cover_map = open_sans_5px_char_109_bitmap,
    .width = 5,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 5
};

static const  ipgui_glyph_t open_sans_5px_char_110 = {
    .cover_map = open_sans_5px_char_110_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_111 = {
    .cover_map = open_sans_5px_char_111_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_112 = {
    .cover_map = open_sans_5px_char_112_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_113 = {
    .cover_map = open_sans_5px_char_113_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_114 = {
    .cover_map = open_sans_5px_char_114_bitmap,
    .width = 2,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_115 = {
    .cover_map = open_sans_5px_char_115_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_116 = {
    .cover_map = open_sans_5px_char_116_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_117 = {
    .cover_map = open_sans_5px_char_117_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_118 = {
    .cover_map = open_sans_5px_char_118_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_119 = {
    .cover_map = open_sans_5px_char_119_bitmap,
    .width = 4,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 4
};

static const  ipgui_glyph_t open_sans_5px_char_120 = {
    .cover_map = open_sans_5px_char_120_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_121 = {
    .cover_map = open_sans_5px_char_121_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_122 = {
    .cover_map = open_sans_5px_char_122_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_123 = {
    .cover_map = open_sans_5px_char_123_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_124 = {
    .cover_map = open_sans_5px_char_124_bitmap,
    .width = 1,
    .height = 6,
    .bearing_x = 1,
    .bearing_y = 4,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_125 = {
    .cover_map = open_sans_5px_char_125_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

static const  ipgui_glyph_t open_sans_5px_char_126 = {
    .cover_map = open_sans_5px_char_126_bitmap,
    .width = 3,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

static const  ipgui_glyph_t open_sans_5px_char_127 = {
    .cover_map = open_sans_5px_char_127_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字形数组 */
static const  ipgui_glyph_t open_sans_5px_glyph_array[128] = {
    open_sans_5px_char_000,  /*   0: NUL  */
    open_sans_5px_char_001,  /*   1: SOH  */
    open_sans_5px_char_002,  /*   2: STX  */
    open_sans_5px_char_003,  /*   3: ETX  */
    open_sans_5px_char_004,  /*   4: EOT  */
    open_sans_5px_char_005,  /*   5: ENQ  */
    open_sans_5px_char_006,  /*   6: ACK  */
    open_sans_5px_char_007,  /*   7: BEL  */
    open_sans_5px_char_008,  /*   8: BS   */
    open_sans_5px_char_009,  /*   9: HT   */
    open_sans_5px_char_010,  /*  10: LF   */
    open_sans_5px_char_011,  /*  11: VT   */
    open_sans_5px_char_012,  /*  12: FF   */
    open_sans_5px_char_013,  /*  13: CR   */
    open_sans_5px_char_014,  /*  14: SO   */
    open_sans_5px_char_015,  /*  15: SI   */
    open_sans_5px_char_016,  /*  16: DLE  */
    open_sans_5px_char_017,  /*  17: DC1  */
    open_sans_5px_char_018,  /*  18: DC2  */
    open_sans_5px_char_019,  /*  19: DC3  */
    open_sans_5px_char_020,  /*  20: DC4  */
    open_sans_5px_char_021,  /*  21: NAK  */
    open_sans_5px_char_022,  /*  22: SYN  */
    open_sans_5px_char_023,  /*  23: ETB  */
    open_sans_5px_char_024,  /*  24: CAN  */
    open_sans_5px_char_025,  /*  25: EM   */
    open_sans_5px_char_026,  /*  26: SUB  */
    open_sans_5px_char_027,  /*  27: ESC  */
    open_sans_5px_char_028,  /*  28: FS   */
    open_sans_5px_char_029,  /*  29: GS   */
    open_sans_5px_char_030,  /*  30: RS   */
    open_sans_5px_char_031,  /*  31: US   */
    open_sans_5px_char_032,  /*  32: SPACE */
    open_sans_5px_char_033,  /*  33: !    */
    open_sans_5px_char_034,  /*  34: "    */
    open_sans_5px_char_035,  /*  35: #    */
    open_sans_5px_char_036,  /*  36: $    */
    open_sans_5px_char_037,  /*  37: %    */
    open_sans_5px_char_038,  /*  38: &    */
    open_sans_5px_char_039,  /*  39: '    */
    open_sans_5px_char_040,  /*  40: (    */
    open_sans_5px_char_041,  /*  41: )    */
    open_sans_5px_char_042,  /*  42: *    */
    open_sans_5px_char_043,  /*  43: +    */
    open_sans_5px_char_044,  /*  44: ,    */
    open_sans_5px_char_045,  /*  45: -    */
    open_sans_5px_char_046,  /*  46: .    */
    open_sans_5px_char_047,  /*  47: /    */
    open_sans_5px_char_048,  /*  48: 0    */
    open_sans_5px_char_049,  /*  49: 1    */
    open_sans_5px_char_050,  /*  50: 2    */
    open_sans_5px_char_051,  /*  51: 3    */
    open_sans_5px_char_052,  /*  52: 4    */
    open_sans_5px_char_053,  /*  53: 5    */
    open_sans_5px_char_054,  /*  54: 6    */
    open_sans_5px_char_055,  /*  55: 7    */
    open_sans_5px_char_056,  /*  56: 8    */
    open_sans_5px_char_057,  /*  57: 9    */
    open_sans_5px_char_058,  /*  58: :    */
    open_sans_5px_char_059,  /*  59: ;    */
    open_sans_5px_char_060,  /*  60: <    */
    open_sans_5px_char_061,  /*  61: =    */
    open_sans_5px_char_062,  /*  62: >    */
    open_sans_5px_char_063,  /*  63: ?    */
    open_sans_5px_char_064,  /*  64: @    */
    open_sans_5px_char_065,  /*  65: A    */
    open_sans_5px_char_066,  /*  66: B    */
    open_sans_5px_char_067,  /*  67: C    */
    open_sans_5px_char_068,  /*  68: D    */
    open_sans_5px_char_069,  /*  69: E    */
    open_sans_5px_char_070,  /*  70: F    */
    open_sans_5px_char_071,  /*  71: G    */
    open_sans_5px_char_072,  /*  72: H    */
    open_sans_5px_char_073,  /*  73: I    */
    open_sans_5px_char_074,  /*  74: J    */
    open_sans_5px_char_075,  /*  75: K    */
    open_sans_5px_char_076,  /*  76: L    */
    open_sans_5px_char_077,  /*  77: M    */
    open_sans_5px_char_078,  /*  78: N    */
    open_sans_5px_char_079,  /*  79: O    */
    open_sans_5px_char_080,  /*  80: P    */
    open_sans_5px_char_081,  /*  81: Q    */
    open_sans_5px_char_082,  /*  82: R    */
    open_sans_5px_char_083,  /*  83: S    */
    open_sans_5px_char_084,  /*  84: T    */
    open_sans_5px_char_085,  /*  85: U    */
    open_sans_5px_char_086,  /*  86: V    */
    open_sans_5px_char_087,  /*  87: W    */
    open_sans_5px_char_088,  /*  88: X    */
    open_sans_5px_char_089,  /*  89: Y    */
    open_sans_5px_char_090,  /*  90: Z    */
    open_sans_5px_char_091,  /*  91: [    */
    open_sans_5px_char_092,  /*  92: \    */
    open_sans_5px_char_093,  /*  93: ]    */
    open_sans_5px_char_094,  /*  94: ^    */
    open_sans_5px_char_095,  /*  95: _    */
    open_sans_5px_char_096,  /*  96: `    */
    open_sans_5px_char_097,  /*  97: a    */
    open_sans_5px_char_098,  /*  98: b    */
    open_sans_5px_char_099,  /*  99: c    */
    open_sans_5px_char_100,  /* 100: d    */
    open_sans_5px_char_101,  /* 101: e    */
    open_sans_5px_char_102,  /* 102: f    */
    open_sans_5px_char_103,  /* 103: g    */
    open_sans_5px_char_104,  /* 104: h    */
    open_sans_5px_char_105,  /* 105: i    */
    open_sans_5px_char_106,  /* 106: j    */
    open_sans_5px_char_107,  /* 107: k    */
    open_sans_5px_char_108,  /* 108: l    */
    open_sans_5px_char_109,  /* 109: m    */
    open_sans_5px_char_110,  /* 110: n    */
    open_sans_5px_char_111,  /* 111: o    */
    open_sans_5px_char_112,  /* 112: p    */
    open_sans_5px_char_113,  /* 113: q    */
    open_sans_5px_char_114,  /* 114: r    */
    open_sans_5px_char_115,  /* 115: s    */
    open_sans_5px_char_116,  /* 116: t    */
    open_sans_5px_char_117,  /* 117: u    */
    open_sans_5px_char_118,  /* 118: v    */
    open_sans_5px_char_119,  /* 119: w    */
    open_sans_5px_char_120,  /* 120: x    */
    open_sans_5px_char_121,  /* 121: y    */
    open_sans_5px_char_122,  /* 122: z    */
    open_sans_5px_char_123,  /* 123: {    */
    open_sans_5px_char_124,  /* 124: |    */
    open_sans_5px_char_125,  /* 125: }    */
    open_sans_5px_char_126,  /* 126: ~    */
    open_sans_5px_char_127,  /* 127: DEL  */
};

/* 字体结构体 */
const  ipgui_font_t open_sans_5px = {
    .glyphs = ( ipgui_glyph_t*)open_sans_5px_glyph_array,
    .line_height = 7,
    .baseline = 4,
    .font_size = 5,
    .max_height = 6,
    .space_width = 1
};

/* 获取字形函数 */
const  ipgui_glyph_t* open_sans_5px_get_glyph(unsigned char char_code)
{
    if (char_code < 128) {
        return &open_sans_5px_glyph_array[char_code];
    }
    return &open_sans_5px_char_063;  /* 返回'?'字符 */
}

/* 文本宽度计算 */
unsigned short open_sans_5px_text_width(const char* text)
{
    unsigned short width = 0;
    while (*text) {
        unsigned char ch = (unsigned char)*text;
        if (ch < 128) {
            const  ipgui_glyph_t* glyph = &open_sans_5px_glyph_array[ch];
            width += glyph->advance;
        }
        text++;
    }
    return width;
}
