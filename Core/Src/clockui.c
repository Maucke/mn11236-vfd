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

/* ============================================================
 * 俄罗斯方块界面（DISP_MODE_GAME 且 game==1）
 * 3x3 像素一格：37 列 x 6 行（右侧余 1 像素）
 *   - 方块统一从屏幕正中出现（原始朝向），先滑到 AI 选好的列，再一格一格转过去
 *   - 每块随机红/蓝/白；拼块内部满亮度，轮廓与块间暗缝用灰度 3
 *   - 满行先变白（白条同样带暗边），再从中间向两边擦掉，上方整体下落
 *   - 堆到顶部后停止掉落，加速逐行消除，清空后重新开始
 *   - 右上角小窗显示"下一块"（2 像素一格，黑底 + 一圈暗边框，棋盘在那儿让位）
 * ============================================================ */
/* 定义在文件后面的逐点绘制函数，这里先声明（C99 不允许隐式声明） */
void draw_point_gray(int x, int y, Color color, uint8_t gray);

#define TET_PX 3
#define TET_COLS (MAX_X / TET_PX) /* 37 列 */
#define TET_ROWS (MAX_Y / TET_PX) /* 6 行 */

#define TET_DROP_MS 300        /* 下落时每 300ms 掉一格 */
#define TET_ENTER_MS 130       /* 进场时每 130ms 从屏幕上边露出一行 */
#define TET_SPIN_MS 90         /* 整块进屏幕之后，每转 90° 停多久 */
#define TET_WIPE_MS 320        /* 满行擦除时长 */
#define TET_TOPOUT_WIPE_MS 110 /* 顶满后逐行加速消除 */

#define TET_CELL_RED 1
#define TET_CELL_BLUE 2
#define TET_CELL_WHITE 3

/* 棋盘格编码：高 6 位拼块编号、低 2 位颜色，0 表示空。
 * 拼块编号用来在相邻的不同拼块之间留一条暗缝。 */
#define TET_PID_SHIFT 2
#define TET_COLOR_MASK 0x03
#define TET_PID_MAX 63

#define TET_ST_FALL 0
#define TET_ST_CLEAR 1
#define TET_ST_ENTER 2 /* 从屏幕上边一格一格掉进来（这期间不许转） */
#define TET_ST_SPIN 3  /* 整块都进屏幕了，原地转正 */

/* 右上角"下一块"预览：2 像素一格，最多 4x2 格，外面套一圈暗边框。
 * 整屏都是棋盘，所以它只是叠在上面的小窗，边框压暗、底下的方块照样看得见。 */
#define TET_NEXT_PX 2                          /* 预览里一格几像素 */
#define TET_NEXT_W (4 * TET_NEXT_PX)           /* 8 */
#define TET_NEXT_H (2 * TET_NEXT_PX)           /* 4 */
#define TET_NEXT_X (MAX_X - TET_NEXT_W - 1)    /* 103：边框贴着屏幕右边 */
#define TET_NEXT_Y 1
#define TET_NEXT_EDGE 1                        /* 边框灰度：比方块暗边的 3 还暗一点 */
#define TET_NEXT_GRAY 3                         /* 填充灰度*/

static uint8_t tet_board[TET_ROWS][TET_COLS]; /* 0 空 / 1 红 / 2 蓝 */
static uint16_t tet_shape[7][4];              /* 7 种形状 x 4 个旋转，已左上角对齐 */
static uint8_t tet_inited;

static uint8_t tet_state;
static uint8_t tet_active; /* 当前方块是否还在下落 */
static uint8_t tet_topout; /* 顶满后的加速消行模式 */
static uint8_t tet_shape_id;
static uint8_t tet_rot_id;
static uint8_t tet_rot_show; /* 画出来的朝向：出场时从 0 一格一格转到 tet_rot_id */
static uint8_t tet_color;
static uint8_t tet_pid = 1; /* 当前拼块编号（每生成一块 +1） */
static int8_t tet_x, tet_y;
static int8_t tet_x_from, tet_x_to; /* 进场滑动：起点列（屏幕正中）/ 目标列 */
static uint32_t tet_enter_t0;       /* 进场起点时刻 */
static uint32_t tet_spin_t0;        /* 原地旋转一步的起点时刻 */

static uint16_t tet_clears; /* 正在消除的行（bit0 = 第 0 行） */
static uint32_t tet_clear_ms;
static uint32_t tet_t0;   /* 消除动画起点 */
static uint32_t tet_last; /* 上次下落的时刻 */

static uint32_t tet_rnd = 0x9E3779B9u;

static uint32_t tet_rand(void)
{
    tet_rnd = tet_rnd * 1664525u + 1013904223u;
    return tet_rnd >> 8;
}

/* 先摇出"下一块"存着，等这一块落地再用，这样右上角才有得显示 */
static uint8_t tet_next_shape;
static uint8_t tet_next_color;

static void tet_roll_next(void)
{
    tet_next_shape = (uint8_t)(tet_rand() % 7);
    tet_next_color = (uint8_t)(TET_CELL_RED + tet_rand() % 3); /* 红 / 蓝 / 白随机 */
}

/* 顺时针旋转 90°（4x4 框内） */
static uint16_t tet_rot90(uint16_t m)
{
    uint16_t r = 0;

    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (m & (uint16_t)(1u << (y * 4 + x)))
                r |= (uint16_t)(1u << (x * 4 + (3 - y)));

    return r;
}

/* 把形状挪到 4x4 框左上角，方便按格数算宽高 */
static uint16_t tet_align(uint16_t m)
{
    int minx = 4, miny = 4;
    uint16_t r = 0;

    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (m & (uint16_t)(1u << (y * 4 + x)))
            {
                if (x < minx)
                    minx = x;
                if (y < miny)
                    miny = y;
            }

    if (minx == 4)
        return 0;

    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (m & (uint16_t)(1u << (y * 4 + x)))
                r |= (uint16_t)(1u << ((y - miny) * 4 + (x - minx)));

    return r;
}

static int tet_mask_w(uint16_t m)
{
    int w = 0;

    for (int x = 0; x < 4; x++)
        for (int y = 0; y < 4; y++)
            if (m & (uint16_t)(1u << (y * 4 + x)))
            {
                if (x + 1 > w)
                    w = x + 1;
                break;
            }

    return w;
}

static int tet_mask_h(uint16_t m)
{
    int h = 0;

    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (m & (uint16_t)(1u << (y * 4 + x)))
            {
                h = y + 1;
                break;
            }

    return h;
}

/* 方块框左上角放在 (x,y) 时是否越界/重叠（y<0 表示还在屏幕上方，允许） */
static uint8_t tet_hits(int x, int y, uint16_t m)
{
    for (int by = 0; by < 4; by++)
        for (int bx = 0; bx < 4; bx++)
        {
            int cx, cy;

            if (!(m & (uint16_t)(1u << (by * 4 + bx))))
                continue;

            cx = x + bx;
            cy = y + by;

            if (cx < 0 || cx >= TET_COLS)
                return 1;
            if (cy >= TET_ROWS)
                return 1;
            if (cy >= 0 && tet_board[cy][cx])
                return 1;
        }

    return 0;
}

static void tet_heights(uint8_t *h)
{
    for (int c = 0; c < TET_COLS; c++)
    {
        int r = 0;

        while (r < TET_ROWS && tet_board[r][c] == 0)
            r++;

        h[c] = (uint8_t)(TET_ROWS - r);
    }
}

/* 落点评分：低、平、能消行最好；留洞、堆到屏幕外重罚 */
static int32_t tet_score(uint16_t m, int x, const uint8_t *h)
{
    uint8_t nh[TET_COLS];
    int rest[4] = {-1, -1, -1, -1};
    int land = 1 << 14;
    int agg = 0, bump = 0, minh = 255, over = 0, cleared;
    int holes = 0;
    int32_t sumsq = 0;

    for (int bx = 0; bx < 4; bx++)
    {
        int bot = -1;

        for (int by = 3; by >= 0; by--)
            if (m & (uint16_t)(1u << (by * 4 + bx)))
            {
                bot = by;
                break;
            }

        if (bot < 0)
            continue;

        rest[bx] = (TET_ROWS - 1 - (int)h[x + bx]) - bot;
        if (rest[bx] < land)
            land = rest[bx];
    }

    if (land == (1 << 14))
        return -100000;

    /* 悬空盖住的空格：这一列贴地比最低列还高多少，就留几个洞 */
    for (int bx = 0; bx < 4; bx++)
        if (rest[bx] >= 0)
            holes += rest[bx] - land;

    for (int c = 0; c < TET_COLS; c++)
        nh[c] = h[c];

    for (int bx = 0; bx < 4; bx++)
    {
        int top = -1, c, hh;

        for (int by = 0; by < 4; by++)
            if (m & (uint16_t)(1u << (by * 4 + bx)))
            {
                top = by;
                break;
            }

        if (top < 0)
            continue;

        c = x + bx;
        hh = TET_ROWS - (land + top);
        if (hh > (int)nh[c])
            nh[c] = (uint8_t)(hh > 255 ? 255 : hh);
    }

    for (int c = 0; c < TET_COLS; c++)
    {
        agg += nh[c];
        sumsq += (int32_t)nh[c] * nh[c];
        if (nh[c] < minh)
            minh = nh[c];
        if (nh[c] > TET_ROWS)
            over += nh[c] - TET_ROWS;
        if (c)
            bump += (nh[c] > nh[c - 1]) ? (nh[c] - nh[c - 1]) : (nh[c - 1] - nh[c]);
    }

    /* 底部连续填满的行数（近似消行数） */
    cleared = (minh > TET_ROWS) ? TET_ROWS : minh;
    agg -= cleared * TET_COLS;
    if (agg < 0)
        agg = 0;

    /* 高度平方和这一项很关键：只看相邻列高差的话，格子多而扁时会堆成一边高一边低的斜坡 */
    return (int32_t)cleared * 40 - (int32_t)agg - (int32_t)bump * 2 -
           (int32_t)holes * 200 - sumsq * 3 - (int32_t)over * 200;
}

static void tet_pick(uint8_t shape, int *out_x, int *out_rot)
{
    uint8_t h[TET_COLS];
    int32_t best = -0x7FFFFFFF;
    int bx = 0, br = 0;

    tet_heights(h);

    for (int r = 0; r < 4; r++)
    {
        int w = tet_mask_w(tet_shape[shape][r]);

        for (int x = 0; x + w <= TET_COLS; x++)
        {
            int32_t s = tet_score(tet_shape[shape][r], x, h);

            if (s > best)
            {
                best = s;
                bx = x;
                br = r;
            }
        }
    }

    *out_x = bx;
    *out_rot = br;
}

static int tet_lowest_row(void)
{
    for (int r = TET_ROWS - 1; r >= 0; r--)
        for (int c = 0; c < TET_COLS; c++)
            if (tet_board[r][c])
                return r;

    return -1;
}

static uint8_t tet_row_full(int r)
{
    for (int c = 0; c < TET_COLS; c++)
        if (tet_board[r][c] == 0)
            return 0;

    return 1;
}

static void tet_spawn(uint32_t now)
{
    int x, rot, w, h;
    uint8_t shape = tet_next_shape; /* 用早先摇好的"下一块"，再摇新的下一块 */
    uint16_t m;

    tet_pick(shape, &x, &rot);

    tet_shape_id = shape;
    tet_rot_id = (uint8_t)rot;
    tet_color = tet_next_color;
    tet_roll_next();

    tet_pid = (uint8_t)(tet_pid % TET_PID_MAX + 1); /* 换一个拼块编号，和旧块之间留暗缝 */

    /* 进场用“原始朝向”的尺寸：整块先待在屏幕上边外面，再一格一格掉进来 */
    m = tet_shape[shape][0];
    w = tet_mask_w(m);
    h = tet_mask_h(m);

    tet_x_from = (int8_t)((TET_COLS - w) / 2); /* 从屏幕正中往下走 */
    tet_x_to = (int8_t)x;
    tet_x = tet_x_from;
    tet_y = (int8_t)(-h);
    tet_rot_show = 0;
    tet_active = 1;
    tet_state = TET_ST_ENTER;
    tet_enter_t0 = now;
}

static void tet_lock(uint32_t now)
{
    uint16_t m = tet_shape[tet_shape_id][tet_rot_id];
    uint16_t full = 0;
    uint8_t above = 0;
    int low;

    for (int by = 0; by < 4; by++)
        for (int bx = 0; bx < 4; bx++)
        {
            int cx, cy;

            if (!(m & (uint16_t)(1u << (by * 4 + bx))))
                continue;

            cx = tet_x + bx;
            cy = tet_y + by;

            if (cx < 0 || cx >= TET_COLS)
                continue;
            if (cy < 0)
            {
                above = 1;
                continue;
            }
            if (cy >= TET_ROWS)
                continue;

            tet_board[cy][cx] = (uint8_t)((tet_pid << TET_PID_SHIFT) | tet_color);
        }

    tet_active = 0;

    if (above) /* 已经堆出屏幕：进入加速消行 */
    {
        low = tet_lowest_row();

        tet_topout = 1;
        tet_state = TET_ST_CLEAR;
        tet_clear_ms = TET_TOPOUT_WIPE_MS;
        tet_t0 = now;
        tet_clears = (low >= 0) ? (uint16_t)(1u << low) : 0;
        return;
    }

    for (int r = 0; r < TET_ROWS; r++)
        if (tet_row_full(r))
            full |= (uint16_t)(1u << r);

    if (full)
    {
        tet_clears = full;
        tet_clear_ms = TET_WIPE_MS;
        tet_t0 = now;
        tet_state = TET_ST_CLEAR;
    }
    else
    {
        tet_spawn(now);
    }
}

static void tet_remove_rows(uint16_t mask)
{
    int dst = TET_ROWS - 1;

    for (int r = TET_ROWS - 1; r >= 0; r--)
    {
        if (mask & (uint16_t)(1u << r))
            continue;
        if (dst != r)
            memcpy(tet_board[dst], tet_board[r], TET_COLS);
        dst--;
    }

    for (; dst >= 0; dst--)
        memset(tet_board[dst], 0, TET_COLS);
}

static void tet_finish_clear(uint32_t now)
{
    int low;

    tet_remove_rows(tet_clears);
    tet_clears = 0;

    if (tet_topout)
    {
        low = tet_lowest_row();

        if (low < 0) /* 清空了，重新开始堆 */
        {
            tet_topout = 0;
            tet_spawn(now);
            return;
        }

        tet_clears = (uint16_t)(1u << low);
        tet_clear_ms = TET_TOPOUT_WIPE_MS;
        tet_t0 = now;
        return;
    }

    tet_spawn(now);
}

