#include "clockui.h"
#include <string.h>
#include <stdlib.h>
#include "llist.h"
#include "nexlink_app.h"

//  Hexadecimal code corresponding to each character
//  0123456789:
const uint8_t hex_codes_3x3[][3] = {
    {0x00, 0x00, 0x00}, /*" ", 32(ASCII)*/
    {0x07, 0x05, 0x07}, /*"0", 48(ASCII)*/
    {0x05, 0x07, 0x04}, /*"1", 49(ASCII)*/
    {0x01, 0x07, 0x04}, /*"2", 50(ASCII)*/
    {0x05, 0x07, 0x02}, /*"3", 51(ASCII)*/
    {0x03, 0x02, 0x07}, /*"4", 52(ASCII)*/
    {0x04, 0x07, 0x01}, /*"5", 53(ASCII)*/
    {0x07, 0x06, 0x06}, /*"6", 54(ASCII)*/
    {0x01, 0x01, 0x07}, /*"7", 55(ASCII)*/
    {0x07, 0x07, 0x07}, /*"8", 56(ASCII)*/
    {0x03, 0x03, 0x07}, /*"9", 57(ASCII)*/
    {0x05, 0x00, 0x00}, /*":", 58(ASCII)*/
};
//  Hexadecimal code corresponding to each character
//  0123456789:
const uint8_t hex_codes_3x4[][3] = {
    {0x00, 0x00, 0x00}, /*" ", 32(ASCII)*/
    {0x0F, 0x09, 0x0F}, /*"0", 48(ASCII)*/
    {0x0A, 0x0F, 0x08}, /*"1", 49(ASCII)*/
    {0x0D, 0x0D, 0x0A}, /*"2", 50(ASCII)*/
    {0x0D, 0x0F, 0x06}, /*"3", 51(ASCII)*/
    {0x07, 0x04, 0x0F}, /*"4", 52(ASCII)*/
    {0x0B, 0x0D, 0x0D}, /*"5", 53(ASCII)*/
    {0x0F, 0x0D, 0x0D}, /*"6", 54(ASCII)*/
    {0x01, 0x0D, 0x03}, /*"7", 55(ASCII)*/
    {0x0F, 0x0D, 0x0F}, /*"8", 56(ASCII)*/
    {0x0B, 0x0B, 0x0F}, /*"9", 57(ASCII)*/
    {0x0D, 0x00, 0x00}, /*":", 58(ASCII)*/
};
//  Hexadecimal code corresponding to each character
//  0123456789
const uint8_t hex_codes_4x8[][4] = {
    {0x00, 0x00, 0x00, 0x00}, /*" ", 58(ASCII)*/
    {0xFF, 0x81, 0x81, 0xFF}, /*"0", 32(ASCII)*/
    {0x00, 0x00, 0xFF, 0x00}, /*"1", 48(ASCII)*/
    {0xF1, 0x91, 0x91, 0x9F}, /*"2", 49(ASCII)*/
    {0x81, 0x91, 0x91, 0xFF}, /*"3", 50(ASCII)*/
    {0x1E, 0x10, 0x10, 0xFF}, /*"4", 51(ASCII)*/
    {0x9F, 0x91, 0x91, 0xF1}, /*"5", 52(ASCII)*/
    {0xFF, 0x91, 0x91, 0xF1}, /*"6", 53(ASCII)*/
    {0x01, 0x01, 0xF1, 0x1F}, /*"7", 54(ASCII)*/
    {0xFF, 0x91, 0x91, 0xFF}, /*"8", 55(ASCII)*/
    {0x9F, 0x91, 0x91, 0xFF}, /*"9", 56(ASCII)*/
};
//  Hexadecimal code corresponding to each character
//  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz
const uint8_t hex_codes_3x5[CHAR_COUNT][3] = {
    {0x00, 0x00, 0x00}, /*" ", 32(ASCII)*/
    {0x00, 0x17, 0x00}, /*"!", 33(ASCII)*/
    {0x00, 0x17, 0x00}, /*""", 34(ASCII)*/
    {0x1F, 0x0A, 0x1F}, /*"#", 35(ASCII)*/
    {0x16, 0x1F, 0x0D}, /*"$", 36(ASCII)*/
    {0x19, 0x04, 0x13}, /*"%", 37(ASCII)*/
    {0x0B, 0x15, 0x1A}, /*"&", 38(AFSCII)*/
    {0x00, 0x03, 0x00}, /*"'", 39(ASCII)*/
    {0x0E, 0x11, 0x00}, /*"(", 40(ASCII)*/
    {0x00, 0x11, 0x0E}, /*")", 41(ASCII)*/
    {0x02, 0x05, 0x02}, /*"*", 42(ASCII)*/
    {0x04, 0x0E, 0x04}, /*"+", 43(ASCII)*/
    {0x00, 0x18, 0x00}, /*",", 44(ASCII)*/
    {0x04, 0x04, 0x04}, /*"-", 45(ASCII)*/
    {0x00, 0x10, 0x00}, /*".", 46(ASCII)*/
    {0x18, 0x04, 0x03}, /*"/", 47(ASCII)*/
    {0x1F, 0x11, 0x1F}, /*"0", 48(ASCII)*/
    {0x12, 0x1F, 0x10}, /*"1", 49(ASCII)*/
    {0x1D, 0x15, 0x17}, /*"2", 50(ASCII)*/
    {0x11, 0x15, 0x1F}, /*"3", 51(ASCII)*/
    {0x07, 0x04, 0x1F}, /*"4", 52(ASCII)*/
    {0x17, 0x15, 0x1D}, /*"5", 53(ASCII)*/
    {0x1E, 0x15, 0x1D}, /*"6", 54(ASCII)*/
    {0x01, 0x1D, 0x03}, /*"7", 55(ASCII)*/
    {0x1F, 0x15, 0x1F}, /*"8", 56(ASCII)*/
    {0x17, 0x15, 0x0F}, /*"9", 57(ASCII)*/
    {0x0A, 0x00, 0x00}, /*":", 58(ASCII)*/
    {0x00, 0x1A, 0x00}, /*";", 59(ASCII)*/
    {0x04, 0x0A, 0x11}, /*"<", 60(ASCII)*/
    {0x0A, 0x0A, 0x0A}, /*"=", 61(ASCII)*/
    {0x11, 0x0A, 0x04}, /*">", 62(ASCII)*/
    {0x01, 0x15, 0x03}, /*"?", 63(ASCII)*/
    {0x1F, 0x11, 0x17}, /*"@", 64(ASCII)*/
    {0x1E, 0x05, 0x1E}, /*"A", 65(ASCII)*/
    {0x1F, 0x15, 0x0E}, /*"B", 66(ASCII)*/
    {0x0E, 0x11, 0x11}, /*"C", 67(ASCII)*/
    {0x1F, 0x11, 0x0E}, /*"D", 68(ASCII)*/
    {0x1F, 0x15, 0x15}, /*"E", 69(ASCII)*/
    {0x1F, 0x05, 0x05}, /*"F", 70(ASCII)*/
    {0x0E, 0x11, 0x1D}, /*"G", 71(ASCII)*/
    {0x1F, 0x04, 0x1F}, /*"H", 72(ASCII)*/
    {0x11, 0x1F, 0x11}, /*"I", 73(ASCII)*/
    {0x08, 0x11, 0x0F}, /*"J", 74(ASCII)*/
    {0x1F, 0x04, 0x1B}, /*"K", 75(ASCII)*/
    {0x1F, 0x10, 0x10}, /*"L", 76(ASCII)*/
    {0x1F, 0x02, 0x1F}, /*"M", 77(ASCII)*/
    {0x1F, 0x01, 0x1E}, /*"N", 78(ASCII)*/
    {0x0E, 0x11, 0x0E}, /*"O", 79(ASCII)*/
    {0x1F, 0x05, 0x02}, /*"P", 80(ASCII)*/
    {0x0E, 0x19, 0x1E}, /*"Q", 81(ASCII)*/
    {0x1F, 0x05, 0x1A}, /*"R", 82(ASCII)*/
    {0x16, 0x15, 0x0D}, /*"S", 83(ASCII)*/
    {0x01, 0x1F, 0x01}, /*"T", 84(ASCII)*/
    {0x1F, 0x10, 0x1F}, /*"U", 85(ASCII)*/
    {0x07, 0x18, 0x07}, /*"V", 86(ASCII)*/
    {0x1F, 0x08, 0x1F}, /*"W", 87(ASCII)*/
    {0x1B, 0x04, 0x1B}, /*"X", 88(ASCII)*/
    {0x03, 0x1C, 0x03}, /*"Y", 89(ASCII)*/
    {0x19, 0x15, 0x13}, /*"Z", 90(ASCII)*/
    {0x1F, 0x11, 0x00}, /*"[", 91(ASCII)*/
    {0x01, 0x0E, 0x10}, /*"\", 92(ASCII)*/
    {0x00, 0x11, 0x1F}, /*"]", 93(ASCII)*/
    {0x02, 0x01, 0x02}, /*"^", 94(ASCII)*/
    {0x10, 0x10, 0x10}, /*"_", 95(ASCII)*/
    {0x00, 0x01, 0x01}, /*"`", 96(ASCII)*/
    {0x1E, 0x05, 0x1E}, /*"A", 65(ASCII)*/
    {0x1F, 0x15, 0x0E}, /*"B", 66(ASCII)*/
    {0x0E, 0x11, 0x11}, /*"C", 67(ASCII)*/
    {0x1F, 0x11, 0x0E}, /*"D", 68(ASCII)*/
    {0x1F, 0x15, 0x15}, /*"E", 69(ASCII)*/
    {0x1F, 0x05, 0x05}, /*"F", 70(ASCII)*/
    {0x0E, 0x11, 0x1D}, /*"G", 71(ASCII)*/
    {0x1F, 0x04, 0x1F}, /*"H", 72(ASCII)*/
    {0x11, 0x1F, 0x11}, /*"I", 73(ASCII)*/
    {0x08, 0x11, 0x0F}, /*"J", 74(ASCII)*/
    {0x1F, 0x04, 0x1B}, /*"K", 75(ASCII)*/
    {0x1F, 0x10, 0x10}, /*"L", 76(ASCII)*/
    {0x1F, 0x02, 0x1F}, /*"M", 77(ASCII)*/
    {0x1F, 0x01, 0x1E}, /*"N", 78(ASCII)*/
    {0x0E, 0x11, 0x0E}, /*"O", 79(ASCII)*/
    {0x1F, 0x05, 0x02}, /*"P", 80(ASCII)*/
    {0x0E, 0x19, 0x1E}, /*"Q", 81(ASCII)*/
    {0x1F, 0x05, 0x1A}, /*"R", 82(ASCII)*/
    {0x16, 0x15, 0x0D}, /*"S", 83(ASCII)*/
    {0x01, 0x1F, 0x01}, /*"T", 84(ASCII)*/
    {0x1F, 0x10, 0x1F}, /*"U", 85(ASCII)*/
    {0x07, 0x18, 0x07}, /*"V", 86(ASCII)*/
    {0x1F, 0x08, 0x1F}, /*"W", 87(ASCII)*/
    {0x1B, 0x04, 0x1B}, /*"X", 88(ASCII)*/
    {0x03, 0x1C, 0x03}, /*"Y", 89(ASCII)*/
    {0x19, 0x15, 0x13}, /*"Z", 90(ASCII)*/
    {0x1F, 0x1F, 0x11}, /*"{", 123(ASCII)*/
    {0x00, 0x1F, 0x00}, /*"|", 124(ASCII)*/
    {0x11, 0x1F, 0x1F}, /*"}", 125(ASCII)*/
    {0x06, 0x04, 0x0C}, /*"~", 126(ASCII)*/
    {0x1F, 0x1F, 0x1F}, /*"", 127(ASCII)*/
};

