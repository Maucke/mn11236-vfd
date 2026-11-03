#include "clockui.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "llist.h"
#include "nexlink_app.h"
#include <math.h>
#include "cmsis_os.h"

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

const uint8_t icon_rpm[] = 
{
	0x0F,0x05,0x0D,0x07,0x00,0x0F,0x05,0x05,0x07,0x00,0x0F,0x01,0x07,0x01,0x0F
};
const uint8_t icon_kmh[] = 
{
	0x0F,0x02,0x07,0x09,0x00,0x0F,0x01,0x07,0x01,0x0F,0x00,0x08,0x06,0x01,0x00,0x0F,
	0x02,0x02,0x0F
};
const uint8_t icon_temp[] = 
{
	0x01,0x00,0x07,0x05,0x01
};
const uint8_t icon_load[] = 
{
	0x03,0x0B,0x04,0x02,0x0D,0x0C
};
const uint8_t icon_volt[] = 
{
	0x03,0x04,0x08,0x04,0x03
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
    {0x07, 0x04, 0x1E}, /*"4", 52(ASCII)*/
    {0x17, 0x15, 0x1D}, /*"5", 53(ASCII)*/
    {0x1F, 0x15, 0x1C}, /*"6", 54(ASCII)*/
    {0x01, 0x1D, 0x03}, /*"7", 55(ASCII)*/
    {0x1F, 0x15, 0x1F}, /*"8", 56(ASCII)*/
    {0x07, 0x15, 0x1F}, /*"9", 57(ASCII)*/
    {0x00, 0x0A, 0x00}, /*":", 58(ASCII)*/
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

#define W1 0.06f
#define W2 0.04f
#define W3 0.03f

#define SPEED1 0.05f
#define SPEED2 0.03f
#define SPEED3 0.05f

/*
 * 相位使用 16 位定点累加（1 单位 = 2π/65536），溢出即完成 2π 回绕。
 * 之前用 float 一直累加：t 增大到 2^20 左右后 ulp(0.125) 大于步进(0.05)，
 * t += SPEED 不再改变数值，波形就永久停在某一帧（约 29 小时，5ms 一次）。
 */
#define PHASE_UNIT (6.28318530718f / 65536.0f)
#define PHASE_STEP(rad) ((uint16_t)((rad) / PHASE_UNIT + 0.5f))

static uint16_t phase1 = 0;
static uint16_t phase2 = 0;
static uint16_t phase3 = 0;

uint8_t frame_wave[MAX_X][MAX_Y]; // 6-4:red, 2-0:blue

/* 擦除染色主题：帧缓冲里的通道值直接就是屏幕灰度，dim 给 2（ch*0.5*2 = ch） */
#define BG_WIPE_DIM_BRIGHT 2.0f
/* 拖尾最亮处（光条旁边）的灰度，7 太亮 */
#define BG_WIPE_TRAIL_MAX 5.0f

/* ---------------- 可暂停的动画时间基准 ----------------
 * 碎裂 / 拼合过渡是阻塞的，期间后台动画（背景、频谱等）不应该偷偷跑：
 * 动画统一用 clock_anim_tick() 取时间，过渡结束后把耗时累加到偏移里，
 * 相当于把动画时间按住了，恢复后从原来的相位继续。
 */
static uint32_t g_anim_pause_off = 0;

uint32_t clock_anim_tick(void)
{
    return HAL_GetTick() - g_anim_pause_off;
}

void clock_anim_pause(uint32_t ms)
{
    g_anim_pause_off += ms;
}

/* 背景通道值（0~7）-> 屏幕灰度（半亮）
   dim 为本像素的亮度系数：普通背景 1.0，擦除染色主题 2.0（通道值即灰度） */
static inline uint8_t bg_map_gray(uint8_t ch, int x, int y, float dim)
{
    (void)x;
    (void)y;

    /* 暗部不托底：不足 1 级的通道直接熄灭，背景才有真正的黑 */
    float v = (float)ch * 0.5f * dim;

    int gray = (int)v;

    if (gray > 7)
        gray = 7;

    return (uint8_t)gray;
}

static void flush_frame(void)
{
    for (int y = 0; y < MAX_Y; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            uint8_t p = frame_wave[x][y];
            float dim;

            /* bit7 = 光柱高光；bit3 = 擦除染色主题：通道值就是最终灰度；
               其它 = 普通背景：固定满亮度（不再随时间淡出） */
            if (p & 0x80)
                dim = 1.0f;
            else if (p & 0x08)
                dim = BG_WIPE_DIM_BRIGHT;
            else
                dim = 1.0f;

            mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, bg_map_gray((p >> 4) & 0x07, x, y, dim));
            mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, bg_map_gray(p & 0x07, x, y, dim));
        }
    }
}

/* 三波叠加的对比度增益：1.0 = |v-0.5|*2 原始曲线，调大更亮 */
#define BG_WAVE_GAIN 1.0f

void gradient_flow_frame_clock(void)
{
    static float sin_x[MAX_X];
    static float sin_y[MAX_Y];
    static float sin_diag[MAX_X + MAX_Y];

    float a1 = phase1 * PHASE_UNIT;
    float a2 = phase2 * PHASE_UNIT;
    float a3 = phase3 * PHASE_UNIT;

    /* 三个 sin 项按行/列/对角线分别计算，避免每个像素都调用一次 sinf */
    for (int x = 0; x < MAX_X; x++)
        sin_x[x] = sinf(x * W1 + a1);
    for (int y = 0; y < MAX_Y; y++)
        sin_y[y] = sinf(y * W2 + a2);
    for (int d = 0; d < MAX_X + MAX_Y; d++)
        sin_diag[d] = sinf(d * W3 + a3);

    for (int y = 0; y < MAX_Y; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            // ===== 多波叠加 =====
            float v = sin_x[x] + sin_y[y] + sin_diag[x + y];
            // 归一化到 0~1
            v = (v + 3.0f) / 6.0f;

            if (v < 0)
                v = 0;
            if (v > 1)
                v = 1;

            /* 映射：以 0.5 为中性，越偏离越亮；v≈0.5 的“零线”直接熄灭。
             * 偏离量 m ∈ [0,1]，大于 0.5 走蓝、小于 0.5 走红，
             * 于是画面是红波带 / 黑 / 蓝波带交替，暗部有真正的黑。
             * 觉得整体太暗就把 BG_WAVE_GAIN 调大（1.0 = 原始曲线）。 */
            float m = v - 0.5f;

            if (m < 0.0f)
                m = -m;
            m *= 2.0f * BG_WAVE_GAIN;

            if (m > 1.0f)
                m = 1.0f;

            uint8_t blue = (v > 0.5f) ? (uint8_t)(m * 7.0f) : 0; // 3bit
            uint8_t red = (v < 0.5f) ? (uint8_t)(m * 7.0f) : 0;  // 3bit

            frame_wave[x][y] = (red << 4) | (blue & 0x07);
        }
    }

    flush_frame();
    phase1 += PHASE_STEP(SPEED1);
    phase2 += PHASE_STEP(SPEED2);
    phase3 += PHASE_STEP(SPEED3);
}

/* ---------------- 背景动画组 ---------------- */

/* 把 0~1 的亮度写进波形缓冲：v 越大越蓝，越小越红 */
static inline void bg_put(int x, int y, float v)
{
    if (v < 0.0f)
        v = 0.0f;
    if (v > 1.0f)
        v = 1.0f;

    uint8_t blue = (uint8_t)(v * 7.0f);
    uint8_t red = (uint8_t)((1.0f - v) * 7.0f);

    frame_wave[x][y] = (red << 4) | (blue & 0x07);
}

/* 横向光带：上下流动的条纹，带一点倾斜 */
static void bg_frame_bands(void)
{
    static uint16_t ph = 0;
    static float row[MAX_Y];
    static float col[MAX_X];

    float t = ph * PHASE_UNIT;

    for (int y = 0; y < MAX_Y; y++)
        row[y] = sinf(y * 0.42f + t);
    for (int x = 0; x < MAX_X; x++)
        col[x] = sinf(x * 0.09f - 2.0f * t);

    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            bg_put(x, y, 0.5f + 0.4f * row[y] + 0.12f * col[x]);

    ph += PHASE_STEP(0.06f);
}

