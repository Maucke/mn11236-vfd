#ifndef __GU20X8_301_H__
#define __GU20X8_301_H__

#include "main.h"

#define CHAR_COUNT (95 + 1)

#define MAX_X 20
#define MAX_Y 8
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

void draw_char_3x5(int x,int y, char ch, Color color);
void draw_char(int x,int y, char ch, Color color);
int roll_long_str(LongStr_t *s);
void clear_all_long_str(void);
void refrash_gram(void);
void vfd_init(void);
void set_dimming(uint8_t dimm);
void draw_str(int x,int y, char *str, Color color);
void draw_str_3x5(int x,int y, char *str, Color color);
void draw_num_str(int x, int y, char *str, uint8_t height, Color color);
void clear_gram(Color color);
LongStr_t *draw_long_str(char *str, Color color, int repeats);
void draw_point(int x, int y, Color color);
void draw_fft(long *fft_result, int len, int delta, Color bar, Color dot);
int get_long_str_count(void);
void roll_all_long_str(void);
void beginSnake(void);
void refrashSnake(void);

#endif
