#include "bootui.h"
#include "gui.h"
#include "mn11236.h"
#include "main.h"
#include "usb_device.h"

/* VFD 只有红/蓝两种颜色，GUI 里 1 = 红，2 = 蓝 */
#define COL_RED        1u
#define COL_BLUE       2u

/* 112x18 屏幕分三块：0..7 标题，8 分隔线，10..17 状态 */
#define TITLE_Y        0
#define RULE_Y         8
#define BODY_Y         10
#define BODY_H         8

#define VERSION_TEXT   "V1.0"

#define ANIM_MS        40u   /* 跑马灯每走 1 像素的时间 */
#define BUSY_MS        40u   /* 有动画时的重绘周期 */
#define IDLE_MS        120u  /* 静止界面的重绘周期 */

static volatile uint8_t  s_writing;
static volatile uint8_t  s_done;
static volatile uint8_t  s_error;
static volatile uint32_t s_bytes;
static uint32_t          s_error_tick;

static uint8_t  s_drawn = 0xFF;
static uint32_t s_last_draw;

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t str_len(const char *s)
{
    uint8_t n = 0;

    while (s[n])
        n++;

    return n;
}

static void text_at(int16_t x, int16_t y, const char *s, uint16_t color)
{
    GUI_SetColor(color, 0);
    GUI_ShowStr(x, y, s);
}

static void text_right(const char *s, int16_t y, uint16_t color)
{
    int16_t x = (int16_t)LCD_W - 6 * (int16_t)str_len(s);

    text_at(x < 0 ? 0 : x, y, s, color);
}

static void draw_title(void)
{
    text_at(0, TITLE_Y, "MN11236 BOOT", COL_RED);
    text_right(VERSION_TEXT, TITLE_Y, COL_BLUE);
}

/* 虚点分隔线：1 像素实线在 VFD 上偏重，隔一个点画一次更清爽 */
static void draw_rule(void)
{
    for (int16_t x = 0; x < LCD_W; x += 2)
        GUI_DrawPoint(x, RULE_Y, COL_BLUE);
}

/* 已写入字节数 -> "45K"，不用 printf，省 flash */
static void kb_text(char *out, uint32_t bytes)
{
    char tmp[4];
    uint8_t n = 0;
    uint32_t k = bytes >> 10;

    if (k > 999u)
        k = 999u;

    do
    {
        tmp[n++] = (char)('0' + (k % 10u));
        k /= 10u;
    } while (k);

    while (n)
        *out++ = tmp[--n];

    *out++ = 'K';
    *out = 0;
}

static void draw_dots(uint32_t tick)
{
    uint8_t phase = (uint8_t)((tick / 350u) % 4u);

    for (uint8_t i = 0; i < 3; i++)
    {
        if (i < phase)
            GUI_DrawBox(LCD_W - 18 + i * 8, BODY_Y + 3, 2, 3, COL_RED);
    }
}

static void render(bootui_state_t st, uint32_t tick)
{
    GUI_ClearBuffer();

    switch (st)
    {
    case BOOTUI_WRITING:
    {
        char kb[6];

        kb_text(kb, s_bytes);

        text_at(0, TITLE_Y, "WRITING", COL_RED);
        text_right(kb, TITLE_Y, COL_BLUE);

        draw_rule();

        GUI_DrawFrame(0, BODY_Y, LCD_W, BODY_H, COL_BLUE);

        /* 文件总长要等 FAT 落盘才知道，这里用跑马灯表示"正在写" */
        int16_t pos = (int16_t)((tick / ANIM_MS) % (LCD_W + 24u)) - 24;

        GUI_DrawBox(pos, BODY_Y + 1, 24, BODY_H - 2, COL_RED);
        break;
    }

    case BOOTUI_DONE:
        text_at(0, TITLE_Y, "UPDATE COMPLETE", COL_RED);
        draw_rule();
        GUI_DrawFrame(0, BODY_Y, LCD_W, BODY_H, COL_BLUE);
        GUI_DrawBox(1, BODY_Y + 1, LCD_W - 2, BODY_H - 2, COL_RED);
        break;

    case BOOTUI_ERROR:
        text_at(0, TITLE_Y, "FILE ERROR", COL_RED);
        draw_rule();

        if ((tick / 300u) & 1u)
            text_at(0, BODY_Y, "BAD HEADER", COL_BLUE);
        break;

    case BOOTUI_READY:
        draw_title();
        draw_rule();
        text_at(0, BODY_Y, "DRAG BIN TO DRIVE", COL_BLUE);

        if ((tick / 400u) & 1u)
            GUI_DrawBox(LCD_W - 6, BODY_Y + 3, 2, 3, COL_RED);
        break;

    default: /* BOOTUI_IDLE */
        draw_title();
        draw_rule();
        text_at(0, BODY_Y, "WAITING FOR USB", COL_BLUE);
        draw_dots(tick);
        break;
    }
}

static bootui_state_t compute_state(void)
{
    if (s_done)
        return BOOTUI_DONE;

    if (s_error)
        return BOOTUI_ERROR;

    if (s_writing)
        return BOOTUI_WRITING;

    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
        return BOOTUI_READY;

    return BOOTUI_IDLE;
}

void bootui_init(void)
{
    GUI_Init();
    GUI_ClearBuffer();

    s_drawn = 0xFF;
}

void bootui_poll(void)
{
    uint32_t tick = HAL_GetTick();
    bootui_state_t st;
    uint32_t interval;

    /* 文件头不对的提示亮 4 秒就恢复，允许用户直接重拖一个文件 */
    if (s_error && (tick - s_error_tick) > 4000u)
        s_error = 0;

    st = compute_state();
    interval = (st == BOOTUI_WRITING) ? BUSY_MS : IDLE_MS;

    if (st != s_drawn || (tick - s_last_draw) >= interval)
    {
        s_drawn = st;
        s_last_draw = tick;

        render(st, tick);
        GUI_SendBuffer();
    }
}

bootui_state_t bootui_get_state(void)
{
    return compute_state();
}

uint8_t bootui_is_finished(void)
{
    return s_done;
}

void bootui_msc_begin(void)
{
    s_bytes = 0;
    s_error = 0;
    s_writing = 1;
}

void bootui_msc_progress(uint32_t bytes)
{
    s_bytes = bytes;
    s_error = 0;
    s_writing = 1;
}

void bootui_msc_end(void)
{
    s_writing = 0;
    s_done = 1;
}

void bootui_msc_error(void)
{
    s_writing = 0;
    s_error = 1;
    s_error_tick = HAL_GetTick();
}