//  Hexadecimal code corresponding to each character
//  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz
const uint8_t hex_codes[CHAR_COUNT][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /*" ", 32(ASCII)*/
    {0x00, 0x00, 0x4F, 0x00, 0x00}, /*"!", 33(ASCII)*/
    {0x00, 0x03, 0x00, 0x03, 0x00}, /*""", 34(ASCII)*/
    {0x22, 0x7F, 0x22, 0x7F, 0x22}, /*"#", 35(ASCII)*/
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, /*"$", 36(ASCII)*/
    {0x23, 0x13, 0x08, 0x64, 0x62}, /*"%", 37(ASCII)*/
    {0x36, 0x49, 0x56, 0x20, 0x58}, /*"&", 38(AFSCII)*/
    {0x00, 0x02, 0x01, 0x00, 0x00}, /*"'", 39(ASCII)*/
    {0x00, 0x1C, 0x22, 0x41, 0x00}, /*"(", 40(ASCII)*/
    {0x00, 0x41, 0x22, 0x1C, 0x00}, /*")", 41(ASCII)*/
    {0x14, 0x08, 0x3E, 0x08, 0x14}, /*"*", 42(ASCII)*/
    {0x08, 0x08, 0x3E, 0x08, 0x08}, /*"+", 43(ASCII)*/
    {0x00, 0x40, 0x20, 0x00, 0x00}, /*",", 44(ASCII)*/
    {0x00, 0x08, 0x08, 0x08, 0x00}, /*"-", 45(ASCII)*/
    {0x00, 0x40, 0x00, 0x00, 0x00}, /*".", 46(ASCII)*/
    {0x20, 0x10, 0x08, 0x04, 0x02}, /*"/", 47(ASCII)*/
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /*"0", 48(ASCII)*/
    {0x00, 0x02, 0x02, 0x7F, 0x00}, /*"1", 49(ASCII)*/
    {0x62, 0x51, 0x49, 0x49, 0x46}, /*"2", 50(ASCII)*/
    {0x22, 0x41, 0x49, 0x49, 0x36}, /*"3", 51(ASCII)*/
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /*"4", 52(ASCII)*/
    {0x27, 0x45, 0x45, 0x45, 0x39}, /*"5", 53(ASCII)*/
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /*"6", 54(ASCII)*/
    {0x01, 0x01, 0x71, 0x0D, 0x03}, /*"7", 55(ASCII)*/
    {0x36, 0x49, 0x49, 0x49, 0x36}, /*"8", 56(ASCII)*/
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /*"9", 57(ASCII)*/
    {0x00, 0x00, 0x44, 0x00, 0x00}, /*":", 58(ASCII)*/
    {0x00, 0x40, 0x24, 0x00, 0x00}, /*";", 59(ASCII)*/
    {0x00, 0x08, 0x14, 0x22, 0x41}, /*"<", 60(ASCII)*/
    {0x14, 0x14, 0x14, 0x14, 0x14}, /*"=", 61(ASCII)*/
    {0x41, 0x22, 0x14, 0x08, 0x00}, /*">", 62(ASCII)*/
    {0x02, 0x01, 0x51, 0x09, 0x06}, /*"?", 63(ASCII)*/
    {0x3E, 0x41, 0x19, 0x25, 0x3E}, /*"@", 64(ASCII)*/
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, /*"A", 65(ASCII)*/
    {0x41, 0x7F, 0x49, 0x49, 0x36}, /*"B", 66(ASCII)*/
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /*"C", 67(ASCII)*/
    {0x41, 0x7F, 0x41, 0x41, 0x3E}, /*"D", 68(ASCII)*/
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /*"E", 69(ASCII)*/
    {0x7F, 0x09, 0x09, 0x09, 0x01}, /*"F", 70(ASCII)*/
    {0x3E, 0x41, 0x49, 0x49, 0x3A}, /*"G", 71(ASCII)*/
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /*"H", 72(ASCII)*/
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /*"I", 73(ASCII)*/
    {0x20, 0x40, 0x41, 0x7F, 0x01}, /*"J", 74(ASCII)*/
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /*"K", 75(ASCII)*/
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /*"L", 76(ASCII)*/
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, /*"M", 77(ASCII)*/
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /*"N", 78(ASCII)*/
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /*"O", 79(ASCII)*/
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /*"P", 80(ASCII)*/
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /*"Q", 81(ASCII)*/
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /*"R", 82(ASCII)*/
    {0x26, 0x49, 0x49, 0x49, 0x32}, /*"S", 83(ASCII)*/
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /*"T", 84(ASCII)*/
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /*"U", 85(ASCII)*/
    {0x0F, 0x30, 0x40, 0x30, 0x0F}, /*"V", 86(ASCII)*/
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, /*"W", 87(ASCII)*/
    {0x63, 0x14, 0x08, 0x14, 0x63}, /*"X", 88(ASCII)*/
    {0x07, 0x08, 0x70, 0x08, 0x07}, /*"Y", 89(ASCII)*/
    {0x61, 0x51, 0x49, 0x45, 0x43}, /*"Z", 90(ASCII)*/
    {0x00, 0x7F, 0x41, 0x41, 0x00}, /*"[", 91(ASCII)*/
    {0x02, 0x04, 0x08, 0x10, 0x20}, /*"\", 92(ASCII)*/
    {0x00, 0x41, 0x41, 0x7F, 0x00}, /*"]", 93(ASCII)*/
    {0x00, 0x02, 0x01, 0x02, 0x00}, /*"^", 94(ASCII)*/
    {0x40, 0x40, 0x40, 0x40, 0x40}, /*"_", 95(ASCII)*/
    {0x00, 0x00, 0x01, 0x02, 0x00}, /*"`", 96(ASCII)*/
    {0x20, 0x54, 0x54, 0x54, 0x78}, /*"a", 97(ASCII)*/
    {0x7F, 0x44, 0x44, 0x44, 0x38}, /*"b", 98(ASCII)*/
    {0x38, 0x44, 0x44, 0x44, 0x44}, /*"c", 99(ASCII)*/
    {0x38, 0x44, 0x44, 0x44, 0x7F}, /*"d", 100(ASCII)*/
    {0x38, 0x54, 0x54, 0x54, 0x58}, /*"e", 101(ASCII)*/
    {0x04, 0x04, 0x7E, 0x05, 0x05}, /*"f", 102(ASCII)*/
    {0x4C, 0x52, 0x52, 0x3C, 0x02}, /*"g", 103(ASCII)*/
    {0x7F, 0x08, 0x04, 0x04, 0x78}, /*"h", 104(ASCII)*/
    {0x00, 0x44, 0x7D, 0x40, 0x00}, /*"i", 105(ASCII)*/
    {0x20, 0x40, 0x44, 0x3D, 0x00}, /*"j", 106(ASCII)*/
    {0x00, 0x7F, 0x10, 0x28, 0x44}, /*"k", 107(ASCII)*/
    {0x00, 0x41, 0x7F, 0x40, 0x00}, /*"l", 108(ASCII)*/
    {0x7C, 0x04, 0x78, 0x04, 0x78}, /*"m", 109(ASCII)*/
    {0x7C, 0x08, 0x04, 0x04, 0x78}, /*"n", 110(ASCII)*/
    {0x38, 0x44, 0x44, 0x44, 0x38}, /*"o", 111(ASCII)*/
    {0x7C, 0x24, 0x24, 0x24, 0x18}, /*"p", 112(ASCII)*/
    {0x18, 0x24, 0x24, 0x24, 0x7C}, /*"q", 113(ASCII)*/
    {0x00, 0x7C, 0x08, 0x04, 0x04}, /*"r", 114(ASCII)*/
    {0x48, 0x54, 0x54, 0x54, 0x24}, /*"s", 115(ASCII)*/
    {0x00, 0x04, 0x3F, 0x44, 0x44}, /*"t", 116(ASCII)*/
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, /*"u", 117(ASCII)*/
    {0x0C, 0x30, 0x40, 0x30, 0x0C}, /*"v", 118(ASCII)*/
    {0x7C, 0x20, 0x18, 0x20, 0x7C}, /*"w", 119(ASCII)*/
    {0x44, 0x28, 0x10, 0x28, 0x44}, /*"x", 120(ASCII)*/
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, /*"y", 121(ASCII)*/
    {0x44, 0x64, 0x54, 0x4C, 0x44}, /*"z", 122(ASCII)*/
    {0x00, 0x08, 0x36, 0x41, 0x00}, /*"{", 123(ASCII)*/
    {0x00, 0x00, 0x7F, 0x00, 0x00}, /*"|", 124(ASCII)*/
    {0x00, 0x41, 0x36, 0x08, 0x00}, /*"}", 125(ASCII)*/
    {0x10, 0x08, 0x18, 0x10, 0x08}, /*"~", 126(ASCII)*/
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F}, /*"", 127(ASCII)*/

};