/* 中心光晕：柔和的径向光斑缓慢漂移、轻微呼吸（不再是高速扩散的同心圆，避免眩晕） */
#define GLOW_TABLE_SIZE 128
static void bg_frame_glow(void)
{
    static uint16_t ph = 0;
    static float glow[GLOW_TABLE_SIZE];
    static uint8_t inited = 0;

    if (!inited)
    {
        for (int i = 0; i < GLOW_TABLE_SIZE; i++)
        {
            float r2 = (float)(i * 32);
            glow[i] = 1.0f / (1.0f + r2 * 0.0035f);
        }
        inited = 1;
    }

    float t = ph * PHASE_UNIT;
    float cx = (MAX_X - 1) * 0.5f + 18.0f * sinf(t);
    float cy = (MAX_Y - 1) * 0.5f;
    float breath = 0.85f + 0.15f * sinf(t);

    for (int y = 0; y < MAX_Y; y++)
    {
        float dy = (y - cy) * 2.2f; /* 纵向拉伸，补偿像素长宽比 */
        for (int x = 0; x < MAX_X; x++)
        {
            float dx = x - cx;
            int idx = (int)((dx * dx + dy * dy) * (1.0f / 32.0f));

            if (idx >= GLOW_TABLE_SIZE)
                idx = GLOW_TABLE_SIZE - 1;

            bg_put(x, y, 0.30f + 0.65f * glow[idx] * breath);
        }
    }

    ph += PHASE_STEP(0.02f);
}

/* 直接写 red/blue 两个通道（范围 0~7） */
static inline void bg_put_rgb(int x, int y, float red, float blue)
{
    if (red < 0.0f)
        red = 0.0f;
    if (red > 7.0f)
        red = 7.0f;
    if (blue < 0.0f)
        blue = 0.0f;
    if (blue > 7.0f)
        blue = 7.0f;

    frame_wave[x][y] = ((uint8_t)red << 4) | ((uint8_t)blue & 0x07);
}

/* 斜向光条参数（擦除染色主题共用）：
 * 光条与水平方向成 30°，从右向左单向扫描；扫一次 1s，然后空闲 1s */
#define BEAM_SLOPE 0.5773503f /* tan(30°) */
#define BEAM_SWEEP_MS 1000    /* 扫描耗时 1s */
#define BEAM_PERIOD_MS 2000   /* 周期 2s（扫描 1s + 空闲 1s） */

/* 擦除染色：一轮蓝、一轮红交替
 * 每个周期 2s：
 *   后 1s —— 白色光条从右向左扫过，身后留下该颜色的拖尾：
 *            离光条越远越暗，一直暗到全黑；光条前方全黑；
 *   前 1s —— 上一轮留下的拖尾整体继续变暗消失。
 */
static void bg_frame_wipe(void)
{
    const float s_max = (MAX_X - 1) + BEAM_SLOPE * (MAX_Y - 1);
    const float d_scale = 1.0f / sqrtf(1.0f + BEAM_SLOPE * BEAM_SLOPE);
    const float trail_len = s_max; /* 拖尾长度 ≈ 屏幕斜向跨度 */

    uint32_t tick = clock_anim_tick();
    uint32_t now_ms = tick % BEAM_PERIOD_MS;
    /* 偶数轮蓝色、奇数轮红色 */
    uint8_t is_blue = ((tick / BEAM_PERIOD_MS) & 1u) == 0u;

    float pos;  /* 光条位置（斜向坐标） */
    float fade; /* 整条拖尾的整体亮度 */
    float bar;  /* 光条亮度：1 = 在屏上，0 = 已扫出屏幕 */
    float base; /* 拖尾颜色：1 = 蓝，0 = 红 */

    if (now_ms < BEAM_SWEEP_MS)
    {
        /* ---------- 前 1s：上一轮扫完后留下的拖尾继续变暗消失 ---------- */
        float p = (float)now_ms / (float)BEAM_SWEEP_MS;

        pos = -20.0f; /* 上一轮结束时光条停在左侧屏外 */
        fade = 1.0f - p;
        bar = 0.0f;
        base = is_blue ? 1.0f : 0.0f; /* 上一轮扫进来的是本轮的颜色 */
    }
    else
    {
        /* ---------- 后 1s：白色光条从右向左扫过，身后留下渐暗的拖尾 ---------- */
        float u = (float)(now_ms - BEAM_SWEEP_MS) / (float)BEAM_SWEEP_MS;
        float e = 1.0f - (1.0f - u) * (1.0f - u); /* 与光柱相同的缓出曲线 */

        pos = (s_max + 20.0f) - e * (s_max + 40.0f);
        fade = 1.0f;
        bar = 1.0f;
        base = is_blue ? 0.0f : 1.0f; /* 本轮光条带来的是另一种颜色 */
    }

    for (int y = 0; y < MAX_Y; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            float s = x + BEAM_SLOPE * y;

            /* 拖尾：光条后方越远越暗，前方全黑 */
            float trail = (s < pos) ? 0.0f : 1.0f - (s - pos) / trail_len;

            if (trail < 0.0f)
                trail = 0.0f;

            float val = BG_WIPE_TRAIL_MAX * trail * fade;

            float red = (1.0f - base) * val;
            float blue = base * val;

            /* 光条：向白色抬升，形成明亮的扫过边沿 */
            float d = (s - pos) * d_scale;
            float g = bar / (1.0f + d * d * 0.022f);

            red += (BG_WIPE_TRAIL_MAX - red) * g;
            blue += (BG_WIPE_TRAIL_MAX - blue) * g;

            /* 通道值即最终灰度（bit3 让 flush 不做衰减），+0.5 取整 */
            bg_put_rgb(x, y, red + 0.5f, blue + 0.5f);
            frame_wave[x][y] |= 0x08;
        }
    }
}

/* 流动渐变：明暗过渡从一侧缓慢流向另一侧（不对称） */
static void bg_frame_flow(void)
{
    static uint16_t ph = 0;
    static float col[MAX_X];
    static float row[MAX_Y];

    float t = ph * PHASE_UNIT;

    for (int x = 0; x < MAX_X; x++)
        col[x] = sinf(x * 0.055f + t); /* 波长约两屏宽，横向缓慢流动 */
    for (int y = 0; y < MAX_Y; y++)
        row[y] = 0.15f * sinf(y * 0.26f - t);

    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            bg_put(x, y, 0.5f + 0.42f * col[x] + row[y]);

    ph += PHASE_STEP(0.02f);
}

/* 等离子：两组正弦相乘形成的流动斑块 */
static void bg_frame_plasma(void)
{
    static uint16_t ph = 0;
    static float col[MAX_X];
    static float row[MAX_Y];

    float t = ph * PHASE_UNIT;

    for (int x = 0; x < MAX_X; x++)
        col[x] = sinf(x * 0.09f + t);
    for (int y = 0; y < MAX_Y; y++)
        row[y] = sinf(y * 0.34f - t);

    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            bg_put(x, y, 0.5f + 0.5f * col[x] * row[y]);

    ph += PHASE_STEP(0.07f);
}

/* 呼吸：整屏明暗缓慢起伏 */
static void bg_frame_breathe(void)
{
    static uint16_t ph = 0;
    static float row[MAX_Y];

    float t = ph * PHASE_UNIT;
    float base = 0.5f + 0.35f * sinf(t);

    for (int y = 0; y < MAX_Y; y++)
        row[y] = base + 0.12f * sinf(y * 0.22f + t);

    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            bg_put(x, y, row[y]);

    ph += PHASE_STEP(0.03f);
}

/* 按主题选择背景动画并刷到显存 */
void clock_draw_background(uint8_t bg)
{
    /* 背景一律满亮度，没有任何随时间淡出的逻辑 */
    switch (bg)
    {
    case CLOCK_BG_WAVE:
        gradient_flow_frame_clock(); /* 内部已经 flush */
        return;

    case CLOCK_BG_BANDS:
        bg_frame_bands();
        break;

    case CLOCK_BG_GLOW:
        bg_frame_glow();
        break;

    case CLOCK_BG_WIPE:
        bg_frame_wipe();
        break;

    case CLOCK_BG_FLOW:
        bg_frame_flow();
        break;

    case CLOCK_BG_PLASMA:
        bg_frame_plasma();
        break;

    case CLOCK_BG_BREATHE:
        bg_frame_breathe();
        break;

    default:
        return; /* CLOCK_BG_NONE：保留纯黑背景 */
    }

    flush_frame();
}

