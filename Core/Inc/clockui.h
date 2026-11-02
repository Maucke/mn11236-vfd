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

typedef enum
{
    ICON_RPM,
    ICON_KMH,
	ICON_LOAD,
	ICON_VOLT,
	ICON_TEMP
} IconType;

typedef struct
{
	Color border;
	Color font;
	uint8_t type; // 0=波形文字 1=普通文字
	uint8_t bg;   // 背景动画，见 ClockBg_t
	uint8_t style; // 字体：0=标准 3x5 放大，1=原生 9x15
} ClockTheme_t;

/* 背景动画组 */
typedef enum
{
	CLOCK_BG_NONE = 0,  // 无背景动画（纯黑背景）
	CLOCK_BG_WAVE,	    // 三波叠加渐变（原 gradient_flow）
	CLOCK_BG_BANDS,	    // 流动光带
	CLOCK_BG_GLOW,      // 中心光晕
	CLOCK_BG_WIPE,      // 光条擦除染色（蓝/红交替）
	CLOCK_BG_FLOW,      // 流动渐变
	CLOCK_BG_PLASMA,    // 等离子斑块
	CLOCK_BG_BREATHE,   // 整屏明暗呼吸
	CLOCK_BG_COUNT
} ClockBg_t;

/* clockTheme[] 的主题数量，必须与数组实际长度一致 */
#define CLOCK_THEME_COUNT 20

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
void draw_char_3x5_wave(int x, int y, char ch, Color color);
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
void clock_draw_background(uint8_t bg);
void clock_shatter_anim(void); /* 当前画面碎裂落地（进菜单用） */
void clock_shatter_rebuild(void); /* 新画面从上方落下拼合（碎裂之后调用） */

void set_g_clockWidth(uint8_t width);
void set_g_maxGray(uint8_t gray);
void clock_set_font_style(uint8_t style); /* 0=标准 3x5 放大 1=原生 9x15 */
void draw_char_3x5_wave_fade(int x, int y, char ch, Color color, uint8_t level);
void draw_char_flip(int x, int y, char ch, Color color, uint8_t visH, uint8_t wave);
void draw_colon_fade(int x, int y, Color color, uint8_t topLevel, uint8_t botLevel, uint8_t wave);
void draw_icon(int x, int y, IconType type, Color color);
void draw_icon_wave(int x, int y, IconType type, Color color);
void draw_bitmap(int x, int y, const uint8_t *bitmap, int width, int height, Color color);
void draw_bitmap_wave(int x, int y, const uint8_t *bitmap, int width, int height, Color color);
#endif