LList long_str_list;

#define SINGLE_LONGSTR_MODE 1

void clock_init()
{
    llist_init(&long_str_list);
		clear_all_long_str();
}

static void draw_bits(uint8_t bit_mask, int x, Color color)
{
    for (int bit = 0; bit < 8; bit++)
    {
        if (bit_mask & (1 << bit))
        {
            draw_point(x, bit, color); 
        }
    }
}

static int roll_long_str(LongStr_t *s)
{
    int position = s->current - s->str; 
    if (s->current != 0)
    {
        // 倒序遍历x轴，保留原逻辑
        for (int x = MAX_X - 1; x >= 0; x--)
        {
            uint8_t bit_mask = s->current[x];
            if (bit_mask == 0) continue; // 无点可绘，直接跳过

            // 颜色掩码1：青，原x坐标 → 调用封装函数
            if (s->color & 1)
                draw_bits(bit_mask, x, Cyan);
            // 颜色掩码2：红，原x坐标 → 调用封装函数
            if (s->color & 2)
                draw_bits(bit_mask, x, Red);

            // x+1不越界时，处理偏移坐标
            if (x + 1 < MAX_X)
            {
                // 颜色掩码4：青，x+1坐标 → 调用封装函数
                if (s->color & 4)
                    draw_bits(bit_mask, x + 1, Cyan);
                // 颜色掩码8：红，x+1坐标 → 调用封装函数
                if (s->color & 8)
                    draw_bits(bit_mask, x + 1, Red);
            }
        }

        // 保留原计数、滚动、重复、释放逻辑（无任何修改）
        static int count = 0;
        if (position % 6 == 2)
        {
            ++count;
            if (count != 5)
                return 1;
            count = 0;
        }
        s->current++;
        if (position > (s->length * 6) + MAX_X)
        {
            if (s->repeats == -1)
            {
                s->current = s->str;
            }
            else if (s->repeats > 0)
            {
                s->current = s->str;
                s->repeats--;
            }
            else
            {
                s->current = 0;
                mn11236_clear_buffers(); 
                llist_remove(&long_str_list, s);
                free(s);
                return 1;
            }
        }
    }
    return 0;
}