void clock_init()
{
    llist_init(&long_str_list);
    clear_all_long_str();
}
static uint8_t g_maxGray = 7; //(3-7)

void set_g_maxGray(uint8_t gray)
{
	g_maxGray = gray;
}

void draw_point_wave(int x, int y, Color color)
{
    if (x < MAX_X && y < MAX_Y && x >= 0 && y >= 0)
    {
        if (color)
        {
            uint8_t p = frame_wave[x][y];

            uint8_t red = (p >> 4) & 0x07;
            uint8_t blue = p & 0x07;

            if (color == Red)
            {
                if (red > blue)
								{
										mn11236_clear_pixel(x, y, MN11236_COLOR_BLUE);
                    mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, g_maxGray);
								}
                else
								{
										mn11236_clear_pixel(x, y, MN11236_COLOR_RED);
                    mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, g_maxGray);
								}
            }
            else if (color == Cyan)
            {
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, red);
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, blue);
            }
            else
            {
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, g_maxGray);
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, g_maxGray);
            }
        }
        else
        {
            mn11236_clear_pixel(x, y, MN11236_COLOR_RED);
            mn11236_clear_pixel(x, y, MN11236_COLOR_BLUE);
        }
    }
}
static uint8_t g_clockWidth = 3;

void set_g_clockWidth(uint8_t width)
{
	g_clockWidth = width;
}

/* ---------------- 字体：标准 3x5 放大 / 原生 9x15 ---------------- */

/*
 *   0 = 标准：3x5 字模按 g_clockWidth 放大（原来的效果）
 *   1 = 原生：按最终像素尺寸直接设计的 9x15 字模，笔画 2 像素、带斜线和圆角，
 *             没有小字模放大后的方块感。
 * 原生字模的 9x15 正好等于 3x5 三倍放大后的尺寸，所以布局、字距都不用改。
 * 只覆盖时间和日期用到的字符，字母等其它字符以及小字号（日期用
 * g_clockWidth = 1 绘制）自动回落到标准 3x5 字模。
 */
#define CLOCK_FONT_STD 0
#define CLOCK_FONT_NATIVE 1

static uint8_t g_fontStyle = CLOCK_FONT_STD;

void clock_set_font_style(uint8_t style)
{
    g_fontStyle = (style != 0) ? CLOCK_FONT_NATIVE : CLOCK_FONT_STD;
}

/* 原生字模尺寸（= 3x5 字模 3 倍放大后的 9x15 像素） */
#define NATIVE_GLYPH_W 9
#define NATIVE_GLYPH_H 15

/* 顺序必须和下面的字模表逐行对应 */
static const char font_native_chars[] = " 0123456789:-/";