static void tet_step(uint32_t now)
{
    if (tet_state == TET_ST_CLEAR)
    {
        if (now - tet_t0 >= tet_clear_ms)
            tet_finish_clear(now);
        return;
    }

    if (!tet_active)
        return;

    if (tet_state == TET_ST_ENTER)
    {
        /* 进场：整块从屏幕上边外面一格一格掉下来，同时从屏幕正中缓到目标列。
         * 整块都进屏幕（tet_y >= 0）之前不许旋转。 */
        int rows = tet_mask_h(tet_shape[tet_shape_id][0]);
        uint32_t dur = (uint32_t)rows * TET_ENTER_MS;
        uint32_t el = now - tet_enter_t0;
        uint32_t et = (el > dur) ? dur : el;
        uint16_t mt = tet_shape[tet_shape_id][tet_rot_id]; /* 按最终朝向判断放不放得下 */
        int ny, blocked = 0;
        int32_t p, e, span;

        /* 横着：从头到尾从屏幕正中缓到目标列 */
        p = (int32_t)(et * 256u / dur);
        e = p * p * (768 - 2 * p) / 65536; /* smoothstep(0..1) 的 Q8 版本 */
        span = (int32_t)tet_x_to - (int32_t)tet_x_from;
        tet_x = (int8_t)(tet_x_from + span * e / 256);

        /* 竖着：一格一格往下露；下面已经堆起来了就停在那一格 */
        ny = (int)(et / TET_ENTER_MS) - rows;

        while (tet_y < ny)
        {
            if (tet_hits(tet_x, tet_y + 1, mt))
            {
                blocked = 1;
                break;
            }

            tet_y++;
        }

        if (el < dur && !blocked)
            return;

        if (!blocked && tet_rot_show != tet_rot_id) /* 整块进屏幕了，这才开始转 */
        {
            tet_state = TET_ST_SPIN;
            tet_spin_t0 = now;
            return;
        }

        tet_rot_show = tet_rot_id; /* 没地方转就按最终朝向放，紧接着就落地 */
        tet_state = TET_ST_FALL;
        tet_last = now;
        return;
    }

    if (tet_state == TET_ST_SPIN)
    {
        if (now - tet_spin_t0 >= TET_SPIN_MS)
        {
            tet_spin_t0 += TET_SPIN_MS;
            tet_rot_show++;

            if (tet_rot_show >= tet_rot_id)
            {
                tet_rot_show = tet_rot_id;
                tet_state = TET_ST_FALL;
                tet_last = now;
            }
        }

        return;
    }

    if (now - tet_last < TET_DROP_MS)
        return;

    tet_last += TET_DROP_MS;

    if (tet_hits(tet_x, tet_y + 1, tet_shape[tet_shape_id][tet_rot_id]))
        tet_lock(now);
    else
        tet_y++;
}

/* 拼块边缘压暗、内部满亮度：
 * 只有"邻格不是同一拼块（空格或别的块）"的那一侧才压暗，
 * 所以每块 4 格内部是成片的亮色，块与块之间、块与背景之间都留一条暗缝。
 * 斜对角也参与判断，否则凹进去的内折角会缺一个暗像素，轮廓断个口。 */
#define TET_EDGE_GRAY 3
#define TET_FILL_GRAY 7

/* 当前是否有一块在场（进场 / 旋转 / 下落） */
static uint8_t tet_piece_alive(void)
{
    return (uint8_t)(tet_active && (tet_state == TET_ST_FALL ||
                                    tet_state == TET_ST_ENTER ||
                                    tet_state == TET_ST_SPIN));
}

/* 取一格的占用值：高 6 位拼块编号、低 2 位颜色，0 = 空 */
static uint8_t tet_cell_at(int c, int r)
{
    if (c < 0 || c >= TET_COLS || r < 0 || r >= TET_ROWS)
        return 0;

    if (tet_board[r][c])
        return tet_board[r][c];

    if (tet_piece_alive())
    {
        int bx = c - tet_x, by = r - tet_y;

        if (bx >= 0 && bx < 4 && by >= 0 && by < 4 &&
            (tet_shape[tet_shape_id][tet_rot_show] & (uint16_t)(1u << (by * 4 + bx))))
            return (uint8_t)((tet_pid << TET_PID_SHIFT) | tet_color);
    }

    return 0;
}

/* 相邻格不是同一拼块（空、别块、屏幕外）→ 这一侧压暗 */
static uint8_t tet_seam(int c, int r, int dc, int dr, uint8_t self)
{
    return (uint8_t)(tet_cell_at(c + dc, r + dr) != self);
}

/* 右上角预览窗盖住的范围：棋盘画到这里就跳过，给它留一块纯黑底 */
static uint8_t tet_hud_hit(int x, int y)
{
    return (uint8_t)(x >= TET_NEXT_X - 1 && x <= TET_NEXT_X + TET_NEXT_W &&
                     y >= TET_NEXT_Y - 1 && y <= TET_NEXT_Y + TET_NEXT_H);
}

static void tet_put_cell(int c, int r, Color color)
{
    int x0, y0;
    uint8_t self, edge_l, edge_r, edge_u, edge_d;
    uint8_t dg_lu, dg_ru, dg_ld, dg_rd;

    if (c < 0 || c >= TET_COLS || r < 0 || r >= TET_ROWS)
        return;

    self = tet_cell_at(c, r);
    edge_l = tet_seam(c, r, -1, 0, self);
    edge_r = tet_seam(c, r, 1, 0, self);
    edge_u = tet_seam(c, r, 0, -1, self);
    edge_d = tet_seam(c, r, 0, 1, self);
    dg_lu = tet_seam(c, r, -1, -1, self);
    dg_ru = tet_seam(c, r, 1, -1, self);
    dg_ld = tet_seam(c, r, -1, 1, self);
    dg_rd = tet_seam(c, r, 1, 1, self);

    x0 = c * TET_PX;
    y0 = r * TET_PX;

    for (int dy = 0; dy < TET_PX; dy++)
        for (int dx = 0; dx < TET_PX; dx++)
        {
            uint8_t edge = 0;

            if (dx == 0 && edge_l)
                edge = 1;
            if (dx == TET_PX - 1 && edge_r)
                edge = 1;
            if (dy == 0 && edge_u)
                edge = 1;
            if (dy == TET_PX - 1 && edge_d)
                edge = 1;

            /* 内折角：斜对角的格子空着时，这个角像素也要压暗 */
            if (dx == 0 && dy == 0 && dg_lu)
                edge = 1;
            if (dx == TET_PX - 1 && dy == 0 && dg_ru)
                edge = 1;
            if (dx == 0 && dy == TET_PX - 1 && dg_ld)
                edge = 1;
            if (dx == TET_PX - 1 && dy == TET_PX - 1 && dg_rd)
                edge = 1;

            if (tet_hud_hit(x0 + dx, y0 + dy)) /* 让出右上角预览窗 */
                continue;

            draw_point_gray(x0 + dx, y0 + dy, color, edge ? TET_EDGE_GRAY : TET_FILL_GRAY);
        }
}

/* 消行时整行刷白：整行当成一块来画，上下沿、左右到屏幕边都压暗边，
 * 中间连续满亮度。相邻行也在消行时不断开，几条一起消就是一条完整的白带。 */
static void tet_put_cell_solid(int c, int r, Color color)
{
    int x0, y0;
    uint8_t edge_l, edge_r, edge_u, edge_d;

    if (c < 0 || c >= TET_COLS || r < 0 || r >= TET_ROWS)
        return;

    edge_l = (uint8_t)(c == 0);            /* 屏幕左边缘 */
    edge_r = (uint8_t)(c == TET_COLS - 1); /* 屏幕右边缘 */
    edge_u = (uint8_t)(r == 0 || !((tet_clears >> (r - 1)) & 1u));
    edge_d = (uint8_t)(r == TET_ROWS - 1 || !((tet_clears >> (r + 1)) & 1u));

    x0 = c * TET_PX;
    y0 = r * TET_PX;

    for (int dy = 0; dy < TET_PX; dy++)
        for (int dx = 0; dx < TET_PX; dx++)
        {
            uint8_t edge = (uint8_t)((dy == 0 && edge_u) ||
                                     (dy == TET_PX - 1 && edge_d) ||
                                     (dx == 0 && edge_l) ||
                                     (dx == TET_PX - 1 && edge_r));

            if (tet_hud_hit(x0 + dx, y0 + dy)) /* 让出右上角预览窗 */
                continue;

            draw_point_gray(x0 + dx, y0 + dy, color, edge ? TET_EDGE_GRAY : TET_FILL_GRAY);
        }
}

/* 棋盘颜色值 -> 屏幕颜色（低 2 位才是颜色） */
static Color tet_cell_color(uint8_t v)
{
    v &= TET_COLOR_MASK;

    if (v == TET_CELL_RED)
        return Red;
    if (v == TET_CELL_BLUE)
        return Cyan;
    if (v == TET_CELL_WHITE)
        return White;

    return None;
}

/* 右上角：下一块预览。2 像素一格，形状居中放在 4x2 格里，
 * 外圈画一条暗边把它和棋盘隔开（只压暗边框那几像素，底下的方块还看得见）。 */
static void tet_render_next(void)
{
    uint16_t m = tet_shape[tet_next_shape][0];
    int w = tet_mask_w(m);
    int h = tet_mask_h(m);
    int ox = TET_NEXT_X + (TET_NEXT_W - w * TET_NEXT_PX) / 2;
    int oy = TET_NEXT_Y + (TET_NEXT_H - h * TET_NEXT_PX) / 2;
    Color color = tet_cell_color(tet_next_color);

    for (int x = TET_NEXT_X - 1; x <= TET_NEXT_X + TET_NEXT_W; x++)
    {
        draw_point_gray(x, TET_NEXT_Y - 1, White, TET_NEXT_EDGE);
        draw_point_gray(x, TET_NEXT_Y + TET_NEXT_H, White, TET_NEXT_EDGE);
    }

    for (int y = TET_NEXT_Y; y < TET_NEXT_Y + TET_NEXT_H; y++)
    {
        draw_point_gray(TET_NEXT_X - 1, y, White, TET_NEXT_EDGE);
        draw_point_gray(TET_NEXT_X + TET_NEXT_W, y, White, TET_NEXT_EDGE);
    }

    for (int by = 0; by < 4; by++)
        for (int bx = 0; bx < 4; bx++)
        {
            if (!(m & (uint16_t)(1u << (by * 4 + bx))))
                continue;

            for (int dy = 0; dy < TET_NEXT_PX; dy++)
                for (int dx = 0; dx < TET_NEXT_PX; dx++)
                    draw_point_gray(ox + bx * TET_NEXT_PX + dx,
                                    oy + by * TET_NEXT_PX + dy, color, TET_NEXT_GRAY);
        }
}

static void tet_render(uint32_t now)
{
    int wipe = 0;
    int c0, c1;

    /* 消行：整行先变白，再从中间向两边擦掉 */
    if (tet_clears)
    {
        uint32_t el = now - tet_t0;

        if (el > tet_clear_ms)
            el = tet_clear_ms;

        wipe = (int)((uint32_t)(TET_COLS / 2) * el / tet_clear_ms);
    }

    c0 = TET_COLS / 2 - wipe;
    c1 = TET_COLS / 2 + wipe;

    for (int r = 0; r < TET_ROWS; r++)
    {
        uint8_t clearing = (uint8_t)((tet_clears >> r) & 1u);

        for (int c = 0; c < TET_COLS; c++)
        {
            if (clearing)
            {
                /* 中间已经擦掉的部分留黑，还没擦到的部分保持白色 */
                if (c < c0 || c >= c1)
                    tet_put_cell_solid(c, r, White);
                continue;
            }

            if (tet_board[r][c])
                tet_put_cell(c, r, tet_cell_color(tet_board[r][c]));
        }
    }

    /* 正在出场滑动 / 旋转 / 下落的方块 */
    if (tet_piece_alive())
    {
        uint16_t m = tet_shape[tet_shape_id][tet_rot_show];
        Color color = tet_cell_color(tet_color);

        for (int by = 0; by < 4; by++)
            for (int bx = 0; bx < 4; bx++)
                if (m & (uint16_t)(1u << (by * 4 + bx)))
                    tet_put_cell(tet_x + bx, tet_y + by, color);
    }

    /* 下一块预览画在最后，压在其他东西上面 */
    tet_render_next();
}

/* 俄罗斯方块界面：推进一帧并画到显存（调用方负责先清屏） */
void refrashTetris(void)
{
    uint32_t now = clock_anim_tick();

    if (!tet_inited)
    {
        static const uint16_t base[7] = {
            0x00F0, /* I */
            0x0033, /* O */
            0x0072, /* T */
            0x0036, /* S */
            0x0063, /* Z */
            0x0071, /* J */
            0x0074, /* L */
        };

        memset(tet_board, 0, sizeof(tet_board));
        for (int s = 0; s < 7; s++)
        {
            tet_shape[s][0] = tet_align(base[s]);
            for (int r = 1; r < 4; r++)
                tet_shape[s][r] = tet_align(tet_rot90(tet_shape[s][r - 1]));
        }

        tet_state = TET_ST_FALL;
        tet_last = now;
        tet_inited = 1;
        tet_roll_next(); /* 先把"下一块"摇出来，再生成当前这块 */
        tet_spawn(now);
    }

    /* 切走界面 / 长时间没画之后回来：不要一次补几十步 */
    if (now - tet_last > 1000)
        tet_last = now;

    tet_step(now);
    tet_render(now);
}

/* ============================================================
 * 坦克大战界面（DISP_MODE_GAME 且 game==2）
 * 1 像素一格：车身 4x4、2x2 一块墙、1 像素一颗子弹
 *   - 红蓝两队各 3 辆，满场乱跑、互相开火
 *   - 墙分单层（暗灰，打得掉）和多层（更亮，炸不毁）
 *   - 一方全灭就结束本局，碎屏动画换一张新地图接着打
 * ============================================================ */
#define TNK_LONG 4  /* 车身长边 */
#define TNK_SHORT 4 /* 车身短边 */
#define TNK_BOX 4   /* 碰撞方框边长：车身正好铺满，转身不会卡住 */

#define TNK_WALL_MAX 40
#define TNK_TANK_NUM 6 /* 红蓝各 3 辆 */
#define TNK_BULLET_MAX 10
#define TNK_BOOM_MAX 5

#define TNK_SIM_MS 8      /* 仿真步长 */
#define TNK_MOVE_MS 46    /* 坦克每 46ms 走 1 像素 */
#define TNK_BULLET_MS 16  /* 子弹每 16ms 走 1 像素 */
#define TNK_FIRE_MIN 500  /* 开火间隔 */
#define TNK_FIRE_MAX 1300
#define TNK_REAIM_MS 120  /* 没瞄到人时隔多久再看一眼 */
#define TNK_BREAK_MS 420  /* 顶着砖墙拆墙的开炮间隔 */
#define TNK_BOOM_MS 280
#define TNK_ROUND_END_MS 350 /* 全灭后停一下再走碎屏换关 */
#define TNK_ROUND_MAX_MS 30000 /* 一局最长 30 秒：到点就换关，防止两边卡住打不完 */
#define TNK_BOOM_R 2         /* 爆炸菱形半径 */
#define TNK_LIVES 3          /* 每辆坦克 3 条命：中 3 炮才炸 */
#define TNK_HURT_MS 700      /* 中一炮之后闪 0.7 秒，这期间打不穿 */

#define TNK_DIR_UP 0
#define TNK_DIR_RIGHT 1
#define TNK_DIR_DOWN 2
#define TNK_DIR_LEFT 3