void roll_all_long_str()
{
    LListNode *current = long_str_list.head;
    //	if (current != NULL)
    //		clear_gram(White); // 如果有通知，清除当前显示
    while (current != NULL)
    {
        LListNode *next = current->next;
        roll_long_str(current->data);
        current = next;
    }
}

int get_long_str_count()
{
    int count = 0;

    LListNode *current = long_str_list.head;
    while (current != NULL)
    {
        current = current->next;
        count++;
    }
    return count;
}

void clear_all_long_str()
{
    LListNode *current = long_str_list.head;
    while (current != NULL)
    {
        LListNode *next = current->next;
        LongStr_t *s = current->data;
        s->current = 0;
        // memset(gram, 0, MAX_X);
        llist_remove(&long_str_list, s);
        free(s);
        current = next;
    }
    mn11236_clear_buffers();
}

void clear_gram(Color color)
{
    if (color == None)
        return;
    if (color > White)
        return;

    mn11236_clear_buffers();
}

LongStr_t *draw_long_str(char *str, Color color, int repeats)
{
#if SINGLE_LONGSTR_MODE
    LListNode *current = long_str_list.head;
    if (current != NULL)
    {
        clear_all_long_str();
    }
//		return NULL;
#endif

    int len = strlen(str);
    if (len > LONG_STR_LEN)
        len = LONG_STR_LEN;
    if (color == None)
        return NULL;
    LongStr_t *s = (LongStr_t *)malloc(sizeof(LongStr_t));
    if (s == NULL)
        return NULL;
    s->repeats = repeats;
    s->color = color;
    memset(s->str, 0, MAX_X);
    for (int i = 0; i < len; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            //			s->str[i * 6 + j + MAX_X] = ((hex_codes[str[i] - ' '][j] & 0x3) << 0) | ((hex_codes[str[i] - ' '][j] & 0x7E) << 1);
            s->str[i * 6 + j + MAX_X] = hex_codes[str[i] - ' '][j] << 1;
        }
        s->str[i * 6 + 5 + MAX_X] = 0;
    }
    s->length = len;
    s->current = s->str;

    llist_add(&long_str_list, s);
    return s;
}