/* 原生尺寸字模：9x15 像素/字，每列一个 16 位掩码（bit0 = 最上一行） */
static const uint16_t font_native_9x15[][NATIVE_GLYPH_W] = {
    {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, /* ' ' */
    {0x1FFC, 0x3FFE, 0x6003, 0x4001, 0x4001, 0x4001, 0x6003, 0x3FFE, 0x1FFC}, /* '0' */
    {0x0000, 0x0000, 0x4004, 0x4006, 0x7FFF, 0x7FFF, 0x4000, 0x4000, 0x0000}, /* '1' */
    {0x7804, 0x7C06, 0x4603, 0x4301, 0x4181, 0x40C1, 0x4063, 0x403E, 0x401C}, /* '2' */
    {0x1004, 0x3006, 0x6003, 0x4001, 0x40C1, 0x40C1, 0x60C3, 0x3FFE, 0x1FFC}, /* '3' */
    {0x03E0, 0x03F0, 0x0318, 0x030C, 0x0306, 0x7FFF, 0x7FFF, 0x0300, 0x0300}, /* '4' */
    {0x18FF, 0x38FF, 0x6041, 0x4021, 0x4021, 0x4021, 0x6061, 0x3FC1, 0x1F81}, /* '5' */
    {0x1FFC, 0x3FFE, 0x6083, 0x4041, 0x4041, 0x4041, 0x60C3, 0x3F86, 0x1F04}, /* '6' */
    {0x0003, 0x7C03, 0x7F03, 0x03C3, 0x00F3, 0x003B, 0x000F, 0x0007, 0x0003}, /* '7' */
    {0x1F1C, 0x3FBE, 0x60E3, 0x4041, 0x4041, 0x4041, 0x60E3, 0x3FBE, 0x1F1C}, /* '8' */
    {0x00FC, 0x01FE, 0x4303, 0x4301, 0x4201, 0x6201, 0x6303, 0x7FFE, 0x3DFC}, /* '9' */
    {0x0000, 0x0000, 0x0000, 0x1C1C, 0x1C1C, 0x0000, 0x0000, 0x0000, 0x0000}, /* ':' */
    {0x0000, 0x0000, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x0000, 0x0000}, /* '-' */
    {0x6000, 0x7800, 0x1E00, 0x0780, 0x01E0, 0x0078, 0x001E, 0x0007, 0x0001}, /* '/' */
};

/* 取原生字模，没有定义的字符返回 NULL */
static inline const uint16_t *native_glyph(char ch)
{
    const char *p;

    if (ch == '\0')
        return NULL;

    p = strchr(font_native_chars, ch);

    if (p == NULL)
        return NULL;

    return font_native_9x15[p - font_native_chars];
}

/* 用原生字模绘制需要满足的条件：大字号 + 字模表里有这个字符 */
static inline uint8_t use_native_font(char ch)
{
    return (g_fontStyle == CLOCK_FONT_NATIVE && g_clockWidth == 3) ? (native_glyph(ch) != NULL) : 0;
}

/* 取标准 3x5 字模 */
static inline const uint8_t *clock_glyph(char ch)
{
    /* 越界字符按空格处理，避免读到表外 */
    if ((uint8_t)ch < ' ' || (uint8_t)ch >= (uint8_t)(' ' + CHAR_COUNT))
        return hex_codes_3x5[0];

    return hex_codes_3x5[(uint8_t)ch - ' '];
}

/* 普通绘制：直接按像素画原生字模（px/py 已经是屏幕像素坐标） */
static void native_draw_plain(const uint16_t *g, int px, int py, Color color)
{
    if (color == None)
        return;

    for (int c = 0; c < NATIVE_GLYPH_W; c++)
    {
        uint16_t bits = g[c];

        if (bits == 0)
            continue;

        for (int r = 0; r < NATIVE_GLYPH_H; r++)
            if (bits & (1u << r))
                draw_point(px + c, py + r, color);
    }
}

/* 波形背景下的原生字模绘制 */
static void native_draw_wave(const uint16_t *g, int px, int py, Color color)
{
    for (int c = 0; c < NATIVE_GLYPH_W; c++)
    {
        uint16_t bits = g[c];

        if (bits == 0)
            continue;

        for (int r = 0; r < NATIVE_GLYPH_H; r++)
            if (bits & (1u << r))
                draw_point_wave(px + c, py + r, color);
    }
}

/* 某一行最右边的笔画在哪一列，没有笔画返回 -1 */
static inline int native_right_edge(const uint16_t *g, int r)
{
    for (int c = NATIVE_GLYPH_W - 1; c >= 0; c--)
        if (g[c] & (1u << r))
            return c;

    return -1;
}

/* 原生字模的右侧阴影：只沿字形右外沿补 2 个像素，
 * 这样数字的中间镂空处不会出现多余的阴影线 */
static void native_draw_shadow_plain(const uint16_t *g, int px, int py, Color color, uint8_t gray)
{
    uint8_t old = g_maxGray;

    if (color == None)
        return;

    g_maxGray = gray;

    for (int r = 0; r < NATIVE_GLYPH_H; r++)
    {
        int c = native_right_edge(g, r);

        if (c < 0)
            continue;

        draw_point(px + c + 1, py + r, color);
        draw_point(px + c + 2, py + r, color);
    }

    g_maxGray = old;
}

/* 波形背景下的右侧阴影（黑色，压在字符右边） */
static void native_draw_shadow_wave(const uint16_t *g, int px, int py, Color color)
{
    for (int r = 0; r < NATIVE_GLYPH_H; r++)
    {
        int c = native_right_edge(g, r);

        if (c >= 0)
            draw_point_wave(px + c + 1, py + r, color);
    }
}

static inline void draw_clock_block_wave(int x, int y, uint8_t color)
{
    int px = x * g_clockWidth;
    int py = y * g_clockWidth;

    for (int dx = 0; dx < g_clockWidth; dx++)
        for (int dy = 0; dy < g_clockWidth; dy++)
            draw_point_wave(px + dx, py + dy, (Color)color);
}

static void draw_bits_wave(uint8_t bit_mask, int x, int y, Color color)
{
    for (int bit = 0; bit < 8; bit++)
    {
        if (bit_mask & (1 << bit))
        {
            draw_clock_block_wave(x, bit + y, color);
        }
    }
}

static inline void draw_clock_block(int x, int y, uint8_t color)
{
    int px = x * g_clockWidth;
    int py = y * g_clockWidth;

    for (int dx = 0; dx < g_clockWidth; dx++)
        for (int dy = 0; dy < g_clockWidth; dy++)
            draw_point(px + dx, py + dy, (Color)color);
}

static void draw_bits(uint8_t bit_mask, int x, int y, Color color)
{
    for (int bit = 0; bit < 8; bit++)
    {
        if (bit_mask & (1 << bit))
        {
            draw_clock_block(x, bit + y, color);
        }
    }
}

/* 纯字主题：文字右侧阴影的固定亮度 */
#define CLOCK_SHADOW_GRAY 2

static void draw_bits_gray(uint8_t bit_mask, int x, int y, Color color, uint8_t gray)
{
    uint8_t old = g_maxGray;

    g_maxGray = gray;
    draw_bits(bit_mask, x, y, color);
    g_maxGray = old;
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
            if (bit_mask == 0)
                continue; // 无点可绘，直接跳过

            // 颜色掩码1：青，原x坐标 → 调用封装函数
            if (s->color & 1)
                draw_bits(bit_mask, x, 0, Cyan);
            // 颜色掩码2：红，原x坐标 → 调用封装函数
            if (s->color & 2)
                draw_bits(bit_mask, x, 0, Red);

            // x+1不越界时，处理偏移坐标
            if (x + 1 < MAX_X)
            {
                // 颜色掩码4：青，x+1坐标 → 调用封装函数
                if (s->color & 4)
                    draw_bits(bit_mask, x + 1, 0, Cyan);
                // 颜色掩码8：红，x+1坐标 → 调用封装函数
                if (s->color & 8)
                    draw_bits(bit_mask, x + 1, 0, Red);
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

void draw_char(int x, int y, char ch, Color color)
{
    if (color == None)
        return;
    for (int i = 4; i >= 0; i--)
    {
        if ((i + x) < MAX_X && ((i + x) >= 0))
        {
            uint8_t bit_mask = hex_codes[ch - ' '][i];
            if (bit_mask == 0)
                continue; // 无点可绘，直接跳过
            // 颜色掩码1：青，原x坐标 → 调用封装函数
            if (color & 1)
                draw_bits(bit_mask, i + x, y, Cyan);
            // 颜色掩码2：红，原x坐标 → 调用封装函数
            if (color & 2)
                draw_bits(bit_mask, i + x, y, Red);

            // x+1不越界时，处理偏移坐标
            if (x + 1 < MAX_X)
            {
                // 颜色掩码4：青，x+1坐标 → 调用封装函数
                if (color & 4)
                    draw_bits(bit_mask, i + x + 1, y, Cyan);
                // 颜色掩码8：红，x+1坐标 → 调用封装函数
                if (color & 8)
                    draw_bits(bit_mask, i + x + 1, y, Red);
            }
        }
    }
}

void draw_char_3x5_wave(int x, int y, char ch, Color color)
{
    if (color == None)
        return;

    /* 原生字号：右外沿 1 像素阴影（黑）+ 字符 */
    if (use_native_font(ch))
    {
        const uint16_t *g = native_glyph(ch);
        int px = x * g_clockWidth;
        int py = (y + 1) * g_clockWidth;

        native_draw_shadow_wave(g, px, py, None);
        native_draw_wave(g, px, py, color);
        return;
    }
#if 0
    for (int i = 2; i >= 0; i--)
#else
    for (int i = 0; i < 3; i++)
#endif
    {
        if ((i + x) < MAX_X && ((i + x) >= 0))
        {
            uint8_t bit_mask = clock_glyph(ch)[i];
            if (bit_mask == 0)
                continue;
            draw_bits_wave(bit_mask, i + x, y + 1, color);
						
            // x+1不越界时，处理偏移坐标
            if (x + 1 < MAX_X)
            {
							draw_bits_wave(bit_mask, i + x + 1, y + 1, None);
            }
        }
    }
}

/* 在已绘制好的背景上淡入淡出地绘制一个点：
 * level = 0 时保持背景原样，level = 7 时是正常字符亮度，
 * 这样字符熄灭时露出的是背景（渐变或纯黑），而不会留下黑色空洞。 */
static void draw_point_wave_fade(int x, int y, Color color, uint8_t level)
{
    if (x >= MAX_X || y >= MAX_Y || x < 0 || y < 0)
        return;
    if (level > 7)
        level = 7;

    /* 背景取当前缓冲区里已经画好的颜色 */
    int bg_red = mn11236_read_pixel(x, y, MN11236_COLOR_RED);
    int bg_blue = mn11236_read_pixel(x, y, MN11236_COLOR_BLUE);

    /* 字符满亮度时的取色规则与 draw_point_wave 保持一致 */
    int fg_red, fg_blue;
    if (color == None)
    {
        /* 阴影：和原来一样是黑色，但会随字符一起淡入淡出 */
        fg_red = 0;
        fg_blue = 0;
    }
    else if (color == Red)
    {
        uint8_t p = frame_wave[x][y];
        uint8_t red = (p >> 4) & 0x07;
        uint8_t blue = p & 0x07;

        if (red > blue)
        {
            fg_red = g_maxGray;
            fg_blue = 0;
        }
        else
        {
            fg_red = 0;
            fg_blue = g_maxGray;
        }
    }
    else if (color == Cyan)
    {
        uint8_t p = frame_wave[x][y];

        fg_red = (p >> 4) & 0x07;
        fg_blue = p & 0x07;
    }
    else
    {
        fg_red = g_maxGray;
        fg_blue = g_maxGray;
    }

    mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED,
                            (uint8_t)(bg_red + (fg_red - bg_red) * level / 7));
    mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE,
                            (uint8_t)(bg_blue + (fg_blue - bg_blue) * level / 7));
}

static inline void draw_clock_block_wave_fade(int x, int y, Color color, uint8_t level)
{
    int px = x * g_clockWidth;
    int py = y * g_clockWidth;

    for (int dx = 0; dx < g_clockWidth; dx++)
        for (int dy = 0; dy < g_clockWidth; dy++)
            draw_point_wave_fade(px + dx, py + dy, color, level);
}

static void draw_bits_wave_fade(uint8_t bit_mask, int x, int y, Color color, uint8_t level)
{
    for (int bit = 0; bit < 8; bit++)
    {
        if (bit_mask & (1 << bit))
            draw_clock_block_wave_fade(x, bit + y, color, level);
    }
}

/* 波形背景下淡入淡出的原生字模绘制 */
static void native_draw_wave_fade(const uint16_t *g, int px, int py, Color color, uint8_t level)
{
    for (int c = 0; c < NATIVE_GLYPH_W; c++)
    {
        uint16_t bits = g[c];

        if (bits == 0)
            continue;

        for (int r = 0; r < NATIVE_GLYPH_H; r++)
            if (bits & (1u << r))
                draw_point_wave_fade(px + c, py + r, color, level);
    }
}

/* 波形背景下淡入淡出的右侧阴影 */
static void native_draw_shadow_wave_fade(const uint16_t *g, int px, int py, Color color, uint8_t level)
{
    for (int r = 0; r < NATIVE_GLYPH_H; r++)
    {
        int c = native_right_edge(g, r);

        if (c >= 0)
            draw_point_wave_fade(px + c + 1, py + r, color, level);
    }
}

/* 波形背景下的字符淡入淡出，保留字符右侧 1 像素的阴影 */
void draw_char_3x5_wave_fade(int x, int y, char ch, Color color, uint8_t level)
{
    if (color == None)
        return;

    /* 原生字号：阴影（黑）和字符一起淡入淡出 */
    if (use_native_font(ch))
    {
        const uint16_t *g = native_glyph(ch);
        int px = x * g_clockWidth;
        int py = (y + 1) * g_clockWidth;

        native_draw_shadow_wave_fade(g, px, py, None, level);
        native_draw_wave_fade(g, px, py, color, level);
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        if ((i + x) < MAX_X && ((i + x) >= 0))
        {
            uint8_t bit_mask = clock_glyph(ch)[i];
            if (bit_mask == 0)
                continue;
            draw_bits_wave_fade(bit_mask, i + x, y + 1, color, level);

            // x+1不越界时，绘制右侧阴影（None = 黑色，随亮度一起淡出）
            if (i + x + 1 < MAX_X)
                draw_bits_wave_fade(bit_mask, i + x + 1, y + 1, None, level);
        }
    }
}

void draw_char_3x5(int x, int y, char ch, Color color)
{
    if (color == None)
        return;

    /* 原生字号：右外沿阴影 + 主字 */
    if (use_native_font(ch))
    {
        const uint16_t *g = native_glyph(ch);
        int px = x * g_clockWidth;
        int py = (y + 1) * g_clockWidth;

        if (color & 0x0C)
        {
            uint8_t shadow = (g_maxGray < CLOCK_SHADOW_GRAY) ? g_maxGray : CLOCK_SHADOW_GRAY;

            native_draw_shadow_plain(g, px, py, (Color)(((color & 0x0C) >> 2) & 0x03), shadow);
        }

        native_draw_plain(g, px, py, (Color)(color & 0x03));
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        if ((i + x) < MAX_X && ((i + x) >= 0))
        {
            uint8_t bit_mask = clock_glyph(ch)[i];
            if (bit_mask == 0)
                continue; // 无点可绘，直接跳过
            // 颜色掩码1：青，原x坐标 → 调用封装函数
            if (color & 1)
                draw_bits(bit_mask, i + x, y + 1, Cyan);
            // 颜色掩码2：红，原x坐标 → 调用封装函数
            if (color & 2)
                draw_bits(bit_mask, i + x, y + 1, Red);

            // x+1不越界时，处理偏移坐标
            if (x + 1 < MAX_X)
            {
                /* 纯字主题：右侧阴影固定用 5 级亮度（调暗时不高于当前最大灰度） */
                uint8_t shadow = (g_maxGray < CLOCK_SHADOW_GRAY) ? g_maxGray : CLOCK_SHADOW_GRAY;

                // 颜色掩码4：青，x+1坐标 → 阴影
                if (color & 4)
                    draw_bits_gray(bit_mask, i + x + 1, y + 1, Cyan, shadow);
                // 颜色掩码8：红，x+1坐标 → 阴影
                if (color & 8)
                    draw_bits_gray(bit_mask, i + x + 1, y + 1, Red, shadow);
            }
        }
    }
}

void draw_num(int x, int y, char ch, uint8_t height, Color color)
{

    if (color == None)
        return;

    const uint8_t *codes;
    if (height == 3)
        codes = &hex_codes_3x3[0][0];
    else if (height == 4)
        codes = &hex_codes_3x4[0][0];
    else if (height == 8)
        codes = &hex_codes_4x8[0][0];

    uint8_t width = 3;
    if (height == 8)
        width = 4;
    for (int i = 0; i < width; i++)
    {
        uint8_t ind = ch - '0' + 1;
        if (ch == ' ')
            ind = 0;

        if ((i + x) < MAX_X && ((i + x) >= 0))
        {
            uint8_t bit_mask = codes[ind * width + i] << y;
            if (bit_mask == 0)
                continue; // 无点可绘，直接跳过
            // 颜色掩码1：青，原x坐标 → 调用封装函数
            if (color & 1)
                draw_bits(bit_mask, i + x, y, Cyan);
            // 颜色掩码2：红，原x坐标 → 调用封装函数
            if (color & 2)
                draw_bits(bit_mask, i + x, y, Red);

            // x+1不越界时，处理偏移坐标
            if (x + 1 < MAX_X)
            {
                // 颜色掩码4：青，x+1坐标 → 调用封装函数
                if (color & 4)
                    draw_bits(bit_mask, i + x + 1, y, Cyan);
                // 颜色掩码8：红，x+1坐标 → 调用封装函数
                if (color & 8)
                    draw_bits(bit_mask, i + x + 1, y, Red);
            }
        }
    }
}

void draw_num_str(int x, int y, char *str, uint8_t height, Color color)
{
    if (color == None)
        return;

    int len = strlen(str);
    uint8_t width = 3;
    if (height == 8)
        width = 4;
    for (int i = 0; i < len; i++)
    {
        draw_num(x + i * (width + 1), y, str[i], height, color);
    }
}

void draw_str(int x, int y, char *str, Color color)
{
    if (color == None)
        return;

    int len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        draw_char(x + i * 6, y, str[i], color);
    }
}

