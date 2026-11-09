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
#define CLOCK_THEME_COUNT 14

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
void draw_point_gray(int x, int y, Color color, uint8_t gray);
/* 时钟界面 · 桌面宠物：分钟跳变时调一次，出场演一遍（right_limit = 数字区右边界） */
void clock_pet_trigger(uint32_t now, uint8_t allow_right);
void clock_pet_draw(uint32_t now, int right_limit, uint8_t allow_right, Color color);

void refrash_gram(void);
#define FFT_STYLE_COUNT 16
#define FFT_COLOR_COUNT 6

void draw_fft(long *fft_result, int len, int delta, uint8_t style, Color bar, Color dot);
void fft_style_reset(void);
int get_long_str_count(void);
void roll_all_long_str(void);
#define GAME_COUNT 3        /* 游戏合集里的游戏数量：贪吃蛇 / 俄罗斯方块 / 坦克大战 */
#define SNAKE_FIXED_WIDTH 3 /* 贪吃蛇格子宽度，固定不可调 */
void refrashSnake(void);
void snakeRestart(void); /* 重开一局贪吃蛇（宽度固定 SNAKE_FIXED_WIDTH） */
void refrashTetris(void); /* 俄罗斯方块界面：推进一帧并画到显存 */
void refrashTanks(void);  /* 坦克大战界面：推进一帧并画到显存 */
void tanksRegen(void);    /* 重新随机生成坦克大战地图 */
void game_restart(uint8_t id); /* 重开指定游戏：0=贪吃蛇 1=俄罗斯方块 2=坦克大战 */
/* 冲击波切游戏，分两段调用：中间是黑屏，可以在里面写 FLASH 存档 */
void game_switch_anim_out(uint8_t id); /* 炸掉旧游戏 + 留下黑屏 */
void game_switch_anim_in(void);        /* 黑屏里重开新游戏并涌出来 */
/* 动画之间切换：整屏横向滑走 -> 黑屏 -> 新动画从右侧滑入（滑到位会回弹一下）
 * 三段时长约 154 / 55 / 196 ms，黑屏那段留给调用方写 FLASH。 */
void motion_switch_anim_out(void);          /* 旧动画滑走 + 留下黑屏 */
void motion_switch_anim_in(void (*build)(void)); /* 黑屏里调 build() 画新动画第一帧，再滑进来 */
void gradient_flow_frame_clock(void);
void clock_draw_background(uint8_t bg);
void clock_shatter_anim(void);   /* 当前画面碎裂落地（进菜单用） */
void clock_shatter_rebuild(void); /* 新画面从上方落下拼合（碎裂之后调用） */
void clock_wipe_scan_out(void);   /* 扫描线把当前画面刷掉（结束时全黑，换主题用） */
void clock_wipe_scan_begin_in(void); /* 黑场结束后调用，新画面开始刷进来 */
void clock_wipe_scan_in(void);    /* 每帧画完、送屏之前调用，把新画面用扫描线刷出来 */
uint32_t clock_anim_tick(void);   /* 可暂停的动画时间基准（ms） */
void clock_anim_pause(uint32_t ms); /* 过渡动画期间把动画时间按住 */

void set_g_clockWidth(uint8_t width);
void set_g_maxGray(uint8_t gray);
void clock_set_font_style(uint8_t style); /* 0=标准 3x5 放大 1=原生 9x15 */
void draw_char_3x5_wave_fade(int x, int y, char ch, Color color, uint8_t level);
void draw_char_flip(int x, int y, char ch, Color color, uint8_t visH, uint8_t wave);
/* 碎裂/拼合：phase 0=碎掉下落 1=从上方拼合，elapsed 为该阶段已进行的毫秒数 */
void draw_char_shatter(int x, int y, char ch, Color color, uint8_t wave, uint8_t phase, uint16_t elapsed);
void draw_colon_fade(int x, int y, Color color, uint8_t topLevel, uint8_t botLevel, uint8_t wave);
void draw_icon(int x, int y, IconType type, Color color);
void draw_icon_wave(int x, int y, IconType type, Color color);
void draw_bitmap(int x, int y, const uint8_t *bitmap, int width, int height, Color color);
void draw_bitmap_wave(int x, int y, const uint8_t *bitmap, int width, int height, Color color);
#endif