void draw_point(int x, int y, Color color)
{
    if (x < MAX_X && y < MAX_Y && x >= 0 && y >= 0)
    {
        if (color)
				{
					if (color & 1)
							mn11236_draw_pixel(x, y, MN11236_COLOR_BLUE);
					if (color & 2)
							mn11236_draw_pixel(x, y, MN11236_COLOR_RED);
				}
        else
            mn11236_clear_pixel(x, y, MN11236_COLOR_RED);

        if (x + 1 < MAX_X)
        {
            if (color & 4)
                mn11236_draw_pixel(x + 1, y, MN11236_COLOR_BLUE);
            if (color & 8)
                mn11236_draw_pixel(x + 1, y, MN11236_COLOR_RED);
        }
    }
}

void refrash_gram()
{
    mn11236_send_buffer();
}

#define BAR_COUNT MAX_X
#define BAR_MAX MAX_Y
FFTDisplay_t bars[BAR_COUNT];

void update_display(FFTDisplay_t *fft, float target, float dt)
{
    const float Kp = 1.0f; // 比例增益
                           //	const float Kd = 0.001f; // 微分增益
                           //	target-=0.5f;
    float error = (fft->last_realval * 0.8f + target * 0.2f) - fft->output;

    float derivative = (error - fft->last_error) / dt;
    float delta = Kp * error /* + Kd * derivative*/;

    if (delta >= 1)
        fft->output += 1;
    else if (delta <= -1)
        fft->output -= 1;

    if (fft->output > BAR_MAX - 1)
        fft->output = BAR_MAX - 1;
    else if (fft->output < 0)
        fft->output = 0;
    fft->last_error = error;
    if (fft->maximum < fft->output)
    {
        fft->maximum = fft->output;
        fft->remain = 10;
    }
    else if (fft->remain)
        fft->remain--;
    else if (fft->maximum >= 1)
        fft->maximum--;
    fft->last_realval = target;
}