void draw_str_3x5(int x, int y, char *str, Color color)
{
    int delta = 0;
    if (color == None)
        return;
    int len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        draw_char_3x5(x + delta, y, str[i], color);
        //		if (str[i] == ' ' || str[i] == ':')
        //			delta += 2;
        //		else
        delta += 4;
    }
}

void draw_str_3x5_wave(int x, int y, char *str, Color color)
{
    int delta = 0;
    if (color == None)
        return;
    int len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        draw_char_3x5_wave(x + delta, y, str[i], color);
        //		if (str[i] == ' ' || str[i] == ':')
        //			delta += 2;
        //		else
        delta += 4;
    }
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
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, g_maxGray);
            if (color & 2)
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, g_maxGray);
        }
        else
            mn11236_clear_pixel(x, y, MN11236_COLOR_RED);

        if (x + 1 < MAX_X)
        {
            if (color & 4)
                mn11236_draw_pixel_gray(x + 1, y, MN11236_COLOR_BLUE, g_maxGray);
            if (color & 8)
                mn11236_draw_pixel_gray(x + 1, y, MN11236_COLOR_RED, g_maxGray);
        }
    }
}

void draw_point_gray(int x, int y, Color color,
                     uint8_t gray)
{
    if (x < MAX_X && y < MAX_Y && x >= 0 && y >= 0)
    {
        if (color)
        {
            if (color & 1)
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, gray);
            if (color & 2)
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, gray);
        }
        else
            mn11236_clear_pixel(x, y, MN11236_COLOR_RED);

        if (x + 1 < MAX_X)
        {
            if (color & 4)
                mn11236_draw_pixel_gray(x + 1, y, MN11236_COLOR_BLUE, gray);
            if (color & 8)
                mn11236_draw_pixel_gray(x + 1, y, MN11236_COLOR_RED, gray);
        }
    }
}

/*
 * 翻页动画：把字符按“可见高度” visH（0..15 行）压扁，
 * 以字格垂直中线为轴收缩/展开（像卡片翻面）。
 * visH = 满高时和正常绘制一致，visH = 0 时什么都不画（正好在折页线上）。
 * wave = 1：波形背景混色（动画背景主题）；wave = 0：普通绘制。
 */
