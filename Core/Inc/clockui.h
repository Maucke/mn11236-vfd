#ifndef __CLOCKUI_H__
#define __CLOCKUI_H__

#include "main.h"
#include "mn11236.h"

#define CHAR_COUNT (95 + 1)

#define MAX_X VFD_WIDTH
#define MAX_Y VFD_HEIGHT
#define LONG_STR_LEN 64

typedef enum
{
	None,
	Cyan = 0x01,
	Red = 0x02,
	White = 0x03,

	CyanWithCyan = 0x5,   // 0101
	RedWithCyan = 0x06,	  // 0110
	WhiteWithCyan = 0x07, // 0111
	CyanWithRed = 0x09,	  // 1001
	RedWithRed = 0xA,   	// 1010
	WhiteWithRed = 0x0B,  // 1011
	WhiteWithWhite = 0x0F,// 1111
} Color;

typedef struct
{
	Color border;
	Color font;
	uint8_t type;
} ClockTheme_t;

typedef struct
{
	Color bar;
	Color dot;
} MusicTheme_t;

typedef struct
{
	uint8_t str[LONG_STR_LEN * 6 + MAX_X];
	uint8_t *current;
	int length;
	int repeats; //-1 means infinite
	Color color;
} LongStr_t;

typedef struct
{
	float output;	  // 当前显示值
	float last_error; // 上一次误差

	float last_realval;
	float maximum;
	uint8_t remain;
} FFTDisplay_t;

int roll_long_str(LongStr_t *s);
void clear_all_long_str(void);
void clock_init(void);
void clear_gram(Color color);
LongStr_t *draw_long_str(char *str, Color color, int repeats);
void draw_point(int x, int y, Color color);
void refrash_gram(void);
void draw_fft(long *fft_result, int len, int delta, Color bar, Color dot);
int get_long_str_count(void);
void roll_all_long_str(void);
void beginSnake(void);
void refrashSnake(void);
void setSnakeWidth(uint8_t width);

#endif