void draw_bar(FFTDisplay_t *fft, Color bar, Color dot)
{
    int j = 0;
    for (int i = 0; i < BAR_COUNT; i++)
    {
        for (j = 0; j < (bars[i].output); j++)
        {
            draw_point(i, MAX_Y - 1 - j, bar);
        }
        draw_point(i, MAX_Y - 1 - bars[i].maximum, dot);
        if (bar > White)
            i++;
    }
}

void draw_fft(long *fft_result, int len, int delta, Color bar, Color dot)
{
    static uint32_t last_time = 0;
    uint32_t current_time = HAL_GetTick();
    float dt = (current_time - last_time) / 1000.0f; // 转换为秒
    last_time = current_time;

    // 确保有足够的数据点（至少3个才能排除首尾）
    if (len <= 2)
        return;

    // 防止除零错误
    if (delta == 0)
        delta = 1;

    for (int i = 0; i < BAR_COUNT; i++)
    {
        // 映射到排除首尾的数据范围 [1, len-2]
        float mapped_pos = 1.0f + (float)i / BAR_COUNT * (len - 3);
        int index_low = (int)mapped_pos;
        int index_high = (index_low + 1 < len - 1) ? index_low + 1 : index_low;
        float fraction = mapped_pos - index_low;

        // 线性插值获取映射后的值
        long interpolated_value;
        if (index_low == index_high)
        {
            interpolated_value = fft_result[index_low];
        }
        else
        {
            interpolated_value = (long)((1.0f - fraction) * fft_result[index_low] +
                                        fraction * fft_result[index_high]);
        }

        // 应用delta除法
        long adjusted_value = interpolated_value / delta;

        // 更新并绘制
        update_display(&bars[i], adjusted_value, dt);
    }
    draw_bar(bars, bar, dot);
}
static uint8_t g_snakeWidth = 2;   // 默认宽度
#define COLS (MAX_X / g_snakeWidth)
#define ROWS (MAX_Y / g_snakeWidth)