void draw_char_flip(int x, int y, char ch, Color color, uint8_t visH, uint8_t wave)
{
    int px;
    int cellTop;
    int glyphH;

    if (color == None || visH == 0)
        return;

    px = x * g_clockWidth;
    cellTop = (y + 1) * g_clockWidth;

    if (use_native_font(ch))
    {
        const uint16_t *g = native_glyph(ch);
        int top;

        if (visH > NATIVE_GLYPH_H)
            visH = NATIVE_GLYPH_H;

        top = (NATIVE_GLYPH_H - visH) / 2;

        for (int rs = 0; rs < NATIVE_GLYPH_H; rs++)
        {
            int tr = top + rs * visH / NATIVE_GLYPH_H;

            for (int c = 0; c < NATIVE_GLYPH_W; c++)
            {
                if (!(g[c] & (1u << rs)))
                    continue;

                if (wave)
                    draw_point_wave(px + c, cellTop + tr, color);
                else
                    draw_point(px + c, cellTop + tr, color);
            }
        }

        return;
    }

    /* 标准 3x5 字模：一个字模行对应 g_clockWidth 个设备行 */
    glyphH = 5 * g_clockWidth;

    if (visH > glyphH)
        visH = glyphH;

    int top = (glyphH - visH) / 2;

    for (int rs = 0; rs < glyphH; rs++)
    {
        int tr = top + rs * visH / glyphH;
        int bit = rs / g_clockWidth;
        const uint8_t *glyph = clock_glyph(ch);

        for (int i = 0; i < 3; i++)
        {
            if (!(glyph[i] & (1 << bit)))
                continue;

            for (int dx = 0; dx < g_clockWidth; dx++)
            {
                int p = px + i * g_clockWidth + dx;

                if (wave)
                    draw_point_wave(p, cellTop + tr, color);
                else
                    draw_point(p, cellTop + tr, color);
            }
        }
    }
}

/*
 * 数字切换的“碎裂/拼合”动画（和菜单切换同一套观感）：
 * 把字格按 3x3 像素切成 3 列 x 5 行共 15 块碎片，
 *   phase = 0：旧数字碎掉，碎片带重力下落并向外散开
 *   phase = 1：新数字从字格上方落下拼合，同时向中间收拢
 * elapsed：该阶段已经进行的毫秒数
 * wave = 1：波形背景混色；wave = 0：普通绘制。
 */
#define SHARD_TRAVEL_MS 180 /* 单块碎片走完全程的时间 */
#define SHARD_DROP_PX 20    /* 拼合时碎片起始高度（字格上方多少像素） */
#define SHARD_GRAVITY 1500  /* 下落加速度：位移 = t^2 / SHARD_GRAVITY */

void draw_char_shatter(int x, int y, char ch, Color color, uint8_t wave,
                       uint8_t phase, uint16_t elapsed)
{
    int16_t offX[5][3];
    int16_t offY[5][3];
    int px;
    int cellTop;

    if (color == None)
        return;

    px = x * g_clockWidth;
    cellTop = (y + 1) * g_clockWidth;

    /* 先算出 15 块碎片各自的偏移 */
    for (int sr = 0; sr < 5; sr++)
    {
        for (int sc = 0; sc < 3; sc++)
        {
            int delay = ((sr * 3 + sc * 5) % 4) * 8; /* 每块错开 0~24ms，避免整块一起动 */
            int dt = (int)elapsed - delay;

            if (dt < 0)
                dt = 0;

            if (phase == 0)
            {
                offY[sr][sc] = (int16_t)(dt * dt / SHARD_GRAVITY);
                offX[sr][sc] = (int16_t)((sc - 1) * (dt * dt / 8000));
            }
            else
            {
                int q = SHARD_TRAVEL_MS - dt;

                if (q < 0)
                    q = 0;

                offY[sr][sc] = (int16_t)(-(SHARD_DROP_PX * q * q) /
                                         (SHARD_TRAVEL_MS * SHARD_TRAVEL_MS));
                offX[sr][sc] = (int16_t)((sc - 1) * (3 * q / SHARD_TRAVEL_MS));
            }
        }
    }

    if (use_native_font(ch))
    {
        const uint16_t *g = native_glyph(ch);

        for (int r = 0; r < NATIVE_GLYPH_H; r++)
        {
            int sr = r * 5 / NATIVE_GLYPH_H;

            for (int c = 0; c < NATIVE_GLYPH_W; c++)
            {
                int sc;

                if (!(g[c] & (1u << r)))
                    continue;

                sc = c * 3 / NATIVE_GLYPH_W;

                if (wave)
                    draw_point_wave(px + c + offX[sr][sc], cellTop + r + offY[sr][sc], color);
                else
                    draw_point(px + c + offX[sr][sc], cellTop + r + offY[sr][sc], color);
            }
        }

        return;
    }

    /* 标准 3x5 字模：一个字模行对应 g_clockWidth 个设备行 */
    {
        int glyphH = 5 * g_clockWidth;
        const uint8_t *glyph = clock_glyph(ch);

        for (int r = 0; r < glyphH; r++)
        {
            int bit = r / g_clockWidth;
            int sr = r * 5 / glyphH;

            for (int i = 0; i < 3; i++)
            {
                if (!(glyph[i] & (1 << bit)))
                    continue;

                for (int dx = 0; dx < g_clockWidth; dx++)
                {
                    int c = i * g_clockWidth + dx;

                    if (wave)
                        draw_point_wave(px + c + offX[sr][i], cellTop + r + offY[sr][i], color);
                    else
                        draw_point(px + c + offX[sr][i], cellTop + r + offY[sr][i], color);
                }
            }
        }
    }
}

/*
 * 冒号上下两点分别给亮度（0..7），用来做错时呼吸：
 * 上点先亮、下点滞后，熄灭时也是上点先灭。
 * wave = 1：波形背景混色；wave = 0：普通绘制（纯字主题）。
 */
void draw_colon_fade(int x, int y, Color color, uint8_t topLevel, uint8_t botLevel, uint8_t wave)
{
    int px;
    int cellTop;
    int glyphH;

    if (color == None)
        return;

    if (topLevel > 7)
        topLevel = 7;
    if (botLevel > 7)
        botLevel = 7;

    px = x * g_clockWidth;
    cellTop = (y + 1) * g_clockWidth;

    if (use_native_font(':'))
    {
        const uint16_t *g = native_glyph(':');

        for (int rs = 0; rs < NATIVE_GLYPH_H; rs++)
        {
            uint8_t level = (rs < NATIVE_GLYPH_H / 2) ? topLevel : botLevel;
            int edge = native_right_edge(g, rs);

            if (wave)
            {
                /* 右侧阴影：黑色，随该点一起淡入淡出 */
                if (edge >= 0)
                    draw_point_wave_fade(px + edge + 1, cellTop + rs, None, level);

                for (int c = 0; c < NATIVE_GLYPH_W; c++)
                    if (g[c] & (1u << rs))
                        draw_point_wave_fade(px + c, cellTop + rs, color, level);
            }
            else
            {
                for (int c = 0; c < NATIVE_GLYPH_W; c++)
                    if (g[c] & (1u << rs))
                        draw_point_gray(px + c, cellTop + rs, (Color)(color & 0x03), level);

                if (edge >= 0 && (color & 0x0C))
                {
                    uint8_t sh = (level < CLOCK_SHADOW_GRAY) ? level : CLOCK_SHADOW_GRAY;

                    draw_point_gray(px + edge + 1, cellTop + rs,
                                    (Color)(((color & 0x0C) >> 2) & 0x03), sh);
                }
            }
        }

        return;
    }

    /* 标准 3x5 字模 */
    glyphH = 5 * g_clockWidth;

    for (int rs = 0; rs < glyphH; rs++)
    {
        uint8_t level = (rs < glyphH / 2) ? topLevel : botLevel;
        int bit = rs / g_clockWidth;
        const uint8_t *glyph = clock_glyph(':');

        for (int i = 0; i < 3; i++)
        {
            if (!(glyph[i] & (1 << bit)))
                continue;

            for (int dx = 0; dx < g_clockWidth; dx++)
            {
                int p = px + i * g_clockWidth + dx;

                if (wave)
                    draw_point_wave_fade(p, cellTop + rs, color, level);
                else
                    draw_point_gray(p, cellTop + rs, (Color)(color & 0x03), level);
            }

            /* 右侧阴影：波形主题留 1 个字符格的黑色，纯字主题用阴影色 */
            if (wave)
            {
                int sp = px + (i + 1) * g_clockWidth;

                for (int dx = 0; dx < g_clockWidth; dx++)
                    draw_point_wave_fade(sp + dx, cellTop + rs, None, level);
            }
            else if (color & 0x0C)
            {
                uint8_t sh = (level < CLOCK_SHADOW_GRAY) ? level : CLOCK_SHADOW_GRAY;
                int sp = px + (i + 1) * g_clockWidth;

                for (int dx = 0; dx < g_clockWidth; dx++)
                    draw_point_gray(sp + dx, cellTop + rs,
                                    (Color)(((color & 0x0C) >> 2) & 0x03), sh);
            }
        }
    }
}