static const int8_t tnk_dx[4] = {0, 1, 0, -1};
static const int8_t tnk_dy[4] = {-1, 0, 1, 0};

/* 车身位图只存“朝上”这一张（4x4，中间的炮塔 2 格宽，两条履带各 1 格）：
 *   . # # .     炮塔
 *   # # # #     车体
 *   # # # #     车体
 *   # . . #     两条履带
 * 另外三个朝向都按 90 度一格现转，保证四个方向严格互转
 */
static const uint8_t tnk_body_up[TNK_SHORT] = {0x06, 0x0F, 0x0F, 0x09};

/* 朝 dir 时车身的宽高：朝左右就竖过来 */
static void tnk_size(int dir, int *w, int *h)
{
    if (dir & 1)
    {
        *w = TNK_SHORT;
        *h = TNK_LONG;
    }
    else
    {
        *w = TNK_LONG;
        *h = TNK_SHORT;
    }
}

/* 车身上 (x,y) 这一格是不是实心的：把坐标反着转回“朝上”基准图再查 */
static uint8_t tnk_body_at(int x, int y, int dir)
{
    int w, h;

    tnk_size(dir, &w, &h);

    for (int k = 0; k < dir; k++)
    {
        int t = x;

        x = y;
        y = w - 1 - t;

        t = w;
        w = h;
        h = t;
    }

    return (uint8_t)((tnk_body_up[y] >> x) & 1u);
}

/* 炮口那条边（车头最前面的一排）点得更亮，用来指示朝向 */
static uint8_t tnk_bar_line(int dx, int dy, int w, int h, int dir)
{
    switch (dir)
    {
    case TNK_DIR_UP: return (uint8_t)(dy == 0);
    case TNK_DIR_RIGHT: return (uint8_t)(dx == w - 1);
    case TNK_DIR_DOWN: return (uint8_t)(dy == h - 1);
    default: return (uint8_t)(dx == 0);
    }
}

/* 炮口可以出弹的几列（沿车体宽度方向，相对碰撞方框）：正好是炮塔那 2 格 */
static void tnk_lane_range(int dir, int *lo, int *hi)
{
    (void)dir;
    *lo = TNK_BOX / 2 - 1;
    *hi = TNK_BOX / 2;
}

typedef struct
{
    uint8_t x, y; /* 左上角 */
    uint8_t dir;
    uint8_t team;    /* 0 红 / 1 蓝 */
    uint8_t alive;
    uint8_t lives;      /* 还剩几条命 */
    uint16_t hurt_ms;   /* >0：刚中一炮，闪一下，这期间无敌 */
    uint16_t move_ms;
    uint16_t fire_ms;
} TnkTank;

typedef struct
{
    uint8_t x, y, dir, team, on;
} TnkBullet;

typedef struct
{
    uint8_t x, y;
    uint16_t age;
} TnkBoom;

/* [0]=x [1]=y [2]=层数：0 单层（能打掉）/ 1 多层（炸不毁） */
static uint8_t tnk_wall[TNK_WALL_MAX][3];
static uint8_t tnk_wall_n;
static TnkTank tnk_tank[TNK_TANK_NUM];
static TnkBullet tnk_bullet[TNK_BULLET_MAX];
static TnkBoom tnk_boom[TNK_BOOM_MAX];

static uint8_t tnk_inited;
static uint8_t tnk_regen;
static uint16_t tnk_acc;
static uint16_t tnk_btick;
static uint16_t tnk_round_ms; /* >0：本局已结束，倒计时开下一局 */
static uint16_t tnk_round_len; /* 本局已经跑了多久 */
static uint8_t tnk_level_flip; /* 该换关了：碎屏动画 + 新地图 */
static uint32_t tnk_last;

/* 下面几个函数互相调用，先声明一下 */
static void tnk_muzzle_dir(const TnkTank *t, int d, int lane, int *ox, int *oy);
static uint8_t tnk_wall_at(int x, int y);
static uint8_t tnk_wall_hit(int x, int y);
static int tnk_find_break(const TnkTank *t, int d, int max);

static int tnk_rnd(int n)
{
    if (n <= 1)
        return 0;

    return (int)(tet_rand() % (uint32_t)n);
}

/* TNK_BOX x TNK_BOX 的位置能不能站：越界、砖墙、别的坦克都不行 */
static uint8_t tnk_blocked(int x, int y, int self)
{
    if (x < 0 || y < 0 || x + TNK_BOX > MAX_X || y + TNK_BOX > MAX_Y)
        return 1;

    for (int i = 0; i < tnk_wall_n; i++)
        if (x < tnk_wall[i][0] + 2 && tnk_wall[i][0] < x + TNK_BOX &&
            y < tnk_wall[i][1] + 2 && tnk_wall[i][1] < y + TNK_BOX)
            return 1;

    for (int i = 0; i < TNK_TANK_NUM; i++)
    {
        if (i == self || !tnk_tank[i].alive)
            continue;

        if (x < tnk_tank[i].x + TNK_BOX && tnk_tank[i].x < x + TNK_BOX &&
            y < tnk_tank[i].y + TNK_BOX && tnk_tank[i].y < y + TNK_BOX)
            return 1;
    }

    return 0;
}

/* 朝最近的敌人走：横竖两个候选方向优先，都被堵就换别的 */
static uint8_t tnk_pick_dir(int self)
{
    TnkTank *t = &tnk_tank[self];
    uint8_t order[4];
    int cx, cy;
    int best = -1, bd = 1 << 14;
    int adx, ady, tmp;
    uint8_t hd, vd;

    cx = t->x + TNK_BOX / 2;
    cy = t->y + TNK_BOX / 2;

    for (int i = 0; i < TNK_TANK_NUM; i++)
    {
        int dx, dy, d;

        if (i == self || !tnk_tank[i].alive || tnk_tank[i].team == t->team)
            continue;

        dx = (int)tnk_tank[i].x + TNK_BOX / 2 - cx;
        dy = (int)tnk_tank[i].y + TNK_BOX / 2 - cy;
        d = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);

        if (d < bd)
        {
            bd = d;
            best = i;
        }
    }

    if (best < 0) /* 没敌人了：随便走 */
        return (uint8_t)tnk_rnd(4);

    adx = (int)tnk_tank[best].x + TNK_BOX / 2 - cx;
    ady = (int)tnk_tank[best].y + TNK_BOX / 2 - cy;

    hd = (adx < 0) ? TNK_DIR_LEFT : TNK_DIR_RIGHT;
    vd = (ady < 0) ? TNK_DIR_UP : TNK_DIR_DOWN;

    if ((adx < 0 ? -adx : adx) >= (ady < 0 ? -ady : ady))
    {
        order[0] = hd;
        order[1] = vd;
    }
    else
    {
        order[0] = vd;
        order[1] = hd;
    }

    order[2] = (uint8_t)((order[0] + 2) & 3);
    order[3] = (uint8_t)((order[1] + 2) & 3);

    if (tnk_rnd(100) < 30) /* 偶尔先把两个候选顺序倒一下，别走得太呆 */
    {
        tmp = order[0];
        order[0] = order[1];
        order[1] = (uint8_t)tmp;
    }

    /* 前路被能打掉的砖墙堵着：不绕路，原地顶着把它轰开 */
    if (tnk_blocked(t->x + tnk_dx[t->dir], t->y + tnk_dy[t->dir], self) &&
        tnk_find_break(t, t->dir, 2) >= 0)
        return t->dir;

    /* 只能直行 / 左转 / 右转，不许原地掉头 */
    for (int i = 0; i < 4; i++)
    {
        int d = order[i];

        if (((d - t->dir) & 3) == 2)
            continue;

        if (!tnk_blocked(t->x + tnk_dx[d], t->y + tnk_dy[d], self))
            return (uint8_t)d;
    }

    return t->dir;
}

static void tnk_place_tank(int i)
{
    TnkTank *t = &tnk_tank[i];
    uint8_t dir = (uint8_t)tnk_rnd(4);

    for (int k = 0; k < 60; k++)
    {
        int x = tnk_rnd(MAX_X - TNK_BOX + 1);
        int y = tnk_rnd(MAX_Y - TNK_BOX + 1);

        if (tnk_blocked(x, y, i))
            continue;

        t->x = (uint8_t)x;
        t->y = (uint8_t)y;
        t->dir = dir;
        t->alive = 1;
        t->move_ms = (uint16_t)tnk_rnd(TNK_MOVE_MS);
        t->fire_ms = (uint16_t)(200 + tnk_rnd(900));
        return;
    }

    /* 随机没撞上：老老实实从左上角扫一个空位出来 */
    for (int y = 0; y + TNK_BOX <= MAX_Y; y++)
        for (int x = 0; x + TNK_BOX <= MAX_X; x++)
            if (!tnk_blocked(x, y, i))
            {
                t->x = (uint8_t)x;
                t->y = (uint8_t)y;
                t->dir = dir;
                t->alive = 1;
                t->move_ms = TNK_MOVE_MS;
                t->fire_ms = (uint16_t)(200 + tnk_rnd(900));
                return;
            }
}

static void tnk_boom_add(int x, int y)
{
    int slot = -1;

    for (int i = 0; i < TNK_BOOM_MAX; i++)
        if (tnk_boom[i].age >= TNK_BOOM_MS)
        {
            slot = i;
            break;
        }

    if (slot < 0)
        slot = 0;

    tnk_boom[slot].x = (uint8_t)x;
    tnk_boom[slot].y = (uint8_t)y;
    tnk_boom[slot].age = 0;
}

/* 中一炮：掉一条命，车身闪一下接着打；命掉光了才真的炸掉 */
static void tnk_hit(int i, int x, int y)
{
    TnkTank *t = &tnk_tank[i];

    if (t->hurt_ms) /* 刚挨过一炮，这 0.7 秒打不穿 */
        return;

    tnk_boom_add(x, y);
    t->lives--;

    if (t->lives)
    {
        t->hurt_ms = TNK_HURT_MS;
        return;
    }

    t->alive = 0;
}

/* 炮口位置：离碰撞框边缘一格，lane 是沿车体宽度方向的列号 */
static void tnk_muzzle_dir(const TnkTank *t, int d, int lane, int *ox, int *oy)
{
    switch (d)
    {
    case TNK_DIR_UP: *ox = t->x + lane; *oy = t->y - 1; break;
    case TNK_DIR_RIGHT: *ox = t->x + TNK_BOX; *oy = t->y + lane; break;
    case TNK_DIR_DOWN: *ox = t->x + lane; *oy = t->y + TNK_BOX; break;
    default: *ox = t->x - 1; *oy = t->y + lane; break;
    }
}

/* 这个点上有墙吗？0=没有 1=单层 2=多层 */
static uint8_t tnk_wall_at(int x, int y)
{
    for (int i = 0; i < tnk_wall_n; i++)
        if (x >= tnk_wall[i][0] && x < tnk_wall[i][0] + 2 &&
            y >= tnk_wall[i][1] && y < tnk_wall[i][1] + 2)
            return (uint8_t)(tnk_wall[i][2] + 1);

    return 0;
}

/* 顺着 d 方向看出去 max 格：有没有能打掉的砖墙挡着？有就返回对应的炮口列 */
static int tnk_find_break(const TnkTank *t, int d, int max)
{
    int lo, hi;

    tnk_lane_range(d, &lo, &hi);

    for (int l = lo; l <= hi; l++)
    {
        int x, y;

        tnk_muzzle_dir(t, d, l, &x, &y);

        for (int n = 0; n < max; n++)
        {
            uint8_t wt;
            int blocked_by_tank = 0;

            if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
                break;

            /* 前面挡着车：不算墙堵路，别对着自己人放炮 */
            for (int k = 0; k < TNK_TANK_NUM; k++)
            {
                const TnkTank *o = &tnk_tank[k];

                if (!o->alive)
                    continue;

                if (x >= o->x && x < o->x + TNK_BOX && y >= o->y && y < o->y + TNK_BOX)
                {
                    blocked_by_tank = 1;
                    break;
                }
            }

            if (blocked_by_tank)
                break;

            wt = tnk_wall_at(x, y);

            if (wt == 1)
                return l; /* 单层砖：一炮能拆 */

            if (wt == 2)
                break; /* 多层：别浪费炮弹 */

            x += tnk_dx[d];
            y += tnk_dy[d];
        }
    }

    return -1;
}

/* 从炮口的第 lane 列出膛、朝 d 方向打，能打到敌方坦克吗？（墙会挡） */
static uint8_t tnk_lane_clear(const TnkTank *t, int d, int lane)
{
    int x, y;

    tnk_muzzle_dir(t, d, lane, &x, &y);

    for (int n = 0; n < MAX_X + MAX_Y; n++)
    {
        if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
            return 0;

        for (int k = 0; k < TNK_TANK_NUM; k++)
        {
            const TnkTank *o = &tnk_tank[k];

            if (!o->alive || o->team == t->team)
                continue;

            if (x >= o->x && x < o->x + TNK_BOX && y >= o->y && y < o->y + TNK_BOX)
                return 1;
        }

        /* 单层砖可以打穿，多层墙挡视线 */
        if (tnk_wall_at(x, y) == 2)
            return 0;

        x += tnk_dx[d];
        y += tnk_dy[d];
    }

    return 0;
}

/* 挑一个能打到人的朝向（顺带给出炮口列），打不到就返回 -1 */
static int tnk_find_aim(const TnkTank *t, int *lane)
{
    int lo, hi;

    /* 先看当前对着的方向，免得来回扭头 */
    tnk_lane_range(t->dir, &lo, &hi);

    for (int l = lo; l <= hi; l++)
    {
        if (tnk_lane_clear(t, t->dir, l))
        {
            *lane = l;
            return t->dir;
        }
    }

    for (int d = 0; d < 4; d++)
    {
        if (d == t->dir || ((d - t->dir) & 3) == 2)
            continue; /* 不原地掉头：想打背后，先左/右转一格再说 */

        tnk_lane_range(d, &lo, &hi);

        for (int l = lo; l <= hi; l++)
        {
            if (tnk_lane_clear(t, d, l))
            {
                *lane = l;
                return d;
            }
        }
    }

    return -1;
}

static void tnk_fire(int i, int lane)
{
    TnkTank *t = &tnk_tank[i];
    int bx, by;

    tnk_muzzle_dir(t, t->dir, lane, &bx, &by);

    if (bx < 0 || bx >= MAX_X || by < 0 || by >= MAX_Y)
        return;

    /* 炮口就贴在墙上：单层砖当场拆掉，多层墙这一发根本打不出去 */
    {
        uint8_t wt = tnk_wall_at(bx, by);

        if (wt == 2)
            return;

        if (wt == 1)
        {
            tnk_wall_hit(bx, by);
            return;
        }
    }

    /* 顶着别人开火：出膛即命中，别让子弹从人家身体里穿过去 */
    for (int k = 0; k < TNK_TANK_NUM; k++)
    {
        TnkTank *o = &tnk_tank[k];

        if (!o->alive || o->team == t->team || o->hurt_ms) /* 无敌状态：让它打过去 */
            continue;

        if (bx >= o->x && bx < o->x + TNK_BOX && by >= o->y && by < o->y + TNK_BOX)
        {
            tnk_hit(k, bx, by);
            return;
        }
    }

    for (int k = 0; k < TNK_BULLET_MAX; k++)
    {
        TnkBullet *b = &tnk_bullet[k];

        if (b->on)
            continue;

        b->on = 1;
        b->x = (uint8_t)bx;
        b->y = (uint8_t)by;
        b->dir = t->dir;
        b->team = t->team;
        return;
    }
}

