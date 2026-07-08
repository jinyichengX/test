/*
 * Quicksand Medium 5px 字体实现
 * 字体: Quicksand Medium
 * 字符: ASCII 0-127
 */

#include "ipgui_draw_builtin_font.h"

/* 字符   0: NUL  */
static const unsigned char quicksand_medium_5px_char_000_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, /* 行 0 */
    0x00, 0x00, 0x00, 0x00, 0x00, /* 行 1 */
    0x00, 0x00, 0x00, 0x00, 0x00, /* 行 2 */
    0x00, 0x00, 0x00, 0x00, 0x00, /* 行 3 */
    0x00, 0x00, 0x00, 0x00, 0x00, /* 行 4 */
    0x00, 0x00, 0x00, 0x00, 0x00, /* 行 5 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_000 = {
    .cover_map = quicksand_medium_5px_char_000_bitmap,
    .width = 5,
    .height = 6,
    .bearing_x = 0,
    .bearing_y = 6,
    .advance = 5
};

/* 字符   1: SOH  */
static const unsigned char quicksand_medium_5px_char_001_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_001 = {
    .cover_map = quicksand_medium_5px_char_001_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   2: STX  */
static const unsigned char quicksand_medium_5px_char_002_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_002 = {
    .cover_map = quicksand_medium_5px_char_002_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   3: ETX  */
static const unsigned char quicksand_medium_5px_char_003_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_003 = {
    .cover_map = quicksand_medium_5px_char_003_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   4: EOT  */
static const unsigned char quicksand_medium_5px_char_004_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_004 = {
    .cover_map = quicksand_medium_5px_char_004_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   5: ENQ  */
static const unsigned char quicksand_medium_5px_char_005_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_005 = {
    .cover_map = quicksand_medium_5px_char_005_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   6: ACK  */
static const unsigned char quicksand_medium_5px_char_006_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_006 = {
    .cover_map = quicksand_medium_5px_char_006_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   7: BEL  */
static const unsigned char quicksand_medium_5px_char_007_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_007 = {
    .cover_map = quicksand_medium_5px_char_007_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   8: BS   */
static const unsigned char quicksand_medium_5px_char_008_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_008 = {
    .cover_map = quicksand_medium_5px_char_008_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符   9: HT   */
static const unsigned char quicksand_medium_5px_char_009_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_009 = {
    .cover_map = quicksand_medium_5px_char_009_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  10: LF   */
static const unsigned char quicksand_medium_5px_char_010_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_010 = {
    .cover_map = quicksand_medium_5px_char_010_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  11: VT   */
static const unsigned char quicksand_medium_5px_char_011_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_011 = {
    .cover_map = quicksand_medium_5px_char_011_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  12: FF   */
static const unsigned char quicksand_medium_5px_char_012_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_012 = {
    .cover_map = quicksand_medium_5px_char_012_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  13: CR   */
static const unsigned char quicksand_medium_5px_char_013_bitmap[] = { };

static const ipgui_glyph_t quicksand_medium_5px_char_013 = {
    .cover_map = quicksand_medium_5px_char_013_bitmap,
    .width = 1,
    .height = 0,
    .bearing_x = 0,
    .bearing_y = 0,
    .advance = 1
};

/* 字符  14: SO   */
static const unsigned char quicksand_medium_5px_char_014_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_014 = {
    .cover_map = quicksand_medium_5px_char_014_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  15: SI   */
static const unsigned char quicksand_medium_5px_char_015_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_015 = {
    .cover_map = quicksand_medium_5px_char_015_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  16: DLE  */
static const unsigned char quicksand_medium_5px_char_016_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_016 = {
    .cover_map = quicksand_medium_5px_char_016_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  17: DC1  */
static const unsigned char quicksand_medium_5px_char_017_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_017 = {
    .cover_map = quicksand_medium_5px_char_017_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  18: DC2  */
static const unsigned char quicksand_medium_5px_char_018_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_018 = {
    .cover_map = quicksand_medium_5px_char_018_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  19: DC3  */
static const unsigned char quicksand_medium_5px_char_019_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_019 = {
    .cover_map = quicksand_medium_5px_char_019_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  20: DC4  */
static const unsigned char quicksand_medium_5px_char_020_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_020 = {
    .cover_map = quicksand_medium_5px_char_020_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  21: NAK  */
static const unsigned char quicksand_medium_5px_char_021_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_021 = {
    .cover_map = quicksand_medium_5px_char_021_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  22: SYN  */
static const unsigned char quicksand_medium_5px_char_022_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_022 = {
    .cover_map = quicksand_medium_5px_char_022_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  23: ETB  */
static const unsigned char quicksand_medium_5px_char_023_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_023 = {
    .cover_map = quicksand_medium_5px_char_023_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  24: CAN  */
static const unsigned char quicksand_medium_5px_char_024_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_024 = {
    .cover_map = quicksand_medium_5px_char_024_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  25: EM   */
static const unsigned char quicksand_medium_5px_char_025_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_025 = {
    .cover_map = quicksand_medium_5px_char_025_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  26: SUB  */
static const unsigned char quicksand_medium_5px_char_026_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_026 = {
    .cover_map = quicksand_medium_5px_char_026_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  27: ESC  */
static const unsigned char quicksand_medium_5px_char_027_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_027 = {
    .cover_map = quicksand_medium_5px_char_027_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  28: FS   */
static const unsigned char quicksand_medium_5px_char_028_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_028 = {
    .cover_map = quicksand_medium_5px_char_028_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  29: GS   */
static const unsigned char quicksand_medium_5px_char_029_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_029 = {
    .cover_map = quicksand_medium_5px_char_029_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  30: RS   */
static const unsigned char quicksand_medium_5px_char_030_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_030 = {
    .cover_map = quicksand_medium_5px_char_030_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  31: US   */
static const unsigned char quicksand_medium_5px_char_031_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_031 = {
    .cover_map = quicksand_medium_5px_char_031_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  32: SPACE */
static const unsigned char quicksand_medium_5px_char_032_bitmap[] = { };

static const ipgui_glyph_t quicksand_medium_5px_char_032 = {
    .cover_map = quicksand_medium_5px_char_032_bitmap,
    .width = 1,
    .height = 0,
    .bearing_x = 0,
    .bearing_y = 0,
    .advance = 1
};

/* 字符  33: !    */
static const unsigned char quicksand_medium_5px_char_033_bitmap[] = {
    0x70, /* 行 0 */
    0x65, /* 行 1 */
    0x37, /* 行 2 */
    0x4b, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_033 = {
    .cover_map = quicksand_medium_5px_char_033_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

/* 字符  34: "    */
static const unsigned char quicksand_medium_5px_char_034_bitmap[] = {
    0x70, 0x6f, /* 行 0 */
    0x2d, 0x2c, /* 行 1 */
    0x00, 0x00, /* 行 2 */
    0x00, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_034 = {
    .cover_map = quicksand_medium_5px_char_034_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符  35: #    */
static const unsigned char quicksand_medium_5px_char_035_bitmap[] = {
    0x00, 0x66, 0x67, 0x00, /* 行 0 */
    0x5e, 0x9d, 0xb3, 0x14, /* 行 1 */
    0x8a, 0x93, 0xb2, 0x01, /* 行 2 */
    0x61, 0x18, 0x52, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_035 = {
    .cover_map = quicksand_medium_5px_char_035_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  36: $    */
static const unsigned char quicksand_medium_5px_char_036_bitmap[] = {
    0x00, 0x34, 0x00, /* 行 0 */
    0x5c, 0xc4, 0x55, /* 行 1 */
    0x81, 0x89, 0x00, /* 行 2 */
    0x1c, 0xc5, 0x58, /* 行 3 */
    0x44, 0x8e, 0x7e, /* 行 4 */
    0x20, 0x9f, 0x0b, /* 行 5 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_036 = {
    .cover_map = quicksand_medium_5px_char_036_bitmap,
    .width = 3,
    .height = 6,
    .bearing_x = 0,
    .bearing_y = 5,
    .advance = 3
};

/* 字符  37: %    */
static const unsigned char quicksand_medium_5px_char_037_bitmap[] = {
    0x75, 0x64, 0x5b, 0x15, /* 行 0 */
    0x58, 0x5b, 0x6c, 0x00, /* 行 1 */
    0x00, 0x72, 0x55, 0x56, /* 行 2 */
    0x1d, 0x53, 0x67, 0x70, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_037 = {
    .cover_map = quicksand_medium_5px_char_037_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  38: &    */
static const unsigned char quicksand_medium_5px_char_038_bitmap[] = {
    0x24, 0x8e, 0x5d, 0x00, /* 行 0 */
    0x46, 0x6c, 0x00, 0x00, /* 行 1 */
    0x81, 0x61, 0x9b, 0x05, /* 行 2 */
    0x6a, 0x74, 0xa4, 0x2a, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_038 = {
    .cover_map = quicksand_medium_5px_char_038_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  39: '    */
static const unsigned char quicksand_medium_5px_char_039_bitmap[] = {
    0x70, /* 行 0 */
    0x2d, /* 行 1 */
    0x00, /* 行 2 */
    0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_039 = {
    .cover_map = quicksand_medium_5px_char_039_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

/* 字符  40: (    */
static const unsigned char quicksand_medium_5px_char_040_bitmap[] = {
    0x22, 0x6b, /* 行 0 */
    0x86, 0x05, /* 行 1 */
    0x82, 0x00, /* 行 2 */
    0x86, 0x04, /* 行 3 */
    0x25, 0x6d, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_040 = {
    .cover_map = quicksand_medium_5px_char_040_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符  41: )    */
static const unsigned char quicksand_medium_5px_char_041_bitmap[] = {
    0x83, 0x0c, /* 行 0 */
    0x1a, 0x71, /* 行 1 */
    0x00, 0x82, /* 行 2 */
    0x1b, 0x6e, /* 行 3 */
    0x81, 0x0a, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_041 = {
    .cover_map = quicksand_medium_5px_char_041_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符  42: *    */
static const unsigned char quicksand_medium_5px_char_042_bitmap[] = {
    0x8b, 0x4c, /* 行 0 */
    0x31, 0x05, /* 行 1 */
    0x00, 0x00, /* 行 2 */
    0x00, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_042 = {
    .cover_map = quicksand_medium_5px_char_042_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符  43: +    */
static const unsigned char quicksand_medium_5px_char_043_bitmap[] = {
    0x00, 0x78, 0x00, /* 行 0 */
    0x59, 0xc0, 0x4f, /* 行 1 */
    0x00, 0x65, 0x00, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_043 = {
    .cover_map = quicksand_medium_5px_char_043_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符  44: ,    */
static const unsigned char quicksand_medium_5px_char_044_bitmap[] = {
    0x4a, /* 行 0 */
    0x2e, /* 行 1 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_044 = {
    .cover_map = quicksand_medium_5px_char_044_bitmap,
    .width = 1,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 1,
    .advance = 1
};

/* 字符  45: -    */
static const unsigned char quicksand_medium_5px_char_045_bitmap[] = {
    0x5a, 0x56, /* 行 0 */
    0x00, 0x00, /* 行 1 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_045 = {
    .cover_map = quicksand_medium_5px_char_045_bitmap,
    .width = 2,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 2,
    .advance = 2
};

/* 字符  46: .    */
static const unsigned char quicksand_medium_5px_char_046_bitmap[] = {
    0x4b, /* 行 0 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_046 = {
    .cover_map = quicksand_medium_5px_char_046_bitmap,
    .width = 1,
    .height = 1,
    .bearing_x = 0,
    .bearing_y = 1,
    .advance = 1
};

/* 字符  47: /    */
static const unsigned char quicksand_medium_5px_char_047_bitmap[] = {
    0x00, 0x08, 0x63, /* 行 0 */
    0x00, 0x60, 0x16, /* 行 1 */
    0x02, 0x74, 0x00, /* 行 2 */
    0x52, 0x25, 0x00, /* 行 3 */
    0x6d, 0x00, 0x00, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_047 = {
    .cover_map = quicksand_medium_5px_char_047_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  48: 0    */
static const unsigned char quicksand_medium_5px_char_048_bitmap[] = {
    0x4f, 0x96, 0x51, /* 行 0 */
    0x82, 0x00, 0x7f, /* 行 1 */
    0x81, 0x00, 0x7e, /* 行 2 */
    0x50, 0x87, 0x51, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_048 = {
    .cover_map = quicksand_medium_5px_char_048_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  49: 1    */
static const unsigned char quicksand_medium_5px_char_049_bitmap[] = {
    0x97, 0x53, /* 行 0 */
    0x31, 0x58, /* 行 1 */
    0x2c, 0x58, /* 行 2 */
    0x27, 0x52, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_049 = {
    .cover_map = quicksand_medium_5px_char_049_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符  50: 2    */
static const unsigned char quicksand_medium_5px_char_050_bitmap[] = {
    0x5f, 0x8f, 0x37, /* 行 0 */
    0x03, 0x21, 0x61, /* 行 1 */
    0x03, 0x92, 0x09, /* 行 2 */
    0x7c, 0xa3, 0x3d, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_050 = {
    .cover_map = quicksand_medium_5px_char_050_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  51: 3    */
static const unsigned char quicksand_medium_5px_char_051_bitmap[] = {
    0x5d, 0xc1, 0x16, /* 行 0 */
    0x2b, 0xaa, 0x0f, /* 行 1 */
    0x00, 0x2b, 0x51, /* 行 2 */
    0x60, 0x84, 0x19, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_051 = {
    .cover_map = quicksand_medium_5px_char_051_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  52: 4    */
static const unsigned char quicksand_medium_5px_char_052_bitmap[] = {
    0x00, 0x4b, 0x65, /* 行 0 */
    0x1a, 0x7b, 0x6c, /* 行 1 */
    0x6c, 0x8b, 0xa2, /* 行 2 */
    0x00, 0x0d, 0x64, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_052 = {
    .cover_map = quicksand_medium_5px_char_052_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  53: 5    */
static const unsigned char quicksand_medium_5px_char_053_bitmap[] = {
    0x78, 0x7c, 0x2a, /* 行 0 */
    0x8b, 0x8b, 0x18, /* 行 1 */
    0x00, 0x11, 0x6b, /* 行 2 */
    0x5b, 0x87, 0x25, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_053 = {
    .cover_map = quicksand_medium_5px_char_053_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  54: 6    */
static const unsigned char quicksand_medium_5px_char_054_bitmap[] = {
    0x15, 0x88, 0x03, /* 行 0 */
    0x84, 0x86, 0x1c, /* 行 1 */
    0x85, 0x08, 0x74, /* 行 2 */
    0x5e, 0x8b, 0x37, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_054 = {
    .cover_map = quicksand_medium_5px_char_054_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  55: 7    */
static const unsigned char quicksand_medium_5px_char_055_bitmap[] = {
    0x4f, 0x8d, 0x72, /* 行 0 */
    0x00, 0x6a, 0x1c, /* 行 1 */
    0x00, 0x84, 0x00, /* 行 2 */
    0x2a, 0x4e, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_055 = {
    .cover_map = quicksand_medium_5px_char_055_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  56: 8    */
static const unsigned char quicksand_medium_5px_char_056_bitmap[] = {
    0x47, 0x8c, 0x30, /* 行 0 */
    0x7b, 0x0e, 0x6e, /* 行 1 */
    0x66, 0xa3, 0x4b, /* 行 2 */
    0x6a, 0x7c, 0x59, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_056 = {
    .cover_map = quicksand_medium_5px_char_056_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  57: 9    */
static const unsigned char quicksand_medium_5px_char_057_bitmap[] = {
    0x60, 0x8e, 0x34, /* 行 0 */
    0x7e, 0x0a, 0x7a, /* 行 1 */
    0x42, 0xa7, 0x4d, /* 行 2 */
    0x1f, 0x79, 0x01, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_057 = {
    .cover_map = quicksand_medium_5px_char_057_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  58: :    */
static const unsigned char quicksand_medium_5px_char_058_bitmap[] = {
    0x4c, /* 行 0 */
    0x00, /* 行 1 */
    0x4b, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_058 = {
    .cover_map = quicksand_medium_5px_char_058_bitmap,
    .width = 1,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 1
};

/* 字符  59: ;    */
static const unsigned char quicksand_medium_5px_char_059_bitmap[] = {
    0x4c, 0x00, /* 行 0 */
    0x00, 0x00, /* 行 1 */
    0x47, 0x02, /* 行 2 */
    0x2e, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_059 = {
    .cover_map = quicksand_medium_5px_char_059_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 1
};

/* 字符  60: <    */
static const unsigned char quicksand_medium_5px_char_060_bitmap[] = {
    0x0d, 0x87, 0x25, /* 行 0 */
    0xa6, 0x2c, 0x00, /* 行 1 */
    0x0c, 0x84, 0x25, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_060 = {
    .cover_map = quicksand_medium_5px_char_060_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符  61: =    */
static const unsigned char quicksand_medium_5px_char_061_bitmap[] = {
    0x55, 0x7c, 0x3c, /* 行 0 */
    0x55, 0x7c, 0x3c, /* 行 1 */
    0x00, 0x00, 0x00, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_061 = {
    .cover_map = quicksand_medium_5px_char_061_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符  62: >    */
static const unsigned char quicksand_medium_5px_char_062_bitmap[] = {
    0x61, 0x56, 0x00, /* 行 0 */
    0x00, 0x81, 0x50, /* 行 1 */
    0x60, 0x5b, 0x00, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_062 = {
    .cover_map = quicksand_medium_5px_char_062_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符  63: ?    */
static const unsigned char quicksand_medium_5px_char_063_bitmap[] = {
    0x6d, 0x99, 0x1b, /* 行 0 */
    0x00, 0x42, 0x44, /* 行 1 */
    0x19, 0x8c, 0x01, /* 行 2 */
    0x13, 0x3f, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_063 = {
    .cover_map = quicksand_medium_5px_char_063_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  64: @    */
static const unsigned char quicksand_medium_5px_char_064_bitmap[] = {
    0x09, 0x6b, 0x5d, 0x67, 0x25, /* 行 0 */
    0x5e, 0x51, 0x78, 0x67, 0x55, /* 行 1 */
    0x57, 0x6f, 0x2b, 0x5a, 0x52, /* 行 2 */
    0x62, 0x4b, 0x55, 0x71, 0x0d, /* 行 3 */
    0x10, 0x6b, 0x5d, 0x1d, 0x00, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_064 = {
    .cover_map = quicksand_medium_5px_char_064_bitmap,
    .width = 5,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 5
};

/* 字符  65: A    */
static const unsigned char quicksand_medium_5px_char_065_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, /* 行 0 */
    0x00, 0xa4, 0x06, 0x00, /* 行 1 */
    0x1f, 0x6f, 0x4f, 0x00, /* 行 2 */
    0x6e, 0x87, 0x8f, 0x00, /* 行 3 */
    0x67, 0x00, 0x61, 0x0b, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_065 = {
    .cover_map = quicksand_medium_5px_char_065_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 5,
    .advance = 3
};

/* 字符  66: B    */
static const unsigned char quicksand_medium_5px_char_066_bitmap[] = {
    0x86, 0x88, 0x8e, 0x00, /* 行 0 */
    0x8a, 0x8d, 0x9c, 0x00, /* 行 1 */
    0x80, 0x00, 0x80, 0x03, /* 行 2 */
    0x85, 0x80, 0x9a, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_066 = {
    .cover_map = quicksand_medium_5px_char_066_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  67: C    */
static const unsigned char quicksand_medium_5px_char_067_bitmap[] = {
    0x2e, 0x95, 0x88, 0x00, /* 行 0 */
    0x84, 0x01, 0x00, 0x00, /* 行 1 */
    0x82, 0x00, 0x00, 0x00, /* 行 2 */
    0x32, 0x8b, 0x83, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_067 = {
    .cover_map = quicksand_medium_5px_char_067_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  68: D    */
static const unsigned char quicksand_medium_5px_char_068_bitmap[] = {
    0x83, 0x84, 0x96, 0x06, /* 行 0 */
    0x7c, 0x00, 0x39, 0x45, /* 行 1 */
    0x7c, 0x00, 0x38, 0x45, /* 行 2 */
    0x83, 0x80, 0x94, 0x07, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_068 = {
    .cover_map = quicksand_medium_5px_char_068_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  69: E    */
static const unsigned char quicksand_medium_5px_char_069_bitmap[] = {
    0x83, 0x84, 0x4a, /* 行 0 */
    0x88, 0x80, 0x2b, /* 行 1 */
    0x7c, 0x00, 0x00, /* 行 2 */
    0x83, 0x80, 0x48, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_069 = {
    .cover_map = quicksand_medium_5px_char_069_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  70: F    */
static const unsigned char quicksand_medium_5px_char_070_bitmap[] = {
    0x83, 0x84, 0x45, /* 行 0 */
    0x7c, 0x00, 0x00, /* 行 1 */
    0x88, 0x80, 0x26, /* 行 2 */
    0x74, 0x00, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_070 = {
    .cover_map = quicksand_medium_5px_char_070_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  71: G    */
static const unsigned char quicksand_medium_5px_char_071_bitmap[] = {
    0x2a, 0x9a, 0x90, 0x02, /* 行 0 */
    0x86, 0x02, 0x00, 0x00, /* 行 1 */
    0x86, 0x0d, 0x9c, 0x27, /* 行 2 */
    0x2c, 0x96, 0x96, 0x24, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_071 = {
    .cover_map = quicksand_medium_5px_char_071_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  72: H    */
static const unsigned char quicksand_medium_5px_char_072_bitmap[] = {
    0x70, 0x00, 0x49, 0x2a, /* 行 0 */
    0x7c, 0x00, 0x50, 0x30, /* 行 1 */
    0x88, 0x80, 0xa8, 0x30, /* 行 2 */
    0x73, 0x00, 0x4c, 0x2a, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_072 = {
    .cover_map = quicksand_medium_5px_char_072_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  73: I    */
static const unsigned char quicksand_medium_5px_char_073_bitmap[] = {
    0x72, /* 行 0 */
    0x7c, /* 行 1 */
    0x7c, /* 行 2 */
    0x72, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_073 = {
    .cover_map = quicksand_medium_5px_char_073_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

/* 字符  74: J    */
static const unsigned char quicksand_medium_5px_char_074_bitmap[] = {
    0x00, 0x17, 0x5d, /* 行 0 */
    0x00, 0x1c, 0x64, /* 行 1 */
    0x00, 0x1e, 0x61, /* 行 2 */
    0x65, 0x94, 0x22, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_074 = {
    .cover_map = quicksand_medium_5px_char_074_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  75: K    */
static const unsigned char quicksand_medium_5px_char_075_bitmap[] = {
    0x72, 0x02, 0x89, 0x05, /* 行 0 */
    0x7c, 0x8d, 0x26, 0x00, /* 行 1 */
    0x8c, 0x77, 0x5d, 0x00, /* 行 2 */
    0x72, 0x00, 0x86, 0x0a, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_075 = {
    .cover_map = quicksand_medium_5px_char_075_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  76: L    */
static const unsigned char quicksand_medium_5px_char_076_bitmap[] = {
    0x72, 0x00, 0x00, /* 行 0 */
    0x7c, 0x00, 0x00, /* 行 1 */
    0x7c, 0x00, 0x00, /* 行 2 */
    0x83, 0x84, 0x43, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_076 = {
    .cover_map = quicksand_medium_5px_char_076_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  77: M    */
static const unsigned char quicksand_medium_5px_char_077_bitmap[] = {
    0x84, 0x1f, 0x0e, 0x92, /* 行 0 */
    0x79, 0x84, 0x73, 0x89, /* 行 1 */
    0x74, 0x4d, 0x5a, 0x7c, /* 行 2 */
    0x6b, 0x00, 0x00, 0x72, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_077 = {
    .cover_map = quicksand_medium_5px_char_077_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  78: N    */
static const unsigned char quicksand_medium_5px_char_078_bitmap[] = {
    0x85, 0x26, 0x2b, 0x3b, /* 行 0 */
    0x71, 0x94, 0x32, 0x40, /* 行 1 */
    0x6c, 0x2d, 0x9b, 0x40, /* 行 2 */
    0x65, 0x00, 0x78, 0x3a, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_078 = {
    .cover_map = quicksand_medium_5px_char_078_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  79: O    */
static const unsigned char quicksand_medium_5px_char_079_bitmap[] = {
    0x33, 0x98, 0x9a, 0x16, /* 行 0 */
    0x84, 0x00, 0x0e, 0x78, /* 行 1 */
    0x82, 0x00, 0x0c, 0x77, /* 行 2 */
    0x33, 0x93, 0x95, 0x16, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_079 = {
    .cover_map = quicksand_medium_5px_char_079_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  80: P    */
static const unsigned char quicksand_medium_5px_char_080_bitmap[] = {
    0x7f, 0x85, 0x6b, /* 行 0 */
    0x74, 0x00, 0x7c, /* 行 1 */
    0x84, 0x84, 0x49, /* 行 2 */
    0x6b, 0x00, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_080 = {
    .cover_map = quicksand_medium_5px_char_080_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  81: Q    */
static const unsigned char quicksand_medium_5px_char_081_bitmap[] = {
    0x37, 0x8c, 0x95, 0x18, /* 行 0 */
    0x80, 0x00, 0x08, 0x78, /* 行 1 */
    0x90, 0x17, 0x33, 0x78, /* 行 2 */
    0x21, 0xd5, 0xb4, 0x0f, /* 行 3 */
    0x04, 0x44, 0x7f, 0x7e, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_081 = {
    .cover_map = quicksand_medium_5px_char_081_bitmap,
    .width = 4,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  82: R    */
static const unsigned char quicksand_medium_5px_char_082_bitmap[] = {
    0x83, 0x84, 0x92, 0x00, /* 行 0 */
    0x7c, 0x00, 0x7d, 0x02, /* 行 1 */
    0x87, 0x7c, 0x9b, 0x00, /* 行 2 */
    0x73, 0x00, 0x6e, 0x0e, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_082 = {
    .cover_map = quicksand_medium_5px_char_082_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  83: S    */
static const unsigned char quicksand_medium_5px_char_083_bitmap[] = {
    0x64, 0x84, 0x53, /* 行 0 */
    0x81, 0x4d, 0x03, /* 行 1 */
    0x01, 0x4b, 0x7e, /* 行 2 */
    0x6b, 0x7e, 0x5e, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_083 = {
    .cover_map = quicksand_medium_5px_char_083_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  84: T    */
static const unsigned char quicksand_medium_5px_char_084_bitmap[] = {
    0x66, 0xc3, 0x71, /* 行 0 */
    0x00, 0x84, 0x00, /* 行 1 */
    0x00, 0x84, 0x00, /* 行 2 */
    0x00, 0x79, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_084 = {
    .cover_map = quicksand_medium_5px_char_084_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  85: U    */
static const unsigned char quicksand_medium_5px_char_085_bitmap[] = {
    0x72, 0x00, 0x4e, 0x1c, /* 行 0 */
    0x7c, 0x00, 0x54, 0x20, /* 行 1 */
    0x7a, 0x00, 0x5c, 0x17, /* 行 2 */
    0x33, 0x8b, 0x84, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_085 = {
    .cover_map = quicksand_medium_5px_char_085_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 4
};

/* 字符  86: V    */
static const unsigned char quicksand_medium_5px_char_086_bitmap[] = {
    0x75, 0x00, 0x5e, 0x1a, /* 行 0 */
    0x65, 0x1d, 0x80, 0x00, /* 行 1 */
    0x11, 0x8b, 0x62, 0x00, /* 行 2 */
    0x00, 0x9d, 0x0f, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_086 = {
    .cover_map = quicksand_medium_5px_char_086_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  87: W    */
static const unsigned char quicksand_medium_5px_char_087_bitmap[] = {
    0x7a, 0x00, 0x00, 0x01, 0x7a, /* 行 0 */
    0x7e, 0x0d, 0xa8, 0x36, 0x51, /* 行 1 */
    0x3e, 0x98, 0x83, 0x96, 0x0b, /* 行 2 */
    0x03, 0xa5, 0x0a, 0xa0, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_087 = {
    .cover_map = quicksand_medium_5px_char_087_bitmap,
    .width = 5,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 5
};

/* 字符  88: X    */
static const unsigned char quicksand_medium_5px_char_088_bitmap[] = {
    0x7c, 0x09, 0x7b, /* 行 0 */
    0x1a, 0xb2, 0x39, /* 行 1 */
    0x18, 0xb4, 0x36, /* 行 2 */
    0x74, 0x07, 0x87, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_088 = {
    .cover_map = quicksand_medium_5px_char_088_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  89: Y    */
static const unsigned char quicksand_medium_5px_char_089_bitmap[] = {
    0x85, 0x06, 0x7d, /* 行 0 */
    0x2f, 0xb8, 0x26, /* 行 1 */
    0x00, 0x7b, 0x00, /* 行 2 */
    0x00, 0x6e, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_089 = {
    .cover_map = quicksand_medium_5px_char_089_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  90: Z    */
static const unsigned char quicksand_medium_5px_char_090_bitmap[] = {
    0x47, 0x84, 0xcc, 0x00, /* 行 0 */
    0x00, 0x59, 0x46, 0x00, /* 行 1 */
    0x18, 0x87, 0x00, 0x00, /* 行 2 */
    0x9f, 0x8e, 0x74, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_090 = {
    .cover_map = quicksand_medium_5px_char_090_bitmap,
    .width = 4,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  91: [    */
static const unsigned char quicksand_medium_5px_char_091_bitmap[] = {
    0x80, 0x5b, /* 行 0 */
    0x74, 0x00, /* 行 1 */
    0x74, 0x00, /* 行 2 */
    0x74, 0x00, /* 行 3 */
    0x80, 0x5b, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_091 = {
    .cover_map = quicksand_medium_5px_char_091_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符  92: \    */
static const unsigned char quicksand_medium_5px_char_092_bitmap[] = {
    0x6b, 0x00, 0x00, /* 行 0 */
    0x4f, 0x26, 0x00, /* 行 1 */
    0x02, 0x73, 0x00, /* 行 2 */
    0x00, 0x5d, 0x18, /* 行 3 */
    0x00, 0x07, 0x65, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_092 = {
    .cover_map = quicksand_medium_5px_char_092_bitmap,
    .width = 3,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  93: ]    */
static const unsigned char quicksand_medium_5px_char_093_bitmap[] = {
    0x5b, 0x82, /* 行 0 */
    0x00, 0x74, /* 行 1 */
    0x00, 0x74, /* 行 2 */
    0x00, 0x74, /* 行 3 */
    0x5b, 0x82, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_093 = {
    .cover_map = quicksand_medium_5px_char_093_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符  94: ^    */
static const unsigned char quicksand_medium_5px_char_094_bitmap[] = {
    0x1c, 0xc2, 0x0d, /* 行 0 */
    0x76, 0x21, 0x6a, /* 行 1 */
    0x00, 0x00, 0x00, /* 行 2 */
    0x00, 0x00, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_094 = {
    .cover_map = quicksand_medium_5px_char_094_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  95: _    */
static const unsigned char quicksand_medium_5px_char_095_bitmap[] = {
    0x49, 0x6c, 0x6b, 0x0c, /* 行 0 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_095 = {
    .cover_map = quicksand_medium_5px_char_095_bitmap,
    .width = 4,
    .height = 1,
    .bearing_x = 0,
    .bearing_y = 0,
    .advance = 3
};

/* 字符  96: `    */
static const unsigned char quicksand_medium_5px_char_096_bitmap[] = {
    0x6f, /* 行 0 */
    0x00, /* 行 1 */
    0x00, /* 行 2 */
    0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_096 = {
    .cover_map = quicksand_medium_5px_char_096_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

/* 字符  97: a    */
static const unsigned char quicksand_medium_5px_char_097_bitmap[] = {
    0x62, 0x87, 0x94, /* 行 0 */
    0x76, 0x00, 0x81, /* 行 1 */
    0x5e, 0x73, 0x94, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_097 = {
    .cover_map = quicksand_medium_5px_char_097_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符  98: b    */
static const unsigned char quicksand_medium_5px_char_098_bitmap[] = {
    0x6e, 0x00, 0x00, /* 行 0 */
    0x90, 0x85, 0x6b, /* 行 1 */
    0x7f, 0x00, 0x74, /* 行 2 */
    0x8a, 0x7c, 0x68, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_098 = {
    .cover_map = quicksand_medium_5px_char_098_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符  99: c    */
static const unsigned char quicksand_medium_5px_char_099_bitmap[] = {
    0x61, 0x85, 0x33, /* 行 0 */
    0x7c, 0x00, 0x00, /* 行 1 */
    0x62, 0x7b, 0x2e, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_099 = {
    .cover_map = quicksand_medium_5px_char_099_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 100: d    */
static const unsigned char quicksand_medium_5px_char_100_bitmap[] = {
    0x00, 0x00, 0x6e, /* 行 0 */
    0x62, 0x85, 0x9d, /* 行 1 */
    0x76, 0x00, 0x81, /* 行 2 */
    0x5e, 0x73, 0x94, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_100 = {
    .cover_map = quicksand_medium_5px_char_100_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符 101: e    */
static const unsigned char quicksand_medium_5px_char_101_bitmap[] = {
    0x5d, 0x8c, 0x47, /* 行 0 */
    0x9b, 0x74, 0x66, /* 行 1 */
    0x5d, 0x7a, 0x3b, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_101 = {
    .cover_map = quicksand_medium_5px_char_101_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 102: f    */
static const unsigned char quicksand_medium_5px_char_102_bitmap[] = {
    0x33, 0x80, /* 行 0 */
    0x8d, 0x7c, /* 行 1 */
    0x50, 0x28, /* 行 2 */
    0x4b, 0x23, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_102 = {
    .cover_map = quicksand_medium_5px_char_102_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符 103: g    */
static const unsigned char quicksand_medium_5px_char_103_bitmap[] = {
    0x62, 0x85, 0x9b, /* 行 0 */
    0x78, 0x00, 0x81, /* 行 1 */
    0x60, 0x79, 0xa1, /* 行 2 */
    0x33, 0x82, 0x63, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_103 = {
    .cover_map = quicksand_medium_5px_char_103_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 104: h    */
static const unsigned char quicksand_medium_5px_char_104_bitmap[] = {
    0x6e, 0x00, 0x00, /* 行 0 */
    0x8d, 0x86, 0x5a, /* 行 1 */
    0x7a, 0x00, 0x7c, /* 行 2 */
    0x6f, 0x00, 0x72, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_104 = {
    .cover_map = quicksand_medium_5px_char_104_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符 105: i    */
static const unsigned char quicksand_medium_5px_char_105_bitmap[] = {
    0x42, /* 行 0 */
    0x72, /* 行 1 */
    0x7c, /* 行 2 */
    0x72, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_105 = {
    .cover_map = quicksand_medium_5px_char_105_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

/* 字符 106: j    */
static const unsigned char quicksand_medium_5px_char_106_bitmap[] = {
    0x00, 0x42, /* 行 0 */
    0x00, 0x72, /* 行 1 */
    0x00, 0x7c, /* 行 2 */
    0x00, 0x7c, /* 行 3 */
    0x05, 0x91, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_106 = {
    .cover_map = quicksand_medium_5px_char_106_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = -1,
    .bearing_y = 4,
    .advance = 1
};

/* 字符 107: k    */
static const unsigned char quicksand_medium_5px_char_107_bitmap[] = {
    0x72, 0x00, 0x00, /* 行 0 */
    0x7c, 0x50, 0x44, /* 行 1 */
    0x91, 0xad, 0x00, /* 行 2 */
    0x72, 0x38, 0x59, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_107 = {
    .cover_map = quicksand_medium_5px_char_107_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字符 108: l    */
static const unsigned char quicksand_medium_5px_char_108_bitmap[] = {
    0x72, /* 行 0 */
    0x7c, /* 行 1 */
    0x7c, /* 行 2 */
    0x72, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_108 = {
    .cover_map = quicksand_medium_5px_char_108_bitmap,
    .width = 1,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 1
};

/* 字符 109: m    */
static const unsigned char quicksand_medium_5px_char_109_bitmap[] = {
    0x83, 0x7d, 0x8d, 0x9a, 0x18, /* 行 0 */
    0x79, 0x00, 0x7e, 0x3d, 0x3b, /* 行 1 */
    0x6e, 0x00, 0x72, 0x37, 0x37, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_109 = {
    .cover_map = quicksand_medium_5px_char_109_bitmap,
    .width = 5,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 5
};

/* 字符 110: n    */
static const unsigned char quicksand_medium_5px_char_110_bitmap[] = {
    0x81, 0x72, 0x64, /* 行 0 */
    0x78, 0x00, 0x7c, /* 行 1 */
    0x6e, 0x00, 0x72, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_110 = {
    .cover_map = quicksand_medium_5px_char_110_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 111: o    */
static const unsigned char quicksand_medium_5px_char_111_bitmap[] = {
    0x5d, 0x84, 0x5e, /* 行 0 */
    0x7b, 0x00, 0x77, /* 行 1 */
    0x5d, 0x79, 0x5e, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_111 = {
    .cover_map = quicksand_medium_5px_char_111_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 112: p    */
static const unsigned char quicksand_medium_5px_char_112_bitmap[] = {
    0x88, 0x79, 0x69, /* 行 0 */
    0x7f, 0x00, 0x74, /* 行 1 */
    0x92, 0x7c, 0x69, /* 行 2 */
    0x6f, 0x00, 0x00, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_112 = {
    .cover_map = quicksand_medium_5px_char_112_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 113: q    */
static const unsigned char quicksand_medium_5px_char_113_bitmap[] = {
    0x62, 0x85, 0x93, /* 行 0 */
    0x76, 0x00, 0x81, /* 行 1 */
    0x60, 0x7a, 0x9d, /* 行 2 */
    0x00, 0x00, 0x6f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_113 = {
    .cover_map = quicksand_medium_5px_char_113_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 114: r    */
static const unsigned char quicksand_medium_5px_char_114_bitmap[] = {
    0x81, 0x65, /* 行 0 */
    0x7b, 0x00, /* 行 1 */
    0x6e, 0x00, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_114 = {
    .cover_map = quicksand_medium_5px_char_114_bitmap,
    .width = 2,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

/* 字符 115: s    */
static const unsigned char quicksand_medium_5px_char_115_bitmap[] = {
    0x6f, 0x7d, 0x08, /* 行 0 */
    0x4e, 0x80, 0x07, /* 行 1 */
    0x62, 0x84, 0x14, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_115 = {
    .cover_map = quicksand_medium_5px_char_115_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

/* 字符 116: t    */
static const unsigned char quicksand_medium_5px_char_116_bitmap[] = {
    0x2f, 0x06, /* 行 0 */
    0xa2, 0x5a, /* 行 1 */
    0x68, 0x10, /* 行 2 */
    0x51, 0x4e, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_116 = {
    .cover_map = quicksand_medium_5px_char_116_bitmap,
    .width = 2,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符 117: u    */
static const unsigned char quicksand_medium_5px_char_117_bitmap[] = {
    0x6e, 0x00, 0x72, /* 行 0 */
    0x78, 0x00, 0x7c, /* 行 1 */
    0x60, 0x8a, 0x4e, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_117 = {
    .cover_map = quicksand_medium_5px_char_117_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 118: v    */
static const unsigned char quicksand_medium_5px_char_118_bitmap[] = {
    0x72, 0x0a, 0x5f, /* 行 0 */
    0x4a, 0x77, 0x11, /* 行 1 */
    0x01, 0x5c, 0x00, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_118 = {
    .cover_map = quicksand_medium_5px_char_118_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 119: w    */
static const unsigned char quicksand_medium_5px_char_119_bitmap[] = {
    0x72, 0x28, 0x12, 0x70, /* 行 0 */
    0x69, 0x80, 0x9a, 0x3d, /* 行 1 */
    0x19, 0x5e, 0x75, 0x01, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_119 = {
    .cover_map = quicksand_medium_5px_char_119_bitmap,
    .width = 4,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 4
};

/* 字符 120: x    */
static const unsigned char quicksand_medium_5px_char_120_bitmap[] = {
    0x83, 0x5e, 0x24, /* 行 0 */
    0x3d, 0xaa, 0x00, /* 行 1 */
    0x7c, 0x64, 0x27, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_120 = {
    .cover_map = quicksand_medium_5px_char_120_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

/* 字符 121: y    */
static const unsigned char quicksand_medium_5px_char_121_bitmap[] = {
    0x6e, 0x00, 0x72, /* 行 0 */
    0x79, 0x00, 0x7d, /* 行 1 */
    0x67, 0x82, 0x86, /* 行 2 */
    0x3f, 0x8b, 0x43, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_121 = {
    .cover_map = quicksand_medium_5px_char_121_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 3
};

/* 字符 122: z    */
static const unsigned char quicksand_medium_5px_char_122_bitmap[] = {
    0x55, 0xc4, 0x1a, /* 行 0 */
    0x21, 0x73, 0x00, /* 行 1 */
    0xa6, 0x82, 0x15, /* 行 2 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_122 = {
    .cover_map = quicksand_medium_5px_char_122_bitmap,
    .width = 3,
    .height = 3,
    .bearing_x = 0,
    .bearing_y = 3,
    .advance = 2
};

/* 字符 123: {    */
static const unsigned char quicksand_medium_5px_char_123_bitmap[] = {
    0x11, 0x7b, /* 行 0 */
    0x3d, 0x35, /* 行 1 */
    0x92, 0x0d, /* 行 2 */
    0x3c, 0x34, /* 行 3 */
    0x16, 0x77, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_123 = {
    .cover_map = quicksand_medium_5px_char_123_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符 124: |    */
static const unsigned char quicksand_medium_5px_char_124_bitmap[] = {
    0x34, /* 行 0 */
    0x7c, /* 行 1 */
    0x7c, /* 行 2 */
    0x7c, /* 行 3 */
    0x7c, /* 行 4 */
    0x72, /* 行 5 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_124 = {
    .cover_map = quicksand_medium_5px_char_124_bitmap,
    .width = 1,
    .height = 6,
    .bearing_x = 0,
    .bearing_y = 5,
    .advance = 1
};

/* 字符 125: }    */
static const unsigned char quicksand_medium_5px_char_125_bitmap[] = {
    0x6f, 0x1f, /* 行 0 */
    0x27, 0x4c, /* 行 1 */
    0x07, 0x9c, /* 行 2 */
    0x27, 0x4c, /* 行 3 */
    0x6d, 0x1d, /* 行 4 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_125 = {
    .cover_map = quicksand_medium_5px_char_125_bitmap,
    .width = 2,
    .height = 5,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 2
};

/* 字符 126: ~    */
static const unsigned char quicksand_medium_5px_char_126_bitmap[] = {
    0x45, 0x6d, 0x16, /* 行 0 */
    0x00, 0x00, 0x00, /* 行 1 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_126 = {
    .cover_map = quicksand_medium_5px_char_126_bitmap,
    .width = 3,
    .height = 2,
    .bearing_x = 0,
    .bearing_y = 2,
    .advance = 3
};

/* 字符 127: DEL  */
static const unsigned char quicksand_medium_5px_char_127_bitmap[] = {
    0x36, 0x28, 0x26, /* 行 0 */
    0x3f, 0x12, 0x28, /* 行 1 */
    0x24, 0x2e, 0x29, /* 行 2 */
    0x3a, 0x2a, 0x5f, /* 行 3 */
};

static const ipgui_glyph_t quicksand_medium_5px_char_127 = {
    .cover_map = quicksand_medium_5px_char_127_bitmap,
    .width = 3,
    .height = 4,
    .bearing_x = 0,
    .bearing_y = 4,
    .advance = 3
};

/* 字形数组 */
static const ipgui_glyph_t quicksand_medium_5px_glyph_array[128] = {
    quicksand_medium_5px_char_000,  /*   0: NUL  */
    quicksand_medium_5px_char_001,  /*   1: SOH  */
    quicksand_medium_5px_char_002,  /*   2: STX  */
    quicksand_medium_5px_char_003,  /*   3: ETX  */
    quicksand_medium_5px_char_004,  /*   4: EOT  */
    quicksand_medium_5px_char_005,  /*   5: ENQ  */
    quicksand_medium_5px_char_006,  /*   6: ACK  */
    quicksand_medium_5px_char_007,  /*   7: BEL  */
    quicksand_medium_5px_char_008,  /*   8: BS   */
    quicksand_medium_5px_char_009,  /*   9: HT   */
    quicksand_medium_5px_char_010,  /*  10: LF   */
    quicksand_medium_5px_char_011,  /*  11: VT   */
    quicksand_medium_5px_char_012,  /*  12: FF   */
    quicksand_medium_5px_char_013,  /*  13: CR   */
    quicksand_medium_5px_char_014,  /*  14: SO   */
    quicksand_medium_5px_char_015,  /*  15: SI   */
    quicksand_medium_5px_char_016,  /*  16: DLE  */
    quicksand_medium_5px_char_017,  /*  17: DC1  */
    quicksand_medium_5px_char_018,  /*  18: DC2  */
    quicksand_medium_5px_char_019,  /*  19: DC3  */
    quicksand_medium_5px_char_020,  /*  20: DC4  */
    quicksand_medium_5px_char_021,  /*  21: NAK  */
    quicksand_medium_5px_char_022,  /*  22: SYN  */
    quicksand_medium_5px_char_023,  /*  23: ETB  */
    quicksand_medium_5px_char_024,  /*  24: CAN  */
    quicksand_medium_5px_char_025,  /*  25: EM   */
    quicksand_medium_5px_char_026,  /*  26: SUB  */
    quicksand_medium_5px_char_027,  /*  27: ESC  */
    quicksand_medium_5px_char_028,  /*  28: FS   */
    quicksand_medium_5px_char_029,  /*  29: GS   */
    quicksand_medium_5px_char_030,  /*  30: RS   */
    quicksand_medium_5px_char_031,  /*  31: US   */
    quicksand_medium_5px_char_032,  /*  32: SPACE */
    quicksand_medium_5px_char_033,  /*  33: !    */
    quicksand_medium_5px_char_034,  /*  34: "    */
    quicksand_medium_5px_char_035,  /*  35: #    */
    quicksand_medium_5px_char_036,  /*  36: $    */
    quicksand_medium_5px_char_037,  /*  37: %    */
    quicksand_medium_5px_char_038,  /*  38: &    */
    quicksand_medium_5px_char_039,  /*  39: '    */
    quicksand_medium_5px_char_040,  /*  40: (    */
    quicksand_medium_5px_char_041,  /*  41: )    */
    quicksand_medium_5px_char_042,  /*  42: *    */
    quicksand_medium_5px_char_043,  /*  43: +    */
    quicksand_medium_5px_char_044,  /*  44: ,    */
    quicksand_medium_5px_char_045,  /*  45: -    */
    quicksand_medium_5px_char_046,  /*  46: .    */
    quicksand_medium_5px_char_047,  /*  47: /    */
    quicksand_medium_5px_char_048,  /*  48: 0    */
    quicksand_medium_5px_char_049,  /*  49: 1    */
    quicksand_medium_5px_char_050,  /*  50: 2    */
    quicksand_medium_5px_char_051,  /*  51: 3    */
    quicksand_medium_5px_char_052,  /*  52: 4    */
    quicksand_medium_5px_char_053,  /*  53: 5    */
    quicksand_medium_5px_char_054,  /*  54: 6    */
    quicksand_medium_5px_char_055,  /*  55: 7    */
    quicksand_medium_5px_char_056,  /*  56: 8    */
    quicksand_medium_5px_char_057,  /*  57: 9    */
    quicksand_medium_5px_char_058,  /*  58: :    */
    quicksand_medium_5px_char_059,  /*  59: ;    */
    quicksand_medium_5px_char_060,  /*  60: <    */
    quicksand_medium_5px_char_061,  /*  61: =    */
    quicksand_medium_5px_char_062,  /*  62: >    */
    quicksand_medium_5px_char_063,  /*  63: ?    */
    quicksand_medium_5px_char_064,  /*  64: @    */
    quicksand_medium_5px_char_065,  /*  65: A    */
    quicksand_medium_5px_char_066,  /*  66: B    */
    quicksand_medium_5px_char_067,  /*  67: C    */
    quicksand_medium_5px_char_068,  /*  68: D    */
    quicksand_medium_5px_char_069,  /*  69: E    */
    quicksand_medium_5px_char_070,  /*  70: F    */
    quicksand_medium_5px_char_071,  /*  71: G    */
    quicksand_medium_5px_char_072,  /*  72: H    */
    quicksand_medium_5px_char_073,  /*  73: I    */
    quicksand_medium_5px_char_074,  /*  74: J    */
    quicksand_medium_5px_char_075,  /*  75: K    */
    quicksand_medium_5px_char_076,  /*  76: L    */
    quicksand_medium_5px_char_077,  /*  77: M    */
    quicksand_medium_5px_char_078,  /*  78: N    */
    quicksand_medium_5px_char_079,  /*  79: O    */
    quicksand_medium_5px_char_080,  /*  80: P    */
    quicksand_medium_5px_char_081,  /*  81: Q    */
    quicksand_medium_5px_char_082,  /*  82: R    */
    quicksand_medium_5px_char_083,  /*  83: S    */
    quicksand_medium_5px_char_084,  /*  84: T    */
    quicksand_medium_5px_char_085,  /*  85: U    */
    quicksand_medium_5px_char_086,  /*  86: V    */
    quicksand_medium_5px_char_087,  /*  87: W    */
    quicksand_medium_5px_char_088,  /*  88: X    */
    quicksand_medium_5px_char_089,  /*  89: Y    */
    quicksand_medium_5px_char_090,  /*  90: Z    */
    quicksand_medium_5px_char_091,  /*  91: [    */
    quicksand_medium_5px_char_092,  /*  92: \    */
    quicksand_medium_5px_char_093,  /*  93: ]    */
    quicksand_medium_5px_char_094,  /*  94: ^    */
    quicksand_medium_5px_char_095,  /*  95: _    */
    quicksand_medium_5px_char_096,  /*  96: `    */
    quicksand_medium_5px_char_097,  /*  97: a    */
    quicksand_medium_5px_char_098,  /*  98: b    */
    quicksand_medium_5px_char_099,  /*  99: c    */
    quicksand_medium_5px_char_100,  /* 100: d    */
    quicksand_medium_5px_char_101,  /* 101: e    */
    quicksand_medium_5px_char_102,  /* 102: f    */
    quicksand_medium_5px_char_103,  /* 103: g    */
    quicksand_medium_5px_char_104,  /* 104: h    */
    quicksand_medium_5px_char_105,  /* 105: i    */
    quicksand_medium_5px_char_106,  /* 106: j    */
    quicksand_medium_5px_char_107,  /* 107: k    */
    quicksand_medium_5px_char_108,  /* 108: l    */
    quicksand_medium_5px_char_109,  /* 109: m    */
    quicksand_medium_5px_char_110,  /* 110: n    */
    quicksand_medium_5px_char_111,  /* 111: o    */
    quicksand_medium_5px_char_112,  /* 112: p    */
    quicksand_medium_5px_char_113,  /* 113: q    */
    quicksand_medium_5px_char_114,  /* 114: r    */
    quicksand_medium_5px_char_115,  /* 115: s    */
    quicksand_medium_5px_char_116,  /* 116: t    */
    quicksand_medium_5px_char_117,  /* 117: u    */
    quicksand_medium_5px_char_118,  /* 118: v    */
    quicksand_medium_5px_char_119,  /* 119: w    */
    quicksand_medium_5px_char_120,  /* 120: x    */
    quicksand_medium_5px_char_121,  /* 121: y    */
    quicksand_medium_5px_char_122,  /* 122: z    */
    quicksand_medium_5px_char_123,  /* 123: {    */
    quicksand_medium_5px_char_124,  /* 124: |    */
    quicksand_medium_5px_char_125,  /* 125: }    */
    quicksand_medium_5px_char_126,  /* 126: ~    */
    quicksand_medium_5px_char_127,  /* 127: DEL  */
};

/* 字体结构体 */
const ipgui_font_t quicksand_medium_5px = {
    .glyphs = (ipgui_glyph_t*)quicksand_medium_5px_glyph_array,
    .line_height = 8,
    .baseline = 6,
    .font_size = 5,
    .max_height = 6,
    .space_width = 1
};

/* 获取字形函数 */
const ipgui_glyph_t* quicksand_medium_5px_get_glyph(unsigned char char_code)
{
    if (char_code < 128) {
        return &quicksand_medium_5px_glyph_array[char_code];
    }
    return &quicksand_medium_5px_char_063;  /* 返回'?'字符 */
}

/* 文本宽度计算 */
unsigned short quicksand_medium_5px_text_width(const char* text)
{
    unsigned short width = 0;
    while (*text) {
        unsigned char ch = (unsigned char)*text;
        if (ch < 128) {
            const ipgui_glyph_t* glyph = &quicksand_medium_5px_glyph_array[ch];
            width += glyph->advance;
        }
        text++;
    }
    return width;
}