void draw_icon(int x, int y, IconType type, Color color)
{
    const uint8_t *icon_data;
    int width;
    switch(type) {
        case ICON_RPM:
            icon_data = icon_rpm;
						width = sizeof(icon_rpm);
            break;
        case ICON_KMH:
            icon_data = icon_kmh;
						width = sizeof(icon_kmh);
            break;
        case ICON_LOAD:
            icon_data = icon_load;
						width = sizeof(icon_load);
            break;
        case ICON_VOLT:
            icon_data = icon_volt;
						width = sizeof(icon_volt);
            break;
        default:
            icon_data = icon_temp;
						width = sizeof(icon_temp);
            break;
    }
    if (color == None)
        return;
    draw_bitmap(x, y, icon_data, width, 5, color);
}

void draw_icon_wave(int x, int y, IconType type, Color color)
{
    const uint8_t *icon_data;
    int width = 15;
    switch(type) {
        case ICON_RPM:
            icon_data = icon_rpm;
						width = sizeof(icon_rpm);
            break;
        case ICON_KMH:
            icon_data = icon_kmh;
						width = sizeof(icon_kmh);
            break;
        case ICON_LOAD:
            icon_data = icon_load;
						width = sizeof(icon_load);
            break;
        case ICON_VOLT:
            icon_data = icon_volt;
						width = sizeof(icon_volt);
            break;
        default:
            icon_data = icon_temp;
						width = sizeof(icon_temp);
            break;
    }
    if (color == None)
        return;
    draw_bitmap_wave(x, y, icon_data, width, 5, color);
}

void draw_bitmap(int x, int y, const uint8_t *bitmap, int width, int height, Color color)
{
    if (color == None || bitmap == NULL || width <= 0 || height <= 0 || height > 8)
        return;

    for (int col = 0; col < width; col++)
    {
        int draw_x = x + col;
        if (draw_x >= MAX_X || draw_x < 0)
            continue;

        uint8_t bit_mask = bitmap[col];
        // Mask to only the lower 'height' bits
        bit_mask &= (1 << height) - 1;

        if (bit_mask == 0)
            continue;

        // Draw the bits
        if (color & 1)
            draw_bits(bit_mask, draw_x, y, Cyan);
        if (color & 2)
            draw_bits(bit_mask, draw_x, y, Red);

        // Handle offset for color effects
        if (x + 1 < MAX_X)
        {
            if (color & 4)
                draw_bits(bit_mask, draw_x + 1, y, Cyan);
            if (color & 8)
                draw_bits(bit_mask, draw_x + 1, y, Red);
        }
    }
}

void draw_bitmap_wave(int x, int y, const uint8_t *bitmap, int width, int height, Color color)
{
    if (color == None || bitmap == NULL || width <= 0 || height <= 0 || height > 8)
        return;

    for (int col = 0; col < width; col++)
    {
        int draw_x = x + col;
        if (draw_x >= MAX_X || draw_x < 0)
            continue;

        uint8_t bit_mask = bitmap[col];
        // Mask to only the lower 'height' bits
        bit_mask &= (1 << height) - 1;

        if (bit_mask == 0)
            continue;

        draw_bits_wave(bit_mask, draw_x, y, color);

        // Handle offset for color effects
        if (x + 1 < MAX_X)
        {
            draw_bits_wave(bit_mask, draw_x + 1, y, None);
        }
    }
}

void refrash_gram()
{
    mn11236_send_buffer();
}

/* ============================================================
 * 画面碎裂落地 / 碎片归位拼合 共用
 * ============================================================ */
#define SHATTER_SHARD_W 14
#define SHATTER_SHARD_H 6
#define SHATTER_STEP_MS 16
#define SHATTER_TOTAL_MS 400

/* 碎裂/拼合共用的画面快照（[x][y]，与驱动缓冲区同布局） */
static uint8_t g_shatterSnap[VFD_WIDTH][VFD_HEIGHT];

/*
 * 当前画面碎裂落地：切成小块，从中心向外依次裂开，
 * 碎片带横向飞溅 + 重力下落，全部掉出屏幕后清屏返回。
 */
void clock_shatter_anim(void)
{
    uint8_t *buf = mn11236_get_buffer();
    uint32_t t_begin = HAL_GetTick();
    int cols = (VFD_WIDTH + SHATTER_SHARD_W - 1) / SHATTER_SHARD_W;
    int rows = (VFD_HEIGHT + SHATTER_SHARD_H - 1) / SHATTER_SHARD_H;

    /* 抓取当前画面作为碎片来源 */
    memcpy(g_shatterSnap, buf, sizeof(g_shatterSnap));

    for (int t = 0; t <= SHATTER_TOTAL_MS; t += SHATTER_STEP_MS)
    {
        mn11236_clear_buffers();

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                /* 奇数行错开半块，裂纹看起来不规则 */
                int x0 = c * SHATTER_SHARD_W - ((r & 1) ? SHATTER_SHARD_W / 2 : 0);
                int y0 = r * SHATTER_SHARD_H;
                int w = SHATTER_SHARD_W - 1; /* 留 1px 缝当裂纹 */
                int h = SHATTER_SHARD_H - 1;
                int cx, dist, delay, dt, dx, dy, dir;

                if (x0 < 0)
                {
                    w += x0;
                    x0 = 0;
                }
                if (x0 + w > VFD_WIDTH)
                    w = VFD_WIDTH - x0;
                if (y0 + h > VFD_HEIGHT)
                    h = VFD_HEIGHT - y0;
                if (w <= 0 || h <= 0)
                    continue;

                cx = x0 + w / 2;
                dist = (cx > VFD_WIDTH / 2) ? (cx - VFD_WIDTH / 2) : (VFD_WIDTH / 2 - cx);

                /* 中间先碎，越靠边越晚，再叠一点参差 */
                delay = dist * 2 + r * 20 + (c * 7 + r * 13) % 26;

                dt = t - delay;
                dx = 0;
                dy = 0;

                if (dt > 0)
                {
                    dir = (cx < VFD_WIDTH / 2) ? -1 : 1;
                    dx = dir * ((dt * dt) / 9000 + dt / 40); /* 向外飞溅 */
                    dy = (dt * dt) / 3000;                   /* 重力加速下落 */
                }

                if (dy >= VFD_HEIGHT)
                    continue;

                for (int yy = 0; yy < h; yy++)
                {
                    int sy = y0 + yy;
                    int ty = sy + dy;

                    if (ty < 0 || ty >= VFD_HEIGHT)
                        continue;

                    for (int xx = 0; xx < w; xx++)
                    {
                        int tx = x0 + xx + dx;
                        uint8_t v = g_shatterSnap[x0 + xx][sy];

                        if (v == 0 || tx < 0 || tx >= VFD_WIDTH)
                            continue;

                        buf[tx * VFD_HEIGHT + ty] = v;
                    }
                }
            }
        }

        mn11236_send_buffer();
        osDelay(SHATTER_STEP_MS);
    }

    mn11236_clear_buffers();
    mn11236_send_buffer();

    /* 过渡期间后台动画停表 */
    clock_anim_pause(HAL_GetTick() - t_begin);
}

/* ============================================================
 * 新画面从下方拼合上来
 * 调用前缓冲区里必须是完整的新画面；结束时缓冲区恢复为新画面。
 * 碎片从屏幕上方落下、由外向内归位，逐块“拼”回完整画面。
 * ============================================================ */
#define REBUILD_TOTAL_MS 340 /* 总时长 */
#define REBUILD_DUR_MS 260   /* 单块碎片落下时长 */
#define REBUILD_DROP_PX 20   /* 起始悬停高度（整屏高度 18） */