static void tnk_tank_step(int i)
{
    TnkTank *t = &tnk_tank[i];
    int nd = t->dir;

    if (tnk_rnd(100) < 8) /* 偶尔主动换个方向，看着像在找目标 */
        nd = tnk_pick_dir(i);

    if (!tnk_blocked(t->x + tnk_dx[nd], t->y + tnk_dy[nd], i))
    {
        t->dir = (uint8_t)nd;
        t->x = (uint8_t)(t->x + tnk_dx[nd]);
        t->y = (uint8_t)(t->y + tnk_dy[nd]);
        return;
    }

    /* 撞墙 / 撞车：换个方向，下一拍再走（方框不随朝向变，转身随时都行） */
    t->dir = tnk_pick_dir(i);

    /* 真被围死了：允许倒着挪一格（不掉头，朝向不变） */
    {
        int back = (int)((t->dir + 2) & 3);

        if (tnk_blocked(t->x + tnk_dx[t->dir], t->y + tnk_dy[t->dir], i) &&
            tnk_find_break(t, t->dir, 2) < 0 &&
            !tnk_blocked(t->x + tnk_dx[back], t->y + tnk_dy[back], i))
        {
            t->x = (uint8_t)(t->x + tnk_dx[back]);
            t->y = (uint8_t)(t->y + tnk_dy[back]);
        }
    }
}

/* 打中墙：单层直接拆掉，多层留着挡路 */
static uint8_t tnk_wall_hit(int x, int y)
{
    for (int i = 0; i < tnk_wall_n; i++)
        if (x >= tnk_wall[i][0] && x < tnk_wall[i][0] + 2 &&
            y >= tnk_wall[i][1] && y < tnk_wall[i][1] + 2)
        {
            if (tnk_wall[i][2])
                return 1; /* 多层：炸不毁 */

            tnk_wall_n--;
            tnk_wall[i][0] = tnk_wall[tnk_wall_n][0];
            tnk_wall[i][1] = tnk_wall[tnk_wall_n][1];
            tnk_wall[i][2] = tnk_wall[tnk_wall_n][2];
            return 1;
        }

    return 0;
}

static void tnk_bullets_step(void)
{
    for (int i = 0; i < TNK_BULLET_MAX; i++)
    {
        TnkBullet *b = &tnk_bullet[i];
        int x, y;

        if (!b->on)
            continue;

        x = b->x + tnk_dx[b->dir];
        y = b->y + tnk_dy[b->dir];

        if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
        {
            b->on = 0;
            continue;
        }

        if (tnk_wall_hit(x, y))
        {
            b->on = 0;
            continue;
        }

        for (int k = 0; k < TNK_TANK_NUM; k++)
        {
            TnkTank *t = &tnk_tank[k];

            if (!t->alive || t->team == b->team || t->hurt_ms) /* 无敌状态：子弹穿过去 */
                continue;

            if (x >= t->x && x < t->x + TNK_BOX && y >= t->y && y < t->y + TNK_BOX)
            {
                tnk_hit(k, x, y);
                b->on = 0;
                break;
            }
        }

        if (!b->on)
            continue;

        b->x = (uint8_t)x;
        b->y = (uint8_t)y;
    }
}

/* 随机地图：几个 2x2 的墙簇，随便长几块；有的簇是多层的，炸不毁 */
static void tnk_level(void)
{
    tnk_wall_n = 0;

    for (int c = 0; c < 8; c++)
    {
        int x = 4 + tnk_rnd((MAX_X - 10) / 2) * 2;
        int y = 2 + tnk_rnd((MAX_Y - 8) / 2) * 2;
        int n = 2 + tnk_rnd(4);
        uint8_t kind = (uint8_t)(tnk_rnd(100) < 40); /* 1 = 多层 */

        for (int k = 0; k < n; k++)
        {
            if (tnk_wall_n >= TNK_WALL_MAX)
                return;

            tnk_wall[tnk_wall_n][0] = (uint8_t)x;
            tnk_wall[tnk_wall_n][1] = (uint8_t)y;
            tnk_wall[tnk_wall_n][2] = kind;
            tnk_wall_n++;

            switch (tnk_rnd(4))
            {
            case 0: x += 2; break;
            case 1: x -= 2; break;
            case 2: y += 2; break;
            default: y += -2; break;
            }

            if (x < 2)
                x = 2;
            if (x > MAX_X - 4)
                x = MAX_X - 4;
            if (y < 0)
                y = 0;
            if (y > MAX_Y - 2)
                y = MAX_Y - 2;
        }
    }
}

static uint8_t tnk_alive(int team)
{
    uint8_t n = 0;

    for (int i = 0; i < TNK_TANK_NUM; i++)
        if (tnk_tank[i].alive && tnk_tank[i].team == team)
            n++;

    return n;
}

/* 这队是不是全打光了（命也拼完了） */
static uint8_t tnk_team_out(int team)
{
    for (int i = 0; i < TNK_TANK_NUM; i++)
        if (tnk_tank[i].team == team && tnk_tank[i].alive)
            return 0;

    return 1;
}

/* 开一局新的：换地图、清子弹、双方满命复活 */
static void tnk_new_round(void)
{
    tnk_level();
    memset(tnk_bullet, 0, sizeof(tnk_bullet));
    tnk_round_len = 0;

    for (int i = 0; i < TNK_TANK_NUM; i++)
    {
        tnk_tank[i].alive = 0;
        tnk_tank[i].lives = TNK_LIVES;
        tnk_tank[i].hurt_ms = 0;
        tnk_place_tank(i);
    }

    if (tnk_alive(0) == 0 || tnk_alive(1) == 0)
    {
        /* 极端情况：地图太挤摆不下，干脆清空墙再摆 */
        tnk_wall_n = 0;
        memset(tnk_bullet, 0, sizeof(tnk_bullet));

        for (int i = 0; i < TNK_TANK_NUM; i++)
        {
            tnk_tank[i].alive = 0;
            tnk_tank[i].lives = TNK_LIVES;
            tnk_tank[i].hurt_ms = 0;
            tnk_place_tank(i);
        }
    }
}

static uint8_t tnk_sim(void)
{
    /* 爆炸淡出一直走，停局的时候也让它放完 */
    for (int i = 0; i < TNK_BOOM_MAX; i++)
        if (tnk_boom[i].age < TNK_BOOM_MS)
            tnk_boom[i].age = (uint16_t)(tnk_boom[i].age + TNK_SIM_MS);

    if (tnk_round_ms) /* 本局结束：停一下，等外面放换关动画 */
    {
        if (tnk_round_ms > TNK_SIM_MS)
        {
            tnk_round_ms = (uint16_t)(tnk_round_ms - TNK_SIM_MS);
        }
        else
        {
            tnk_round_ms = 0;
            tnk_level_flip = 1; /* 交给 refrashTanks 做碎屏换关 */
            return 1;           /* 本帧别继续步进，免得又判一次全灭 */
        }
        return 0;
    }

    for (int i = 0; i < TNK_TANK_NUM; i++)
    {
        TnkTank *t = &tnk_tank[i];

        if (!t->alive)
            continue;

        if (t->hurt_ms) /* 中弹后的无敌闪烁 */
        {
            if (t->hurt_ms > TNK_SIM_MS)
                t->hurt_ms = (uint16_t)(t->hurt_ms - TNK_SIM_MS);
            else
                t->hurt_ms = 0;
        }

        if (t->move_ms > TNK_SIM_MS)
            t->move_ms = (uint16_t)(t->move_ms - TNK_SIM_MS);
        else
        {
            t->move_ms = TNK_MOVE_MS;
            tnk_tank_step(i);
        }

        if (t->fire_ms > TNK_SIM_MS)
            t->fire_ms = (uint16_t)(t->fire_ms - TNK_SIM_MS);
        else
        {
            int lane = TNK_BOX / 2;
            int bl = -1;

            /* 前面被砖墙堵着就先拆墙：这条比打人优先，不然可能永远出不去 */
            if (tnk_blocked(t->x + tnk_dx[t->dir], t->y + tnk_dy[t->dir], i))
                bl = tnk_find_break(t, t->dir, 2);

            if (bl >= 0)
            {
                t->fire_ms = (uint16_t)(TNK_BREAK_MS + tnk_rnd(240));
                tnk_fire(i, bl);
            }
            else
            {
                int aim = tnk_find_aim(t, &lane);

                if (aim < 0)
                    t->fire_ms = TNK_REAIM_MS; /* 没瞄到人：不开枪 */
                else if ((uint8_t)aim != t->dir)
                {
                    /* 先转炮口，下一拍再开火（方框不随朝向变，转身随时都行） */
                    t->dir = (uint8_t)aim;
                    t->fire_ms = TNK_REAIM_MS;
                }
                else
                {
                    t->fire_ms = (uint16_t)(TNK_FIRE_MIN + tnk_rnd(TNK_FIRE_MAX - TNK_FIRE_MIN));
                    tnk_fire(i, lane);
                }
            }
        }
    }

    tnk_btick = (uint16_t)(tnk_btick + TNK_SIM_MS);

    if (tnk_btick >= TNK_BULLET_MS)
    {
        tnk_btick = 0;
        tnk_bullets_step();
    }

    /* 一方全灭（连命都打光了），或者这一局拖太久：都算本局结束，碎屏换下一关 */
    tnk_round_len = (uint16_t)(tnk_round_len + TNK_SIM_MS);

    if (tnk_team_out(0) || tnk_team_out(1) || tnk_round_len >= TNK_ROUND_MAX_MS)
        tnk_round_ms = TNK_ROUND_END_MS;

    return 0;
}

static void tnk_render(void)
{
    /* 墙：单层暗白（打得掉），多层更亮（炸不毁） */
    for (int i = 0; i < tnk_wall_n; i++)
    {
        int wx = tnk_wall[i][0];
        int wy = tnk_wall[i][1];
        uint8_t gray = (uint8_t)(tnk_wall[i][2] ? 6 : 3);

        for (int dy = 0; dy < 2; dy++)
            for (int dx = 0; dx < 2; dx++)
                draw_point_gray(wx + dx, wy + dy, White, gray);
    }

    /* 坦克：4x4 的车身，正好铺满碰撞框，炮口那条边点得更亮 */
    for (int i = 0; i < TNK_TANK_NUM; i++)
    {
        TnkTank *t = &tnk_tank[i];
        int w, h, ox, oy;
        Color c;

        if (!t->alive)
            continue;

        if (t->hurt_ms && ((t->hurt_ms / 100) & 1))
            continue; /* 中弹后闪一下，表明这条命已经掉了 */

        c = t->team ? Cyan : Red;
        tnk_size(t->dir, &w, &h);

        /* 车身贴前沿：朝下贴下边、朝右贴右边，剩下的余量留在屁股后头 */
        ox = t->x;
        oy = t->y;

        if (t->dir == TNK_DIR_DOWN)
            oy = t->y + TNK_BOX - h;
        else if (t->dir == TNK_DIR_RIGHT)
            ox = t->x + TNK_BOX - w;

        for (int dy = 0; dy < h; dy++)
            for (int dx = 0; dx < w; dx++)
            {
                uint8_t gray;

                if (!tnk_body_at(dx, dy, t->dir))
                    continue;

                gray = (uint8_t)(tnk_bar_line(dx, dy, w, h, t->dir) ? 7 : 5);
                draw_point_gray(ox + dx, oy + dy, c, gray);
            }
    }

    /* 子弹：1 像素白点 */
    for (int i = 0; i < TNK_BULLET_MAX; i++)
        if (tnk_bullet[i].on)
            draw_point_gray(tnk_bullet[i].x, tnk_bullet[i].y, White, 7);

    /* 爆炸：菱形闪一下再变暗 */
    for (int i = 0; i < TNK_BOOM_MAX; i++)
    {
        TnkBoom *b = &tnk_boom[i];
        uint8_t gray;

        if (b->age >= TNK_BOOM_MS)
            continue;

        gray = (uint8_t)((b->age < 80) ? 7 : (b->age < 180 ? 5 : 3));

        for (int dy = -TNK_BOOM_R; dy <= TNK_BOOM_R; dy++)
            for (int dx = -TNK_BOOM_R; dx <= TNK_BOOM_R; dx++)
            {
                int ad = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);

                if (ad > TNK_BOOM_R)
                    continue;

                draw_point_gray(b->x + dx, b->y + dy, White, gray);
            }
    }
}

/* 重新随机一张地图（Exit 键用） */
void tanksRegen(void)
{
    tnk_regen = 1;
    tnk_level_flip = 0; /* 别让上一局残留的换关冲击波插进游戏切换里 */
}

/* 换关动画：冲击波（实现在文件后面的过渡动画区） */
static void tnk_shockwave(void);