#define MAX_LENGTH 60
#define SNAKE_HEAD White
#define SNAKE_BODY Cyan
#define FOOD_DOT Red
#define BLANK_DOT 0
// 定义蛇的结构体
typedef struct
{
    int16_t x;
    int16_t y;
} SnakeSegment;

// 定义蛇的结构体
typedef struct
{
    SnakeSegment body[MAX_LENGTH];
    int length;
    int direction; // 0: 上, 1: 右, 2: 下, 3: 左
} Snake_t;

// 定义食物的结构体
typedef struct
{
    uint16_t x;
    uint16_t y;
} Food_t;
typedef enum
{
    SNAKE_ALIVE = 0,
    SNAKE_DYING,      // 正在从头到尾变红
    SNAKE_DEAD_WAIT   // 已全红，等待重生
} SnakeState_t;

SnakeState_t snakeState = SNAKE_ALIVE;
int deathIndex = 0;
int deadWaitCnt = 0;

Snake_t snake;
Food_t food;

void initSnake(Snake_t *snake)
{
    int ok = 0;

    while (!ok)
    {
        ok = 1;

        snake->length = 4;
        snake->direction = rand() % 4;

        int hx = rand() % COLS;
        int hy = rand() % ROWS;

        /* 根据方向计算身体增长方向（反方向） */
        int dx = 0, dy = 0;
        switch (snake->direction)
        {
        case 0: dy = 1; break;  // 上 → 身体向下
        case 1: dx = -1; break; // 右 → 身体向左
        case 2: dy = -1; break; // 下 → 身体向上
        case 3: dx = 1; break;  // 左 → 身体向右
        }

        /* 检查是否越界 */
        for (int i = 0; i < snake->length; i++)
        {
            int x = hx + dx * i;
            int y = hy + dy * i;

            if (x < 0 || x >= COLS || y < 0 || y >= ROWS)
            {
                ok = 0;
                break;
            }

            snake->body[i].x = x;
            snake->body[i].y = y;
        }
    }
}

void initFood(Food_t *food, Snake_t *snake)
{
    int valid;
    do
    {
        valid = 1;
        food->x = rand() % COLS;
        food->y = rand() % ROWS;

        for (int i = 0; i < snake->length; i++)
        {
            if (snake->body[i].x == food->x && snake->body[i].y == food->y)
            {
                nexlink_log("x:%dy:%d %d, x:%dy:%d\r\n", food->x, food->y, i, snake->body[i].x, snake->body[i].y);
                valid = 0;
                break;
            }
        }
    } while (!valid);
}
// 打印游戏界面
static inline void draw_block3(int x, int y, uint8_t color)
{
    int px = x * g_snakeWidth;
    int py = y * g_snakeWidth;

    for (int dx = 0; dx < g_snakeWidth; dx++)
        for (int dy = 0; dy < g_snakeWidth; dy++)
            draw_point(px + dx, py + dy, (Color)color);
}


void printBoard(Snake_t *snake, Food_t *food)
{
    draw_block3(snake->body[0].x, snake->body[0].y, SNAKE_HEAD);

    for (int i = 1; i < snake->length; i++)
        draw_block3(snake->body[i].x, snake->body[i].y, SNAKE_BODY);

    draw_block3(food->x, food->y, FOOD_DOT);
} 

// 辅助函数：返回给定方向的相反方向
int oppositeDirection(int dir)
{
    switch (dir)
    {
    case 0:
        return 2;
    case 1:
        return 3;
    case 2:
        return 0;
    case 3:
        return 1;
    default:
        return -1;
    }
}