void clock_shatter_rebuild(void)
{
    uint8_t *buf = mn11236_get_buffer();
    uint32_t t_begin = HAL_GetTick();
    int cols = (VFD_WIDTH + SHATTER_SHARD_W - 1) / SHATTER_SHARD_W;
    int rows = (VFD_HEIGHT + SHATTER_SHARD_H - 1) / SHATTER_SHARD_H;

    /* 抓取新画面 */
    memcpy(g_shatterSnap, buf, sizeof(g_shatterSnap));

    for (int t = 0; t <= REBUILD_TOTAL_MS; t += SHATTER_STEP_MS)
    {
        mn11236_clear_buffers();

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                int x0 = c * SHATTER_SHARD_W - ((r & 1) ? SHATTER_SHARD_W / 2 : 0);
                int y0 = r * SHATTER_SHARD_H;
                int w = SHATTER_SHARD_W - 1;
                int h = SHATTER_SHARD_H - 1;
                int delay, p, q, dy, dx, cx, dir;

                if (x0 < 0)
                {
                    w += x0;
                    x0 = 0;
                }
                if (x0 + w > VFD_WIDTH)
                    w = VFD_WIDTH - x0;
                if (y0 + h > VFD_HEIGHT)
                    h = VFD_HEIGHT - y0;
                if (w <= 0 || h <= 0)
                    continue;

                /* 每块错开一点，看起来是一块块拼上去的 */
                delay = ((c * 5 + r * 11) % 4) * 18;

                p = t - delay;
                if (p < 0)
                    p = 0;
                if (p > REBUILD_DUR_MS)
                    p = REBUILD_DUR_MS;

                q = REBUILD_DUR_MS - p;
                /* 从上方落下：先快后慢，最后稳稳落位 */
                dy = -(REBUILD_DROP_PX * q * q / (REBUILD_DUR_MS * REBUILD_DUR_MS));

                /* 从外侧向内收拢 */
                cx = x0 + w / 2;
                dir = (cx < VFD_WIDTH / 2) ? -1 : 1;
                dx = -dir * (3 * q / REBUILD_DUR_MS);

                if (dy <= -VFD_HEIGHT)
                    continue;

                for (int yy = 0; yy < h; yy++)
                {
                    int sy = y0 + yy;
                    int ty = sy + dy;

                    if (ty < 0 || ty >= VFD_HEIGHT)
                        continue;

                    for (int xx = 0; xx < w; xx++)
                    {
                        int tx = x0 + xx + dx;
                        uint8_t v = g_shatterSnap[x0 + xx][sy];

                        if (v == 0 || tx < 0 || tx >= VFD_WIDTH)
                            continue;

                        buf[tx * VFD_HEIGHT + ty] = v;
                    }
                }
            }
        }

        mn11236_send_buffer();
        osDelay(SHATTER_STEP_MS);
    }

    /* 最后一帧可能还差几个像素，直接恢复完整新画面 */
    memcpy(buf, g_shatterSnap, sizeof(g_shatterSnap));
    mn11236_send_buffer();

    /* 过渡期间后台动画停表 */
    clock_anim_pause(HAL_GetTick() - t_begin);
}

static uint8_t g_snakeWidth = 2; // 默认宽度
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
    SNAKE_DYING,    // 正在从头到尾变红
    SNAKE_DEAD_WAIT // 已全红，等待重生
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
        case 0:
            dy = 1;
            break; // 上 → 身体向下
        case 1:
            dx = -1;
            break; // 右 → 身体向左
        case 2:
            dy = -1;
            break; // 下 → 身体向上
        case 3:
            dx = 1;
            break; // 左 → 身体向右
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
static inline void draw_snake_block_gray(int x, int y, uint8_t color, uint8_t gray)
{
    int px = x * g_snakeWidth;
    int py = y * g_snakeWidth;

    for (int dx = 0; dx < g_snakeWidth; dx++)
        for (int dy = 0; dy < g_snakeWidth; dy++)
            draw_point_gray(px + dx, py + dy, (Color)color, gray);
}

void printBoard(Snake_t *snake, Food_t *food)
{
    int len = snake->length;

    for (int i = 0; i < len; i++)
    {
        float t = (float)i / (len - 1); // 0 → 1

        uint8_t orange = 0;
        uint8_t cyan = 0;

        if (i == 0)
        {
            // 头：纯白
            orange = 7;
            cyan = 7;
        }
        else
        {
            if (t < 0.4f)
            {
                // 白 → 青蓝
                float k = t / 0.4f; // 0~1
                orange = (uint8_t)(7 * (1.0f - k));
                cyan = 7;
            }
            else
            {
                // 青蓝 → 渐灭
                float k = (t - 0.4f) / 0.6f; // 0~1
                orange = 0;
                cyan = (uint8_t)(7 * (1.0f - k));
            }
        }

        if (cyan < 1 && i != 0)
            cyan = 1; // 保证不完全熄灭（可删）

        draw_snake_block_gray(
            snake->body[i].x,
            snake->body[i].y,
            Red,
            orange);
        draw_snake_block_gray(
            snake->body[i].x,
            snake->body[i].y,
            Cyan,
            cyan);
    }

    draw_snake_block_gray(food->x, food->y, FOOD_DOT, 7);
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
        if (dir == 0)
            ny--;
        if (dir == 1)
            nx++;
        if (dir == 2)
            ny++;
        if (dir == 3)
            nx--;

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
    case 0:
        snake->body[0].y--;
        break;
    case 1:
        snake->body[0].x++;
        break;
    case 2:
        snake->body[0].y++;
        break;
    case 3:
        snake->body[0].x--;
        break;
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
    int len = snake->length;

    if (len <= 0)
        return;

    for (int i = 0; i < len; i++)
    {
        uint8_t redLevel = 0;
        uint8_t cyanLevel = 0;

        if (i <= deathIndex)
        {
            /* ---------- 已扩散到的位置 ---------- */

            /* 计算相对扩散位置 */
            int d = deathIndex - i; // 距离“爆点”的距离

            if (d < 0)
                d = 0;
            if (d > 7)
                d = 7;

            /* 红色从7递减 */
            redLevel = 7 - d;

            if (redLevel < 1)
                redLevel = 1;

            cyanLevel = 0; // 被红覆盖
        }
        else
        {
            /* ---------- 还没扩散到 ---------- */
            /* 保持原来的蓝色渐暗 */

            int t = i * 100 / (len - 1);

            if (t < 50)
            {
                cyanLevel = 7;
            }
            else
            {
                cyanLevel = 7 - ((t - 50) * 7 / 50);
            }

            if (cyanLevel < 1)
                cyanLevel = 1;

            redLevel = 0;
        }

        /* 绘制 */
        if (redLevel > 0)
        {
            draw_snake_block_gray(
                snake->body[i].x,
                snake->body[i].y,
                Red,
                redLevel);
        }

        if (cyanLevel > 0)
        {
            draw_snake_block_gray(
                snake->body[i].x,
                snake->body[i].y,
                Cyan,
                cyanLevel);
        }
    }
}

void setSnakeWidth(uint8_t width)
{
    if (width < 1)
        width = 1;
    if (width > 8) // 可按屏幕性能限制
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

    /* ====== 2. 红色渐暗阶段 ====== */
    if (snakeState == SNAKE_DEAD_WAIT)
    {
        uint8_t redLevel;

        if (deadWaitCnt <= 7)
            redLevel = deadWaitCnt;
        else
            redLevel = 0;

        for (int i = 0; i < snake.length; i++)
        {
            if (redLevel > 0)
            {
                draw_snake_block_gray(
                    snake.body[i].x,
                    snake.body[i].y,
                    Red,
                    redLevel);
            }
        }

        deadWaitCnt++;

        if (deadWaitCnt > 7)
        {
            /* 完全熄灭后重生 */
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
        int height = (int)bars[i].output;
        for (j = 0; j < (bars[i].output); j++)
        {
            int brightness = (height - j) * 7 / height + 2;
					
						draw_point_gray(i, MAX_Y - 1 - j, bar, brightness);
        }
        draw_point(i, MAX_Y - 1 - bars[i].maximum, dot);
        if (bar > White)
            i++;
    }
}

void draw_fft(long *fft_result, int len, int delta, Color bar, Color dot)
{
    static uint32_t last_time = 0;
    uint32_t current_time = clock_anim_tick();
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