/* 坦克大战界面：推进一帧并画到显存（调用方负责先清屏） */
void refrashTanks(void)
{
    uint32_t now = clock_anim_tick();

    if (!tnk_inited || tnk_regen)
    {
        tnk_regen = 0;

        if (!tnk_inited)
        {
            for (int i = 0; i < TNK_TANK_NUM; i++)
                tnk_tank[i].team = (uint8_t)(i & 1);

            for (int i = 0; i < TNK_BOOM_MAX; i++)
                tnk_boom[i].age = TNK_BOOM_MS;
        }

        tnk_round_ms = 0;
        tnk_new_round();

        tnk_inited = 1;
        tnk_last = now;
        tnk_acc = 0;
        tnk_btick = 0;
    }

    /* 切走界面 / 卡顿之后回来：别一次补几十步 */
    if (now - tnk_last > 200)
        tnk_last = now;

    tnk_acc = (uint16_t)(tnk_acc + (uint16_t)(now - tnk_last));
    tnk_last = now;

    while (tnk_acc >= TNK_SIM_MS)
    {
        tnk_acc = (uint16_t)(tnk_acc - TNK_SIM_MS);
        if (tnk_sim())
            break; /* 该换关了，别再补步 */
    }

    /* 一方全灭：冲击波把旧关冲掉，再从中间涌出下一关 */
    if (tnk_level_flip)
    {
        tnk_level_flip = 0;

        tnk_render();    /* 旧关最后一帧先留在屏上 */
        tnk_shockwave(); /* 冲击波换关（内部会 tnk_new_round + tnk_render） */

        tnk_last = clock_anim_tick();
        tnk_acc = 0;
        tnk_btick = 0;
        return;
    }

    tnk_render();
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

/* ============================================================
 * 时钟界面 · 桌面宠物
 * 默认隐藏，只在分钟跳变时被 clock_pet_trigger() 叫出来演一次，
 * 演完自动消失。动作有：摇尾巴 / 跳跃 / 趴下 / 左右探头 / 顶部探头
 * 后跌落，活动范围仅限时间左右两侧的空白区（不压数字）。
 * 右侧阴影和时间数字用同一套画法：每行最右像素往右补 2 像素，
 * 亮度取 CLOCK_SHADOW_GRAY，所以阴影只会出现在轮廓外侧，不会灌进
 * 眼睛、嘴巴这些挖空里。
 * ============================================================ */

#define PET_W 10          /* 精灵宽（像素） */
#define PET_H 10          /* 精灵高（像素） */
#define PET_TAIL 3        /* 尾巴像素数 */
#define PET_GROUND 8      /* 站立时精灵顶部 y：8 + 10 = 18 */
#define PET_LIE_Y 13      /* 趴下（5 行高）顶部 y：13 + 5 = 18 */

/* 行掩码：bit0 = 最左一列，1 = 点亮 */
static const uint16_t pet_sit[10] = {
    0x0084, 0x01CE, 0x01FE, 0x03FF, 0x0333,
    0x0333, 0x03FF, 0x03CF, 0x01FE, 0x014A,
};
static const uint16_t pet_sit_blink[10] = {
    0x0084, 0x01CE, 0x01FE, 0x03FF, 0x03FF,
    0x03FF, 0x03FF, 0x03CF, 0x01FE, 0x014A,
};
static const uint16_t pet_lie[5] = {
    0x01FE, 0x0333, 0x03FF, 0x03FF, 0x014A,
};

/* 尾巴三帧：(dx, dy) 相对精灵左上角，dx 可到 12（身体右外 3 像素） */
static const int8_t pet_tail[3][PET_TAIL][2] = {
    {{10, 5}, {11, 4}, {12, 3}},
    {{10, 6}, {11, 6}, {12, 6}},
    {{10, 7}, {11, 7}, {12, 8}},
};

/* 趴下时的尾巴（贴着地面摆） */
static const int8_t pet_tail_lie[3][PET_TAIL][2] = {
    {{10, 2}, {11, 3}, {12, 4}},
    {{10, 3}, {11, 3}, {12, 3}},
    {{10, 4}, {11, 3}, {12, 2}},
};

enum {
    PET_ACT_SIT = 0,
    PET_ACT_WAG,
    PET_ACT_JUMP,
    PET_ACT_LIE,
    PET_ACT_PEEK_L,
    PET_ACT_PEEK_R,
    PET_ACT_DROP,
};

typedef struct {
    uint8_t act;
    uint8_t home;      /* 0 = 左侧空地，1 = 右侧空地 */
    uint32_t t0;
    uint32_t dur;
} PetState_t;

static PetState_t g_pet = {PET_ACT_SIT, 0, 0, 3000};

/* 0 = 藏起来（大部分时间），1 = 正在出场 */
static uint8_t pet_active = 0;

/* 简单 LCG，够用就行 */
static uint32_t pet_rand(void)
{
    static uint32_t s = 0x9E3779B9u;

    s = s * 1664525u + 1013904223u;
    return s >> 16;
}

/* 精灵落点：左侧贴左边，右侧整体贴右边（身体 10 + 尾巴 3 + 阴影 2） */
static int pet_base_x(uint8_t home, int right_limit)
{
    int x;

    if (!home)
        return 2;

    x = MAX_X - (PET_W + PET_TAIL + 2);
    if (x < right_limit)
        x = right_limit;

    return x;
}

/* 挑一个出场动作：加权随机，尽量不连着做同一个 */
static void pet_pick(uint32_t now, uint8_t allow_right)
{
    /* 每次出场只做一件事，所以不放“纯坐着” */
    static const uint8_t pool[] = {
        PET_ACT_WAG, PET_ACT_WAG,
        PET_ACT_JUMP, PET_ACT_JUMP,
        PET_ACT_LIE, PET_ACT_LIE,
        PET_ACT_PEEK_L, PET_ACT_PEEK_R,
        PET_ACT_DROP,
    };
    uint8_t n;
    uint8_t guard = 0;

    do
    {
        n = pool[pet_rand() % (sizeof(pool) / sizeof(pool[0]))];
        guard++;
    } while (n == g_pet.act && guard < 6);

    /* 右半边被日期占用时不出现在右侧 */
    if (!allow_right && n == PET_ACT_PEEK_R)
        n = PET_ACT_PEEK_L;

    g_pet.act = n;
    g_pet.t0 = now;

    switch (n)
    {
    case PET_ACT_SIT:
        g_pet.dur = 3000u;
        break;
    case PET_ACT_WAG:
        g_pet.dur = 2200u;
        break;
    case PET_ACT_JUMP:
        g_pet.dur = 1460u; /* 140 蓄力 + 3 x 340 跳 + 300 站定 */
        break;
    case PET_ACT_LIE:
        g_pet.dur = 3000u;
        break;
    case PET_ACT_PEEK_L:
        g_pet.home = 0;
        g_pet.dur = 2400u;
        break;
    case PET_ACT_PEEK_R:
        g_pet.home = 1;
        g_pet.dur = 2400u;
        break;
    default:
        g_pet.dur = 2500u;
        break;
    }
}

/* 画精灵本体 */
static void pet_blit(int x, int y, const uint16_t *rows, uint8_t h,
                     uint8_t flip, Color color)
{
    for (uint8_t r = 0; r < h; r++)
    {
        uint16_t bits = rows[r];

        for (uint8_t c = 0; bits; c++, bits >>= 1)
            if (bits & 1u)
                draw_point(x + (flip ? (PET_W - 1 - c) : c), y + r, color);
    }
}

/* 画尾巴，tail 为 NULL 表示这一帧不画 */
static void pet_tail_blit(int x, int y, const int8_t (*tail)[2],
                          uint8_t flip, Color color)
{
    for (int i = 0; i < PET_TAIL; i++)
    {
        int dx = tail[i][0];
        int dy = tail[i][1];

        draw_point(x + (flip ? (PET_W - 1 - dx) : dx), y + dy, color);
    }
}

/* 右侧阴影：每行最右像素 +1 / +2，亮度与时间数字的阴影一致 */
static void pet_shadow(int x, int y, const uint16_t *rows, uint8_t h,
                       uint8_t flip, const int8_t (*tail)[2], Color color)
{
    uint8_t old = g_maxGray;

    g_maxGray = CLOCK_SHADOW_GRAY;

    for (uint8_t r = 0; r < h; r++)
    {
        uint16_t bits = rows[r];
        int right = -1;

        if (bits)
        {
            if (flip)
            {
                for (uint8_t c = 0; c < PET_W; c++)
                    if (bits & (1u << c))
                    {
                        right = x + (PET_W - 1 - c);
                        break;
                    }
            }
            else
            {
                for (int c = PET_W - 1; c >= 0; c--)
                    if (bits & (1u << c))
                    {
                        right = x + c;
                        break;
                    }
            }
        }

        if (tail != NULL)
        {
            for (int i = 0; i < PET_TAIL; i++)
            {
                int sx;

                if (tail[i][1] != (int8_t)r)
                    continue;

                sx = x + (flip ? (PET_W - 1 - tail[i][0]) : tail[i][0]);
                if (sx > right)
                    right = sx;
            }
        }

        if (right >= 0)
        {
            draw_point(right + 1, y + r, color);
            draw_point(right + 2, y + r, color);
        }
    }

    g_maxGray = old;
}

/*
 * 触发一次出场（分钟跳变时由时钟界面调用）：挑一个动作演一遍，演完自动隐藏。
 *   allow_right : 0 = 右半边被日期占用，只在左侧活动
 */
void clock_pet_trigger(uint32_t now, uint8_t allow_right)
{
    pet_pick(now, allow_right);
    pet_active = 1;
}

/*
 * 每帧调用一次（时钟界面里，画完时间之后、送屏之前）。没在出场就什么都不画。
 *   right_limit : 数字区右边界（像素），宠物右侧活动范围从这里往右
 *   allow_right : 0 = 右半边被日期占用，只在左侧活动
 */
void clock_pet_draw(uint32_t now, int right_limit, uint8_t allow_right, Color color)
{
    const uint16_t *spr = pet_sit;
    const int8_t (*tail)[2] = pet_tail[1];
    uint8_t h = PET_H;
    uint8_t flip;
    int x, y;
    uint32_t t;

    if (!pet_active)
        return;

    if ((uint32_t)(now - g_pet.t0) >= g_pet.dur)
    {
        /* 演完就躲起来，等下一次分钟跳变 */
        pet_active = 0;
        return;
    }

    if (!allow_right && g_pet.home)
        g_pet.home = 0;

    t = now - g_pet.t0;
    flip = g_pet.home ? 1 : 0;
    x = pet_base_x(g_pet.home, right_limit);
    y = PET_GROUND;

    switch (g_pet.act)
    {
    case PET_ACT_SIT:
    case PET_ACT_WAG:
    {
        uint32_t speed = (g_pet.act == PET_ACT_WAG) ? 150u : 420u;
        uint32_t bt = t % 3000u;

        tail = pet_tail[(t / speed) % 3u];

        /* 偶尔眨眼（避开动作刚开始那一下） */
        if (g_pet.act == PET_ACT_SIT && bt > 2600u && bt < 2740u)
            spr = pet_sit_blink;

        break;
    }
    case PET_ACT_JUMP:
    {
        const uint32_t ant = 140u, hop = 340u, hops = 3u;

        if (t < ant)
        {
            /* 起跳前压一下 */
            spr = pet_lie;
            h = 5;
            y = PET_LIE_Y;
            tail = pet_tail_lie[0];
        }
        else if (t - ant < hops * hop)
        {
            uint32_t p = ((t - ant) % hop) * 1000u / hop;
            uint32_t up = 5u * 4u * p * (1000u - p) / 1000000u;

            y = PET_GROUND - (int)up;
            tail = pet_tail[(p < 500u) ? 0 : 2];
        }
        else
        {
            /* 三下跳完，站定等一下再消失 */
            tail = pet_tail[(t / 200u) % 3u];
        }

        break;
    }
    case PET_ACT_LIE:
        /* 收尾先站起来，别躺着凭空消失 */
        if (t + 500u >= g_pet.dur)
        {
            tail = pet_tail[(t / 200u) % 3u];
            break;
        }

        spr = pet_lie;
        h = 5;
        y = PET_LIE_Y;
        tail = pet_tail_lie[(t / 220u) % 3u];
        break;
    case PET_ACT_PEEK_L:
    case PET_ACT_PEEK_R:
    {
        /* 从屏幕边缘滑进来，只露 5 像素宽的脑袋 */
        uint32_t p = t * 1000u / g_pet.dur;
        uint32_t vis;

        if (p < 280u)
            vis = p * 1000u / 280u;
        else if (p < 720u)
            vis = 1000u;
        else
            vis = (1000u - p) * 1000u / 280u;

        y = PET_GROUND;
        tail = NULL;

        if (g_pet.act == PET_ACT_PEEK_L)
        {
            x = -10 + (int)(vis * 5u / 1000u);
            flip = 0;
        }
        else
        {
            x = MAX_X - (int)(vis * 5u / 1000u);
            flip = 1;
        }

        break;
    }
    case PET_ACT_DROP:
    default:
    {
        const uint32_t peek_ms = 520u, fall_ms = 430u, bounce_ms = 170u;

        if (t < peek_ms)
        {
            /* 先从顶部垂下来一小截 */
            uint32_t a = (t < 330u) ? t : 330u;

            y = -10 + (int)(4u * a / 330u);
            tail = pet_tail[(t / 220u) % 3u];
        }
        else if (t < peek_ms + fall_ms)
        {
            /* 自由落体 */
            uint32_t p = (t - peek_ms) * 1000u / fall_ms;

            y = -6 + (int)((PET_GROUND + 6) * p * p / 1000000u);
            tail = pet_tail[0];
        }
        else if (t < peek_ms + fall_ms + bounce_ms)
        {
            /* 落地弹一下 */
            uint32_t p = (t - peek_ms - fall_ms) * 1000u / bounce_ms;

            y = PET_GROUND - (int)(4u * 4u * p * (1000u - p) / 1000000u);
            tail = pet_tail[2];
        }
        else
        {
            uint32_t e = t - peek_ms - fall_ms - bounce_ms;

            if (e < 520u)
            {
                spr = pet_lie;
                h = 5;
                y = PET_LIE_Y;
                tail = pet_tail_lie[0];
            }
            else
            {
                tail = pet_tail[(t / 200u) % 3u];
            }
        }

        break;
    }
    }

    pet_shadow(x, y, spr, h, flip, tail, color);
    pet_blit(x, y, spr, h, flip, color);

    if (tail != NULL)
        pet_tail_blit(x, y, tail, flip, color);
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

/* ============================================================
 * 换关 / 换主题过渡：冲击波、扫描线刷屏
 * 两者都直接改显存（display_buffer 低 4 位是蓝、高 4 位是红），
 * 不需要额外缓冲区，快照复用碎裂动画的 g_shatterSnap。
 * ============================================================ */

/* 灰阶减半（每个颜色通道单独折一半） */
static uint8_t wipe_dim(uint8_t v)
{
    return (uint8_t)((v >> 1) & 0x77);
}

/* 按 Q8 系数压暗（k = 256 保持原样，128 减半） */
static uint8_t wipe_scale(uint8_t v, uint32_t k)
{
    uint8_t r = (uint8_t)((((v >> 4) & 0x0F) * k) >> 8);
    uint8_t b = (uint8_t)(((v & 0x0F) * k) >> 8);

    return (uint8_t)((r << 4) | b);
}

/* ---- 冲击波 ---- */
#define SHOCK_RING_PX 3 /* 白环厚度 */
#define SHOCK_MAX_R 62  /* 覆盖整屏的半径（对角线一半约 57） */
#define SHOCK_OUT_MS 320
#define SHOCK_IN_MS 250
#define SHOCK_BLACK_MS 50
#define SHOCK_STEP_MS 16

/*
 * 从快照铺一帧冲击波：
 *   reveal = 0  环内已经清空、环外按 k 压暗（旧画面被炸掉）
 *   reveal = 1  环内已经是新画面、环外全黑（新画面涌出来）
 */
static void shock_frame(const uint8_t *snap, int32_t r, uint8_t reveal, uint32_t k)
{
    uint8_t *buf = mn11236_get_buffer();
    const int32_t cx = VFD_WIDTH / 2;
    const int32_t cy = VFD_HEIGHT / 2;
    int32_t rin2 = (r > SHOCK_RING_PX) ? (r - SHOCK_RING_PX) * (r - SHOCK_RING_PX) : 0;
    int32_t rout2 = r * r;

    for (int x = 0; x < VFD_WIDTH; x++)
    {
        int32_t dx = x - cx;
        uint8_t *col = &buf[x * VFD_HEIGHT];
        const uint8_t *scol = &snap[x * VFD_HEIGHT];

        for (int y = 0; y < VFD_HEIGHT; y++)
        {
            int32_t dy = y - cy;
            int32_t d2 = dx * dx + dy * dy;

            if (d2 < rin2)
                col[y] = reveal ? scol[y] : 0;
            else if (d2 < rout2)
                col[y] = 0xFF; /* 白环 */
            else
                col[y] = reveal ? 0 : wipe_scale(scol[y], k);
        }
    }
}

/* 跑一段冲击波，半径按 smoothstep 从中心推到整屏 */
static void shock_run(const uint8_t *snap, uint16_t total_ms, uint8_t reveal)
{
    for (uint32_t t = 0; t <= total_ms; t += SHOCK_STEP_MS)
    {
        uint32_t p = t * 256u / total_ms;
        uint32_t e = p * p * (768u - 2u * p) / 65536u; /* smoothstep(0..1) 的 Q8 版 */
        int32_t r = 1 + (int32_t)(e * SHOCK_MAX_R / 256u);
        uint32_t k = 256u - e / 2u; /* 环外从原亮度渐变到一半 */

        mn11236_clear_buffers();
        shock_frame(snap, r, reveal, k);
        mn11236_send_buffer();
        osDelay(SHOCK_STEP_MS);
    }
}

/* 通用冲击波过渡，拆成三段，中间那段是纯黑屏：
 *   shock_out()     旧画面被炸掉，屏幕全黑并停住（黑场开始）
 *   shock_prepare() 黑场里把新画面画好、拍快照（屏幕还是黑的）
 *   shock_in()      新画面从中心涌出来，收尾留下完整画面
 * 之所以拆开，是为了让调用方能在黑场里插入阻塞操作（比如写 FLASH 存配置），
 * 那几毫秒藏在黑屏里，用户看不到卡顿。 */
static uint32_t shock_t0;

static void shock_out(void)
{
    uint8_t *buf = mn11236_get_buffer();

    shock_t0 = HAL_GetTick();

    memcpy(g_shatterSnap, buf, sizeof(g_shatterSnap)); /* 旧画面最后一帧 */

    shock_run((const uint8_t *)g_shatterSnap, SHOCK_OUT_MS, 0);

    mn11236_clear_buffers();
    mn11236_send_buffer(); /* 全黑，停在这里 */
    osDelay(SHOCK_BLACK_MS);
}

static void shock_prepare(void (*build)(void))
{
    uint8_t *buf = mn11236_get_buffer();

    mn11236_clear_buffers();
    build(); /* 黑屏里画新画面 */

    memcpy(g_shatterSnap, buf, sizeof(g_shatterSnap)); /* 新画面快照 */
}

static void shock_in(void)
{
    uint8_t *buf = mn11236_get_buffer();

    shock_run((const uint8_t *)g_shatterSnap, SHOCK_IN_MS, 1);

    /* 收尾：显存里留下完整的新画面 */
    memcpy(buf, g_shatterSnap, sizeof(g_shatterSnap));
    mn11236_send_buffer();

    clock_anim_pause(HAL_GetTick() - shock_t0);
}

/* 坦克大战换关 */
static void tnk_build_round(void)
{
    tnk_new_round();
    tnk_render();
}

static void tnk_shockwave(void)
{
    shock_out();
    shock_prepare(tnk_build_round);
    shock_in();
}

/* 游戏之间切换：和坦克换关同一个冲击波
 * 黑场里重开新游戏并画好第一帧，涌出来的时候就已经是下一局了 */
static uint8_t game_build_id;

static void game_build_frame(void)
{
    game_restart(game_build_id);

    if (game_build_id == 0)
        refrashSnake();
    else if (game_build_id == 1)
        refrashTetris();
    else
        refrashTanks();
}

/* 第一段：冲击波把旧游戏炸掉，留下一块黑屏（黑场里可以存档） */
void game_switch_anim_out(uint8_t id)
{
    game_build_id = id;
    shock_out();
}

/* 第三段：黑场结束前把新游戏画好，再从中心涌出来 */
void game_switch_anim_in(void)
{
    shock_prepare(game_build_frame);
    shock_in();
}


/* ---- 扫描线刷屏（时钟换主题） ---- */
#define WIPE_OUT_MS 130
#define WIPE_IN_MS 180
#define WIPE_BLACK_MS 45 /* 黑场停留，调用方在这段时间里存档 */
#define WIPE_STEP_MS 10
#define WIPE_BAND 2 /* 亮线高度（行） */

static uint8_t wipe_in_active;
static uint32_t wipe_in_t0;

/* 扫描线刷掉：线以上直接黑、线本身亮白、线下面压暗 */
static void wipe_mask_out(int32_t line)
{
    uint8_t *buf = mn11236_get_buffer();

    for (int y = 0; y < VFD_HEIGHT; y++)
    {
        int mode = (y < line - 1) ? 0 : ((y < line + WIPE_BAND) ? 1 : 2);

        for (int x = 0; x < VFD_WIDTH; x++)
        {
            uint8_t *p = &buf[x * VFD_HEIGHT + y];

            if (mode == 0)
                *p = 0;
            else if (mode == 1)
                *p = 0xFF;
            else
                *p = wipe_dim(*p);
        }
    }
}

/* 阻塞：把当前画面用扫描线刷掉，结束时全黑并停一小会儿 */
void clock_wipe_scan_out(void)
{
    uint8_t *buf = mn11236_get_buffer();
    uint32_t t_begin = HAL_GetTick();

    memcpy(g_shatterSnap, buf, sizeof(g_shatterSnap));

    for (uint32_t t = 0; t <= WIPE_OUT_MS; t += WIPE_STEP_MS)
    {
        int32_t line = -2 + (int32_t)(t * (VFD_HEIGHT + 4) / WIPE_OUT_MS);

        /* 每帧都从快照重画，压暗不会叠加 */
        memcpy(buf, g_shatterSnap, sizeof(g_shatterSnap));
        wipe_mask_out(line);
        mn11236_send_buffer();
        osDelay(WIPE_STEP_MS);
    }

    mn11236_clear_buffers();
    mn11236_send_buffer();
    osDelay(WIPE_BLACK_MS);

    /* 过渡期间后台动画停表 */
    clock_anim_pause(HAL_GetTick() - t_begin);
}

/* 黑场结束后调用：新主题的画面从扫描线下面长出来 */
void clock_wipe_scan_begin_in(void)
{
    wipe_in_active = 1;
    wipe_in_t0 = HAL_GetTick();
}

/* 每帧画完新主题、送屏之前调用 */
void clock_wipe_scan_in(void)
{
    uint8_t *buf = mn11236_get_buffer();
    uint32_t el;
    int32_t line;

    if (!wipe_in_active)
        return;

    el = HAL_GetTick() - wipe_in_t0;

    if (el >= WIPE_IN_MS)
    {
        wipe_in_active = 0;
        return;
    }

    line = -4 + (int32_t)(el * (VFD_HEIGHT + 8) / WIPE_IN_MS);

    for (int y = 0; y < VFD_HEIGHT; y++)
    {
        int mode = (y < line - 4) ? 0 : ((y < line + WIPE_BAND) ? 1 : 2);

        if (mode == 0)
            continue; /* 线上面已经是新画面，原样保留 */

        for (int x = 0; x < VFD_WIDTH; x++)
        {
            uint8_t *p = &buf[x * VFD_HEIGHT + y];

            if (mode == 1)
                *p = 0xFF;
            else
                *p = 0;
        }
    }
}

/* ============================================================
 *  贪吃蛇（格子宽度固定 SNAKE_FIXED_WIDTH = 3，棋盘 37 x 6）
 *
 *  相比老实现：
 *   1. 走格改成时间驱动，不再每帧走一格（原来 20 格/秒，眼睛跟不上）；
 *   2. AI 先用 BFS 算到食物的真实距离，再对每个候选方向做连通块
 *      flood fill，空间不够自己身长就不走，基本不会把自己困死；
 *   3. 食物生成避开蛇身并带重试上限，亮度做呼吸；
 *   4. 死亡动画、渐隐、重生都保留，重生时整盘重新铺，不会留残影。
 * ============================================================ */

#define SNAKE_W SNAKE_FIXED_WIDTH
#define SNAKE_COLS (MAX_X / SNAKE_W)          /* 37 */
#define SNAKE_ROWS (MAX_Y / SNAKE_W)          /* 6  */
#define SNAKE_CELLS (SNAKE_COLS * SNAKE_ROWS) /* 222 */
#define SNAKE_MAX_LEN 60
#define SNAKE_STEP_MS 120 /* 每走一格的时间 */
#define SNAKE_FOOD_UNREACH 250

#define SNAKE_HEAD White
#define SNAKE_BODY Cyan
#define SNAKE_FOOD Red

typedef struct
{
    int16_t x, y;
} SnakeSegment;

typedef struct
{
    SnakeSegment body[SNAKE_MAX_LEN];
    int length;
    int direction; /* 0=上 1=右 2=下 3=左 */
} Snake_t;

typedef struct
{
    int16_t x, y;
} Food_t;

typedef enum
{
    SNAKE_ALIVE = 0,
    SNAKE_DYING,    /* 从头到尾变红 */
    SNAKE_DEAD_WAIT /* 全红之后渐隐，再重生 */
} SnakeState_t;

static SnakeState_t snakeState = SNAKE_ALIVE;
static int deathIndex = 0;
static int deadWaitCnt = 0;
static uint32_t snake_last_step = 0;
static uint32_t snake_now = 0;
/* 平滑移动：一步走一格（SNAKE_W 像素），拆成 SNAKE_W 帧，每帧挪 1 像素。
 * snake_frac = 本步已经走了几帧（0 .. SNAKE_W-1） */
static int snake_frac = 0;
/* 这一口食物已经吃进逻辑里了，但画面上蛇头还没压上去：
 * 先留在原地不动，等下一步再换新位置，免得食物比蛇头早消失 */
static uint8_t snake_food_eaten = 0;

static Snake_t snake;
static Food_t food;

static uint8_t snake_vis[SNAKE_CELLS];
static uint8_t snake_dist[SNAKE_CELLS];
static uint8_t snake_q[SNAKE_CELLS];
static uint8_t snake_gen = 0;

static const int8_t snake_dx[4] = {0, 1, 0, -1};
static const int8_t snake_dy[4] = {-1, 0, 1, 0};

/* ---------------- 基础工具 ---------------- */

/* 画一个格子（SNAKE_W x SNAKE_W） */
static void snake_block(int x, int y, Color c, uint8_t gray)
{
    int px = x * SNAKE_W;
    int py = y * SNAKE_W;

    for (int dx = 0; dx < SNAKE_W; dx++)
        for (int dy = 0; dy < SNAKE_W; dy++)
            draw_point_gray(px + dx, py + dy, c, gray);
}

static int snake_inside(int x, int y)
{
    return x >= 0 && x < SNAKE_COLS && y >= 0 && y < SNAKE_ROWS;
}

/* 某格是否被蛇身占着；ignore_tail=1 时忽略最后一节（下一步它就挪走了） */
static int snake_body_at(const Snake_t *s, int x, int y, int ignore_tail)
{
    int n = s->length - (ignore_tail ? 1 : 0);

    for (int i = 0; i < n; i++)
        if (s->body[i].x == x && s->body[i].y == y)
            return 1;

    return 0;
}

/* 从 (sx, sy) 出发能走到多少格（蛇身当墙，尾巴算空） */
static int snake_space(const Snake_t *s, int sx, int sy)
{
    int qh = 0, qt = 0, n = 0;

    if (++snake_gen == 0)
    {
        memset(snake_vis, 0, sizeof(snake_vis));
        snake_gen = 1;
    }

    snake_q[qt++] = (uint8_t)(sy * SNAKE_COLS + sx);
    snake_vis[sy * SNAKE_COLS + sx] = snake_gen;

    while (qh < qt)
    {
        int idx = snake_q[qh++];
        int x = idx % SNAKE_COLS;
        int y = idx / SNAKE_COLS;

        n++;

        for (int d = 0; d < 4; d++)
        {
            int nx = x + snake_dx[d];
            int ny = y + snake_dy[d];
            int ni;

            if (!snake_inside(nx, ny))
                continue;

            ni = ny * SNAKE_COLS + nx;

            if (snake_vis[ni] == snake_gen || snake_body_at(s, nx, ny, 1))
                continue;

            snake_vis[ni] = snake_gen;
            snake_q[qt++] = (uint8_t)ni;
        }
    }

    return n;
}

/* 从食物倒着 BFS，得到每格到食物的最短步数；走不到就是 SNAKE_FOOD_UNREACH */
static void snake_dist_map(const Snake_t *s, const Food_t *f)
{
    int qh = 0, qt = 0;

    memset(snake_dist, SNAKE_FOOD_UNREACH, sizeof(snake_dist));

    if (!snake_inside(f->x, f->y))
        return;

    snake_dist[f->y * SNAKE_COLS + f->x] = 0;
    snake_q[qt++] = (uint8_t)(f->y * SNAKE_COLS + f->x);

    while (qh < qt)
    {
        int idx = snake_q[qh++];
        int x = idx % SNAKE_COLS;
        int y = idx / SNAKE_COLS;
        uint8_t nd = (uint8_t)(snake_dist[idx] + 1);

        if (nd >= SNAKE_FOOD_UNREACH)
            continue;

        for (int d = 0; d < 4; d++)
        {
            int nx = x + snake_dx[d];
            int ny = y + snake_dy[d];
            int ni;

            if (!snake_inside(nx, ny))
                continue;

            ni = ny * SNAKE_COLS + nx;

            if (snake_dist[ni] != SNAKE_FOOD_UNREACH || snake_body_at(s, nx, ny, 1))
                continue;

            snake_dist[ni] = nd;
            snake_q[qt++] = (uint8_t)ni;
        }
    }
}

/* ---------------- 走一步 ---------------- */

static void snake_ai(Snake_t *s, const Food_t *f)
{
    int hx = s->body[0].x;
    int hy = s->body[0].y;
    int need = s->length + 2;
    int safe_dir = -1;
    int safe_dist = SNAKE_FOOD_UNREACH;
    int free_dir = -1;
    int free_space = -1;

    snake_dist_map(s, f);

    /* 先直行，再右转、左转，掉头直接跳过 */
    for (int k = 0; k < 4; k++)
    {
        int d = (s->direction + k) & 3;
        int nx, ny, space, dist;

        if (d == ((s->direction + 2) & 3))
            continue;

        nx = hx + snake_dx[d];
        ny = hy + snake_dy[d];

        if (!snake_inside(nx, ny) || snake_body_at(s, nx, ny, 1))
            continue;

        space = snake_space(s, nx, ny);

        if (space < need)
        {
            /* 走进去会把自己困住，只当备胎 */
            if (space > free_space)
            {
                free_space = space;
                free_dir = d;
            }
            continue;
        }

        dist = snake_dist[ny * SNAKE_COLS + nx];

        if (safe_dir < 0 || dist < safe_dist)
        {
            safe_dir = d;
            safe_dist = dist;
        }
    }

    if (safe_dir >= 0)
        s->direction = safe_dir;
    else if (free_dir >= 0)
        s->direction = free_dir;
}

static void snake_move(Snake_t *s, int grow)
{
    /* 吃到食物时把旧尾巴留在原地，这样长度真的多一节
     * （老写法是复制最后一节，会有两节叠在同一格里） */
    if (grow && s->length < SNAKE_MAX_LEN)
    {
        s->body[s->length] = s->body[s->length - 1];
        s->length++;
    }

    for (int i = s->length - 1; i > 0; i--)
        s->body[i] = s->body[i - 1];

    s->body[0].x = (int16_t)(s->body[0].x + snake_dx[s->direction]);
    s->body[0].y = (int16_t)(s->body[0].y + snake_dy[s->direction]);
}

/* 撞墙或撞到自己（最后一节不算，它下一步就挪走了） */
static int snake_collision(const Snake_t *s)
{
    int x = s->body[0].x;
    int y = s->body[0].y;

    if (!snake_inside(x, y))
        return 1;

    for (int i = 1; i < s->length - 1; i++)
        if (s->body[i].x == x && s->body[i].y == y)
            return 1;

    return 0;
}

static void snake_place_food(const Snake_t *s, Food_t *f)
{
    int guard = 0;

    do
    {
        f->x = (int16_t)(rand() % SNAKE_COLS);
        f->y = (int16_t)(rand() % SNAKE_ROWS);
    } while (snake_body_at(s, f->x, f->y, 0) && ++guard < 512);
}

static void snake_spawn(Snake_t *s)
{
    s->length = 4;
    s->direction = 1;

    /* 兜底：横着摆在左上角 */
    for (int i = 0; i < s->length; i++)
    {
        s->body[i].x = (int16_t)(3 - i);
        s->body[i].y = 0;
    }

    for (int tries = 0; tries < 64; tries++)
    {
        int d = rand() & 3;
        int hx = rand() % SNAKE_COLS;
        int hy = rand() % SNAKE_ROWS;
        int ok = 1;

        for (int i = 0; i < s->length; i++)
        {
            int x = hx - snake_dx[d] * i;
            int y = hy - snake_dy[d] * i;

            if (!snake_inside(x, y))
            {
                ok = 0;
                break;
            }

            s->body[i].x = (int16_t)x;
            s->body[i].y = (int16_t)y;
        }

        if (ok)
        {
            s->direction = d;
            return;
        }
    }
}

/* ---------------- 渲染 ---------------- */

/* 3x3 的格子太小，纯色方块看着就是一根"香肠"，这里补几处细节：
 *   1. 一格横着切成三条：暗红（背）/ 白（亮脊）/ 蓝（腹）；
 *      竖着走时同样是左列暗红、中列白、右列蓝；
 *   2. 亮脊用红蓝双通道一起亮 → 白色高光，像圆管上的一条反光；
 *   3. 蛇头切掉朝前的两个角 → 有个尖鼻子；
 *   4. 尾巴沿身体方向缩成 2 格宽 → 收尖；
 *   5. 食物画成十字宝石，和蛇身方块区分开。
 *
 * 注意：下面直接用「带号 0/1/2 = 格内坐标 0/1/2」来分条，
 *       所以依赖 SNAKE_W == 3。 */

/* 这一节是横着的还是竖着的（看相邻那节在哪边） */
static int snake_seg_horizontal(int i)
{
    int j = (i + 1 < snake.length) ? i + 1 : i - 1;

    return snake.body[i].x != snake.body[j].x;
}

/* 一格身体里的第 band 条：0=暗红（背） 1=白（亮脊） 2=蓝（腹）
 * g 是这一格的亮度（靠头最亮，往尾巴渐暗） */
static void snake_band_pixel(int x, int y, int band, uint8_t g)
{
    if (band == 0)
    {
        /* 背部：红压暗 2 级 */
        draw_point_gray(x, y, Red, (g > 2) ? (uint8_t)(g - 2) : g);
    }
    else if (band == 1)
    {
        /* 亮脊：红蓝一起亮 → 白 */
        draw_point_gray(x, y, Red, g);
        draw_point_gray(x, y, Cyan, g);
    }
    else
    {
        /* 腹部：蓝 */
        draw_point_gray(x, y, Cyan, g);
    }
}

/* 一节身体：横着按行分条，竖着按列分条 */
static void snake_seg(int px, int py, int horizontal, uint8_t g)
{
    for (int dx = 0; dx < SNAKE_W; dx++)
        for (int dy = 0; dy < SNAKE_W; dy++)
        {
            int band = horizontal ? dy : dx;

            snake_band_pixel(px + dx, py + dy, band, g);
        }
}

/* 蛇头：朝前的两个角切掉，留个尖鼻子
 * px/py 是像素坐标（平滑移动时会带着不足一格的偏移） */
static void snake_head_block(int px, int py, int dir, uint8_t g)
{
    for (int dx = 0; dx < SNAKE_W; dx++)
        for (int dy = 0; dy < SNAKE_W; dy++)
        {
            int front;
            int corner = ((dx == 0 || dx == SNAKE_W - 1) &&
                          (dy == 0 || dy == SNAKE_W - 1));

            if (dir == 1)
                front = (dx == SNAKE_W - 1);
            else if (dir == 3)
                front = (dx == 0);
            else if (dir == 0)
                front = (dy == 0);
            else
                front = (dy == SNAKE_W - 1);

            if (front && corner)
                continue;

            draw_point_gray(px + dx, py + dy, SNAKE_HEAD, g);
        }
}

/* 尾巴：沿身体方向缩成 2 格宽 → 收尖；
 * 贴向身体那一侧对齐，和前一节之间不会留 1 像素的缝；同样是暗红 / 白 / 蓝三条 */
static void snake_tail_block(int px, int py, const SnakeSegment *tail,
                            const SnakeSegment *prev, uint8_t g)
{
    if (prev->x != tail->x)
    {
        /* 横着：宽 2、高 3 */
        int x0 = (prev->x > tail->x) ? 1 : 0;

        for (int dx = 0; dx < SNAKE_W - 1; dx++)
            for (int dy = 0; dy < SNAKE_W; dy++)
                snake_band_pixel(px + x0 + dx, py + dy, dy, g);
    }
    else
    {
        /* 竖着：宽 3、高 2 */
        int y0 = (prev->y > tail->y) ? 1 : 0;

        for (int dy = 0; dy < SNAKE_W - 1; dy++)
            for (int dx = 0; dx < SNAKE_W; dx++)
                snake_band_pixel(px + dx, py + y0 + dy, dx, g);
    }
}

/* 食物：十字宝石 + 呼吸亮度 */
static void snake_food_block(int cx, int cy, uint8_t g)
{
    int px = cx * SNAKE_W;
    int py = cy * SNAKE_W;
    int mid = SNAKE_W / 2;

    for (int dx = 0; dx < SNAKE_W; dx++)
        for (int dy = 0; dy < SNAKE_W; dy++)
        {
            if (dx != mid && dy != mid)
                continue;

            draw_point_gray(px + dx, py + dy, SNAKE_FOOD, g);
        }
}

static void snake_render(void)
{
    int len = snake.length;
    /* 平滑移动：整条蛇往身后错 (SNAKE_W-1-frac) 像素。
     * 这样每步的最后一帧正好落在格子上（蛇头刚好压住食物），
     * 下一帧从上一格 +1 像素接着走，帧与帧之间还是 1 像素。 */
    int back = (SNAKE_W - 1) - snake_frac;

    /* 食物先画：滑过去的时候会被蛇头盖掉，不会红十字压在蛇头上 */
    snake_food_block(food.x, food.y, ((snake_now / 260) & 1) ? 7 : 4);

    for (int i = 0; i < len; i++)
    {
        int t = (len > 1) ? (i * 100 / (len - 1)) : 100;
        int cx = snake.body[i].x;
        int cy = snake.body[i].y;
        int dx, dy;
        int px, py;
        uint8_t gb = (t < 40) ? 7 : (uint8_t)(7 - (t - 40) * 7 / 60);

        if (gb < 1)
            gb = 1;

        /* 每节都从「后一节现在的位置」滑到自己的位置，转弯处也是贴着弯走过去；
         * 尾巴后面没有节点了，按最后两节的方向往外推一格 */
        if (i + 1 < len)
        {
            dx = snake.body[i + 1].x - cx;
            dy = snake.body[i + 1].y - cy;
        }
        else
        {
            dx = cx - snake.body[len - 2].x;
            dy = cy - snake.body[len - 2].y;
        }

        px = cx * SNAKE_W + dx * back;
        py = cy * SNAKE_W + dy * back;

        if (i == 0)
        {
            /* 头：纯白 + 尖鼻子 */
            snake_head_block(px, py, snake.direction, 7);
        }
        else if (i == len - 1)
        {
            /* 尾巴：缩成 2 格宽收尖 */
            snake_tail_block(px, py, &snake.body[i], &snake.body[len - 2],
                             (gb > 2) ? gb : 2);
        }
        else
        {
            /* 暗红 / 白 / 蓝三条，亮度从头到尾渐变 */
            snake_seg(px, py, snake_seg_horizontal(i), gb);
        }
    }
}

/* 死亡：红色从头往尾巴扩散，扩散到的段红得最亮，往后渐暗 */
static void snake_draw_dying(void)
{
    int len = snake.length;

    for (int i = 0; i < len; i++)
    {
        if (i <= deathIndex)
        {
            int d = deathIndex - i;

            if (d > 7)
                d = 7;

            snake_block(snake.body[i].x, snake.body[i].y, Red, (uint8_t)(7 - d));
        }
        else
        {
            int t = (len > 1) ? (i * 100 / (len - 1)) : 100;
            uint8_t g;

            if (t < 50)
                g = 7;
            else
                g = (uint8_t)(7 - (t - 50) * 7 / 50);

            if (g < 1)
                g = 1;

            snake_block(snake.body[i].x, snake.body[i].y, SNAKE_BODY, g);
        }
    }
}

/* ---------------- 对外接口 ---------------- */

void snakeRestart(void)
{
    snakeState = SNAKE_ALIVE;
    deathIndex = 0;
    deadWaitCnt = 0;
    snake_last_step = 0;
    snake_frac = 0;
    snake_food_eaten = 0;

    snake_spawn(&snake);
    snake_place_food(&snake, &food);
}

void refrashSnake(void)
{
    uint32_t now = clock_anim_tick();

    snake_now = now;

    /* ---- 正在死亡：每帧往前推一段 ---- */
    if (snakeState == SNAKE_DYING)
    {
        snake_draw_dying();

        if (++deathIndex >= snake.length - 1)
        {
            snakeState = SNAKE_DEAD_WAIT;
            deadWaitCnt = 0;
        }
        return;
    }

    /* ---- 全红之后渐隐，然后重生 ---- */
    if (snakeState == SNAKE_DEAD_WAIT)
    {
        uint8_t g = (deadWaitCnt <= 7) ? (uint8_t)deadWaitCnt : 0;

        if (g > 0)
            for (int i = 0; i < snake.length; i++)
                snake_block(snake.body[i].x, snake.body[i].y, Red, g);

        if (++deadWaitCnt > 7)
            snakeRestart();

        return;
    }

    /* ---- 正常游戏：按固定节奏走格，帧率再高也不会加速 ---- */
    if (snake_last_step == 0 || now - snake_last_step > 1000)
        snake_last_step = now; /* 切回界面时不要一次补几十步 */

    if (now - snake_last_step >= SNAKE_STEP_MS)
    {
        int grow;

        snake_last_step += SNAKE_STEP_MS;

        /* 上一口：蛇头在画面上已经压住食物了，现在才把新食物放出来 */
        if (snake_food_eaten)
        {
            snake_food_eaten = 0;
            snake_place_food(&snake, &food);
        }

        snake_ai(&snake, &food);

        grow = (snake.body[0].x + snake_dx[snake.direction] == food.x &&
                snake.body[0].y + snake_dy[snake.direction] == food.y);

        snake_move(&snake, grow);

        if (grow)
            snake_food_eaten = 1;

        if (snake_collision(&snake))
        {
            snakeState = SNAKE_DYING;
            deathIndex = 0;
            return;
        }
    }

    /* 平滑：这一步已经走了多久 -> 走了几像素（0 / 1 / 2） */
    {
        uint32_t el = now - snake_last_step;
        int f = (int)(el * SNAKE_W / SNAKE_STEP_MS);

        snake_frac = (f > SNAKE_W - 1) ? (SNAKE_W - 1) : (f < 0 ? 0 : f);
    }

    snake_render();
}

/* 游戏合集：0=贪吃蛇 1=俄罗斯方块 2=坦克大战
 * 切换时重开一局，按 EXIT 后立刻能看到新游戏从头开始 */
void game_restart(uint8_t id)
{
    if (id == 0)
        snakeRestart();
    else if (id == 1)
        tet_inited = 0; /* 下一帧 refrashTetris() 会重建棋盘并重新开局 */
    else
        tanksRegen(); /* 换一张随机地图 */
}

#define BAR_COUNT MAX_X
#define BAR_MAX MAX_Y

FFTDisplay_t bars[BAR_COUNT];

/* 柱子 i -> 频点（16 倍定标：0x0010 = bin 1，0x07E0 = bin 126）
 * f(i) = 40Hz * (5000/40)^(i/111)，除以 39.06Hz/bin 后左移 4 位。
 * 换成对数刻度后低频不再挤在左边十几列。 */
static const uint16_t fft_bin_x16[BAR_COUNT] = {
    0x0010, 0x0011, 0x0012, 0x0013, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018,
    0x0019, 0x001A, 0x001C, 0x001D, 0x001E, 0x001F, 0x0021, 0x0022, 0x0024, 0x0025,
    0x0027, 0x0029, 0x002B, 0x002D, 0x002F, 0x0031, 0x0033, 0x0035, 0x0037, 0x003A,
    0x003C, 0x003F, 0x0042, 0x0045, 0x0048, 0x004B, 0x004E, 0x0052, 0x0056, 0x0059,
    0x005D, 0x0061, 0x0066, 0x006A, 0x006F, 0x0074, 0x0079, 0x007F, 0x0084, 0x008A,
    0x0090, 0x0097, 0x009D, 0x00A4, 0x00AC, 0x00B3, 0x00BB, 0x00C4, 0x00CC, 0x00D5,
    0x00DF, 0x00E9, 0x00F3, 0x00FE, 0x0109, 0x0115, 0x0121, 0x012E, 0x013C, 0x014A,
    0x0158, 0x0167, 0x0177, 0x0188, 0x019A, 0x01AC, 0x01BF, 0x01D3, 0x01E7, 0x01FD,
    0x0214, 0x022B, 0x0244, 0x025E, 0x0279, 0x0295, 0x02B2, 0x02D1, 0x02F1, 0x0313,
    0x0336, 0x035A, 0x0380, 0x03A8, 0x03D2, 0x03FD, 0x042B, 0x045A, 0x048B, 0x04BF,
    0x04F5, 0x052E, 0x0569, 0x05A6, 0x05E6, 0x062A, 0x0670, 0x06B9, 0x0705, 0x0755,
    0x07A9, 0x07E0,
};

/* ---------------- 风格私有缓冲 ----------------
 * 瀑布和粒子互斥，共用同一块 RAM（约 253 字节） */
#define FFT_WF_COLS (BAR_COUNT / 2)                  /* 56 列，每列画 2 像素宽 */
#define FFT_WF_ROWS MAX_Y                            /* 18 行历史 */
#define FFT_WF_ROW_BYTES ((FFT_WF_COLS * 2 + 7) / 8) /* 每行 14 字节，2bit/格 */
#define FFT_SPARK_MAX 20

static union
{
    struct
    {
        uint8_t buf[FFT_WF_ROW_BYTES * FFT_WF_ROWS];
    } wf;
    struct
    {
        int16_t x[FFT_SPARK_MAX], y[FFT_SPARK_MAX];  /* 1/4 像素 */
        int8_t vx[FFT_SPARK_MAX], vy[FFT_SPARK_MAX]; /* 1/4 像素/帧 */
        uint8_t life[FFT_SPARK_MAX];
    } sp;
} fft_ram;

static uint8_t fft_spark_n = 0;
static struct
{
    float needle;
    float peak;
    uint8_t hold;
} fft_vu;

/* VU 刻度弧：相对 (cx, cy) 的偏移，-62° ~ +62° 分 21 档 */
static const int8_t fft_vu_arc[21][2] = {
    {-13, -7}, {-13, -9}, {-12, -10}, {-10, -11}, {-9, -12},
    {-8, -13}, {-6, -14}, {-5, -14}, {-3, -15}, {-2, -15},
    {0, -15}, {2, -15}, {3, -15}, {5, -14}, {6, -14},
    {8, -13}, {9, -12}, {10, -11}, {12, -10}, {13, -9},
    {13, -7},
};

/* 切风格时清一次，免得残留上一种风格的粒子/瀑布 */
void fft_style_reset(void)
{
    memset(bars, 0, sizeof(bars));
    memset(&fft_ram, 0, sizeof(fft_ram));
    fft_spark_n = 0;
    fft_vu.needle = 0.0f;
    fft_vu.peak = 0.0f;
    fft_vu.hold = 0;
}

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

/* 柱身亮度：越靠柱顶越亮（2~7） */
static inline uint8_t fft_lv(int j, int h)
{
    int v = 2 + (j + 1) * 5 / h;

    return (uint8_t)(v > 7 ? 7 : v);
}

/* ① 经典柱状：柱身渐变 + 柱顶高光 + 峰值点 */
static void fft_draw_bars(Color bar, Color dot)
{
    for (int i = 0; i < BAR_COUNT; i++)
    {
        int h = (int)bars[i].output;

        for (int j = 0; j < h; j++)
            draw_point_gray(i, MAX_Y - 1 - j, bar, fft_lv(j, h));

        if (h > 0)
            draw_point(i, MAX_Y - h, bar);

        draw_point(i, MAX_Y - 1 - bars[i].maximum, dot);
    }
}

/* ② 左右镜像：只用 56 段，左右完全对称 */
static void fft_draw_mirror(Color bar, Color dot)
{
    const int half = BAR_COUNT / 2;

    for (int i = 0; i < half; i++)
    {
        int h = (int)bars[i].output;

        for (int j = 0; j < h; j++)
        {
            uint8_t lv = fft_lv(j, h);

            draw_point_gray(i, MAX_Y - 1 - j, bar, lv);
            draw_point_gray(BAR_COUNT - 1 - i, MAX_Y - 1 - j, bar, lv);
        }

        if (h > 0)
        {
            draw_point(i, MAX_Y - h, bar);
            draw_point(BAR_COUNT - 1 - i, MAX_Y - h, bar);
        }

        draw_point(i, MAX_Y - 1 - bars[i].maximum, dot);
        draw_point(BAR_COUNT - 1 - i, MAX_Y - 1 - bars[i].maximum, dot);
    }
}

/* ③ 包络曲线：频谱连成线，线下渐变填充 */
static void fft_draw_curve(Color bar, Color dot)
{
    int y[BAR_COUNT];

    for (int i = 0; i < BAR_COUNT; i++)
        y[i] = MAX_Y - 1 - (int)(bars[i].output + 0.5f);

    /* 线下填充 */
    for (int i = 0; i < BAR_COUNT; i++)
        for (int yy = y[i] + 1; yy < MAX_Y; yy++)
        {
            int d = yy - y[i];

            draw_point_gray(i, yy, bar, d <= 2 ? 3 : (d <= 5 ? 2 : 1));
        }

    /* 曲线本体（竖直方向补齐，避免断线） */
    for (int i = 0; i < BAR_COUNT; i++)
    {
        draw_point(i, y[i], bar);

        if (i + 1 < BAR_COUNT)
        {
            int a = y[i];
            int b = y[i + 1];
            int step = (b > a) ? 1 : -1;

            for (int yy = a; yy != b + step; yy += step)
                draw_point(i, yy, bar);
        }
    }

    /* 局部峰值点 */
    for (int i = 1; i < BAR_COUNT - 1; i++)
        if (bars[i].output > 9.0f &&
            bars[i].output >= bars[i - 1].output &&
            bars[i].output > bars[i + 1].output)
            draw_point(i, y[i] - 1, dot);
}

/* ④ 点阵柱：隔行点亮，LED 矩阵感 */
static void fft_draw_dots(Color bar, Color dot)
{
    for (int i = 0; i < BAR_COUNT; i++)
    {
        int h = (int)bars[i].output;

        for (int j = 0; j < h; j += 2)
            draw_point_gray(i, MAX_Y - 1 - j, bar, fft_lv(j, h));

        if (h > 0)
            draw_point(i, MAX_Y - h, bar);

        draw_point(i, MAX_Y - 1 - bars[i].maximum, dot);
    }
}

/* ⑤ 瀑布声谱：每 55ms 压一行历史，2bit/格 */
static void fft_wf_push(void)
{
    uint8_t *buf = fft_ram.wf.buf;
    uint8_t *row;

    memmove(buf, buf + FFT_WF_ROW_BYTES,
            FFT_WF_ROW_BYTES * (FFT_WF_ROWS - 1));

    row = buf + FFT_WF_ROW_BYTES * (FFT_WF_ROWS - 1);
    memset(row, 0, FFT_WF_ROW_BYTES);

    for (int c = 0; c < FFT_WF_COLS; c++)
    {
        float v = bars[c * 2].output;
        uint8_t lv;

        if (bars[c * 2 + 1].output > v)
            v = bars[c * 2 + 1].output;

        /* 3 档亮度；只有接近柱顶的强频点才用强调色，避免整屏泛红 */
        lv = (v < 1.0f) ? 0 : (v < 6.0f) ? 1 : (v < 14.0f) ? 2 : 3;
        row[c >> 2] |= (uint8_t)(lv << ((c & 3) * 2));
    }
}

static void fft_draw_waterfall(Color bar, Color dot)
{
    static uint32_t last = 0;
    uint32_t now = clock_anim_tick();

    if (now - last >= 55)
    {
        last = now;
        fft_wf_push();
    }

    for (int r = 0; r < FFT_WF_ROWS; r++)
    {
        const uint8_t *row = fft_ram.wf.buf + r * FFT_WF_ROW_BYTES;

        for (int c = 0; c < FFT_WF_COLS; c++)
        {
            uint8_t lv = (row[c >> 2] >> ((c & 3) * 2)) & 3;

            if (!lv)
                continue;

            if (lv == 3)
            {
                draw_point(c * 2, r, dot);
                draw_point(c * 2 + 1, r, dot);
            }
            else
            {
                draw_point_gray(c * 2, r, bar, lv == 2 ? 5 : 2);
                draw_point_gray(c * 2 + 1, r, bar, lv == 2 ? 5 : 2);
            }
        }
    }
}

/* ⑥ 节拍火花：柱状 + 峰值迸出的粒子 */
static void fft_draw_spark(Color bar, Color dot)
{
    /* 柱身压低一档，给粒子留亮度 */
    for (int i = 0; i < BAR_COUNT; i++)
    {
        int h = (int)bars[i].output;

        for (int j = 0; j < h; j++)
        {
            uint8_t lv = fft_lv(j, h);

            draw_point_gray(i, MAX_Y - 1 - j, bar, lv > 5 ? 5 : lv);
        }
    }

    /* 冲出新峰值 -> 迸一个粒子（remain==10 只在刚刷新峰值那一帧成立） */
    for (int i = 0; i < BAR_COUNT; i++)
    {
        uint8_t k;

        if (bars[i].remain != 10 || bars[i].output < 7.0f)
            continue;
        if (fft_spark_n >= FFT_SPARK_MAX)
            break;

        k = fft_spark_n++;
        fft_ram.sp.x[k] = (int16_t)(i * 4 + 2);
        fft_ram.sp.y[k] = (int16_t)((MAX_Y - 1 - (int)bars[i].output) * 4);
        fft_ram.sp.vx[k] = (int8_t)(((i & 1) ? 1 : -1) * (2 + (i % 3)));
        fft_ram.sp.vy[k] = (int8_t)(-8 - (i % 5) * 2);
        fft_ram.sp.life[k] = 70;
    }

    /* 粒子：重力 + 落地回弹 */
    for (int k = 0; k < fft_spark_n;)
    {
        int dead = 0;

        fft_ram.sp.vy[k] += 3;
        fft_ram.sp.x[k] += fft_ram.sp.vx[k];
        fft_ram.sp.y[k] += fft_ram.sp.vy[k];

        if (fft_ram.sp.y[k] >= (MAX_Y - 1) * 4)
        {
            fft_ram.sp.y[k] = (MAX_Y - 1) * 4;
            fft_ram.sp.vy[k] = (int8_t)(-fft_ram.sp.vy[k] * 2 / 3);
            if (fft_ram.sp.vy[k] > -4)
                dead = 1;
        }

        if (fft_ram.sp.life[k] > 0 && --fft_ram.sp.life[k] == 0)
            dead = 1;

        if (dead)
        {
            fft_spark_n--;
            fft_ram.sp.x[k] = fft_ram.sp.x[fft_spark_n];
            fft_ram.sp.y[k] = fft_ram.sp.y[fft_spark_n];
            fft_ram.sp.vx[k] = fft_ram.sp.vx[fft_spark_n];
            fft_ram.sp.vy[k] = fft_ram.sp.vy[fft_spark_n];
            fft_ram.sp.life[k] = fft_ram.sp.life[fft_spark_n];
            continue;
        }

        draw_point(fft_ram.sp.x[k] >> 2, fft_ram.sp.y[k] >> 2, dot);
        k++;
    }
}

/* ⑦ VU 指针表：弧形刻度 + 阻尼指针 + 峰值记忆 */
static void fft_draw_vu(Color bar, Color dot)
{
    const int cx = BAR_COUNT / 2, cy = MAX_Y + 1;
    float sum = 0.0f, wsum = 0.0f;
    float lv;

    for (int i = 0; i < BAR_COUNT; i++)
    {
        float w = 1.4f - (float)i / BAR_COUNT * 0.8f;

        sum += bars[i].output * w;
        wsum += w;
    }

    lv = sum / wsum / (BAR_MAX - 1);
    if (lv > 1.0f)
        lv = 1.0f;

    fft_vu.needle += (lv - fft_vu.needle) * ((lv > fft_vu.needle) ? 0.16f : 0.035f);

    if (lv >= fft_vu.peak)
    {
        fft_vu.peak = lv;
        fft_vu.hold = 26;
    }
    else if (fft_vu.hold)
        fft_vu.hold--;
    else if (fft_vu.peak > lv)
    {
        fft_vu.peak -= 0.004f;
        if (fft_vu.peak < lv)
            fft_vu.peak = lv;
    }

    /* 刻度弧 */
    for (int s = 0; s < 21; s++)
        draw_point_gray(cx + fft_vu_arc[s][0], cy + fft_vu_arc[s][1], bar,
                        (s % 5) == 0 ? 6 : 2);

    /* 峰值记忆点 */
    {
        float a = (-62.0f + fft_vu.peak * 124.0f) * 3.14159265f / 180.0f;

        draw_point((int)(cx + sinf(a) * 15.2f + 0.5f),
                   (int)(cy - cosf(a) * 15.2f + 0.5f), dot);
    }

    /* 指针 */
    {
        float a = (-62.0f + fft_vu.needle * 124.0f) * 3.14159265f / 180.0f;
        float sa = sinf(a), ca = cosf(a);

        for (int s = 0; s <= 16; s++)
        {
            float rr = 15.2f - s * 0.9f;

            draw_point_gray((int)(cx + sa * rr + 0.5f),
                            (int)(cy - ca * rr + 0.5f), bar, s < 2 ? 7 : 5);
        }
    }

    /* 底部电平条 */
    {
        int n = (int)(lv * BAR_COUNT + 0.5f);

        for (int i = 0; i < n; i++)
            draw_point_gray(i, MAX_Y - 1, dot, i > BAR_COUNT * 82 / 100 ? 7 : 4);
    }
}

/* ⑧ 三色分段：低频青 / 中频红 / 高频白 */
static void fft_draw_split(Color bar, Color dot)
{
    (void)bar;

    for (int i = 0; i < BAR_COUNT; i++)
    {
        Color c = (i < BAR_COUNT * 28 / 100) ? Cyan
                  : (i < BAR_COUNT * 66 / 100) ? Red
                                               : White;
        int h = (int)bars[i].output;
        uint8_t lv;

        for (int j = 0; j < h; j++)
        {
            lv = fft_lv(j, h);
            draw_point_gray(i, MAX_Y - 1 - j, c, lv);
        }

        if (h > 0)
            draw_point(i, MAX_Y - h, White);

        draw_point(i, MAX_Y - 1 - bars[i].maximum, dot);
    }
}

void draw_fft(long *fft_result, int len, int delta, uint8_t style, Color bar, Color dot)
{
    static uint32_t last_time = 0;
    uint32_t current_time = clock_anim_tick();
    float dt = (current_time - last_time) / 1000.0f;
    int cols;

    last_time = current_time;

    /* 确保有足够的数据点（至少3个才能排除首尾） */
    if (len <= 2)
        return;

    if (delta == 0)
        delta = 1;

    if (dt <= 0.0f || dt > 0.5f)
        dt = 0.05f;

    cols = (style == 1) ? (BAR_COUNT / 2) : BAR_COUNT;

    for (int i = 0; i < cols; i++)
    {
        int pos16 = fft_bin_x16[(style == 1) ? (i * 2) : i];
        int idx = pos16 >> 4;
        int frac = pos16 & 15;
        long v;

        if (idx < 1)
            idx = 1;
        if (idx > len - 2)
            idx = len - 2;

        v = fft_result[idx];
        if (frac && (idx + 1 < len - 1))
            v = (v * (16 - frac) + fft_result[idx + 1] * frac) / 16;

        update_display(&bars[i], (float)(v / delta), dt);
    }

    switch (style)
    {
    case 1:
        fft_draw_mirror(bar, dot);
        break;
    case 2:
        fft_draw_curve(bar, dot);
        break;
    case 3:
        fft_draw_dots(bar, dot);
        break;
    case 4:
        fft_draw_waterfall(bar, dot);
        break;
    case 5:
        fft_draw_spark(bar, dot);
        break;
    case 6:
        fft_draw_vu(bar, dot);
        break;
    case 7:
        fft_draw_split(bar, dot);
        break;
    case 0:
    default:
        fft_draw_bars(bar, dot);
        break;
    }
}

