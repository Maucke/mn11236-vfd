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
    float output;
    float target;

    float last_realval;
    float last_error;

    int maximum;
    int remain;
} FFTDisplay_t;

void draw_char_3x5(int x,int y, char ch, Color color);
void draw_char(int x,int y, char ch, Color color);
int roll_long_str(LongStr_t *s);
void clear_all_long_str(void);
void clock_init(void);
void draw_str(int x,int y, char *str, Color color);
void draw_str_3x5(int x,int y, char *str, Color color);
void draw_str_3x5_wave(int x, int y, char *str, Color color);
void draw_num_str(int x, int y, char *str, uint8_t height, Color color);
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
void gradient_flow_frame_clock(void);

void set_g_clockWidth(uint8_t width);
#endif