// 让蛇自动追踪食物，避免撞到自己
void autoTrackFood(Snake_t *snake, Food_t *food)
{
    int hx = snake->body[0].x;
    int hy = snake->body[0].y;
    int best = -1;

    static int lastDir = -1;

    int dx = food->x - hx;
    int dy = food->y - hy;

    int cand[4];
    int idx = 0;

    if (abs(dx) > abs(dy))
    {
        cand[idx++] = dx > 0 ? 1 : 3;
        cand[idx++] = dy > 0 ? 2 : 0;
        cand[idx++] = dy > 0 ? 0 : 2;
        cand[idx++] = dx > 0 ? 3 : 1;
    }
    else
    {
        cand[idx++] = dy > 0 ? 2 : 0;
        cand[idx++] = dx > 0 ? 1 : 3;
        cand[idx++] = dx > 0 ? 3 : 1;
        cand[idx++] = dy > 0 ? 0 : 2;
    }

    for (int i = 0; i < 4; i++)
    {
        int dir = cand[i];

        if (dir == oppositeDirection(snake->direction))
            continue;

        int nx = hx, ny = hy;
        if (dir == 0) ny--;
        if (dir == 1) nx++;
        if (dir == 2) ny++;
        if (dir == 3) nx--;

        /* ★ 边界检查 */
        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS)
            continue;

        int safe = 1;
        for (int j = 1; j < snake->length - 1; j++)
        {
            if (nx == snake->body[j].x &&
                ny == snake->body[j].y)
            {
                safe = 0;
                break;
            }
        }

        if (!safe)
            continue;

        if (dir == oppositeDirection(lastDir))
            continue;

        best = dir;
        break;
    }

    if (best != -1)
    {
        snake->direction = best;
        lastDir = best;
    }
}

void moveSnake(Snake_t *snake)
{
    for (int i = snake->length - 1; i > 0; i--)
    {
        snake->body[i] = snake->body[i - 1];
    }

    switch (snake->direction)
    {
    case 0: snake->body[0].y--; break;
    case 1: snake->body[0].x++; break;
    case 2: snake->body[0].y++; break;
    case 3: snake->body[0].x--; break;
    }
    /* ❌ 不允许取模 */
}

// 检查蛇是否吃到食物
int checkEat(Snake_t *snake, Food_t *food)
{
    if (snake->body[0].x == food->x && snake->body[0].y == food->y)
    {
        snake->length++;
        if (snake->length > MAX_LENGTH - 1)
            snake->length = MAX_LENGTH - 1;
        snake->body[snake->length - 1].x = snake->body[snake->length - 2].x;
        snake->body[snake->length - 1].y = snake->body[snake->length - 2].y;
        return 1;
    }
    return 0;
}

int checkCollision(Snake_t *snake)
{
    int x = snake->body[0].x;
    int y = snake->body[0].y;

    /* 撞墙 */
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS)
        return 1;

    /* 撞身体（不查最后一节） */
    for (int i = 1; i < snake->length - 1; i++)
    {
        if (x == snake->body[i].x &&
            y == snake->body[i].y)
            return 1;
    }
    return 0;
}

void drawSnakeDying(Snake_t *snake)
{
    for (int i = 0; i < snake->length; i++)
    {
        if (i <= deathIndex)
        {
            draw_block3(
                snake->body[i].x,
                snake->body[i].y,
                Red     // 已“死亡”的部分
            );
        }
        else
        {
            draw_block3(
                snake->body[i].x,
                snake->body[i].y,
                SNAKE_BODY
            );
        }
    }
}

void setSnakeWidth(uint8_t width)
{
    if (width < 1)
        width = 1;
    if (width > 8)          // 可按屏幕性能限制
        width = 8;

    g_snakeWidth = width;

    /* ⚠️ 切换宽度后，强烈建议重开一局 */
    snakeState = SNAKE_ALIVE;
    deathIndex = 0;
    deadWaitCnt = 0;

    initSnake(&snake);
    initFood(&food, &snake);
}

void beginSnake()
{
    initSnake(&snake);
}

void refrashSnake()
{
    /* ====== 1. 正在死亡动画（逐节变红） ====== */
    if (snakeState == SNAKE_DYING)
    {
        drawSnakeDying(&snake);

        deathIndex++;

        if (deathIndex >= snake.length - 1)
        {
            /* 已经全红，进入等待阶段 */
            snakeState = SNAKE_DEAD_WAIT;
            deadWaitCnt = 0;
        }
        return;
    }

    /* ====== 2. 全红等待阶段 ====== */
    if (snakeState == SNAKE_DEAD_WAIT)
    {
        /* 整条蛇保持红色 */
        for (int i = 0; i < snake.length; i++)
        {
            draw_block3(
                snake.body[i].x,
                snake.body[i].y,
                Red
            );
        }

        deadWaitCnt++;

        if (deadWaitCnt >= 10)   // ★ 等待 10 个周期
        {
            deathIndex = 0;
            deadWaitCnt = 0;
            snakeState = SNAKE_ALIVE;

            initSnake(&snake);
            initFood(&food, &snake);
        }
        return;
    }

    /* ====== 3. 正常游戏阶段 ====== */
    printBoard(&snake, &food);

    autoTrackFood(&snake, &food);
    moveSnake(&snake);

    if (checkEat(&snake, &food))
    {
        initFood(&food, &snake);
    }

    if (checkCollision(&snake))
    {
        nexlink_log("Game Over!\n");
        snakeState = SNAKE_DYING;
        deathIndex = 0;
    }
}
