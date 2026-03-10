/*!
 * Copyright (c) 2023, ErBW_s
 * All rights reserved.
 *
 * @author  Baohan
 */

#include "easy_ui_user_app.h"
#include "usbd_nex_link.h"
#include "lv_anim_light.h"
#include "usbd_nex_link.h"
#include "zf_common_font.h"
#include "animation.h"
#include "main.h"
#include "adc.h"
#include "stm32_rx8900.h"
#include <time.h>
#include "animation.h"
#include "fftaffect.h"
#include "main.h"
#include "clockui.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stmflash.h"
#include "semphr.h"
#include "nexlink_app.h"
#include <math.h>

// Pages
EasyUIPage_t pageMain, pageUSBForm, pageDialog, pageSensor, pageAnimation, pageSetting, pageDiagnostic, pageAbout, pageSetTime;

// Items
EasyUIItem_t itemUSBForm;
EasyUIItem_t itemDebug;
EasyUIItem_t itemSensor;
EasyUIItem_t itemAnimation;
EasyUIItem_t itemSetting, titleSetting, itemMusicTheme, itemClockTheme, itemSetTime, itemDefaultFunc, itemClockDimming;
EasyUIItem_t itemDiagnostic;
EasyUIItem_t itemAbout;
EasyUIItem_t itemMind, itemCircle, itemSnowflake, itemMeteo, itemPlanet, itemTriangle, itemStarwar, itemBlast, itemGCircle, itemFirework, titleAnimation;
EasyUIItem_t itemScreenOnHour;
EasyUIItem_t itemScreenOffHour;

EasyKey_t keyUp, keyDown;

/*!
 * @brief   Sync the operation bool value
 *
 * @param   void
 * @return  void
 */
void EasyUIKeyActionMonitor() // Interrupt trigger, No EasyUIDelay_ms(xx)
{
  if (keyUp.isPressed)
  {
    opnEnter = 1;
    dbmsg("keyUp:isPressed");
  }
  else if (keyUp.isHold)
  {
    opnExit = 1;
    dbmsg("keyUp:isHold");
  }
  else if (keyUp.holdTime > 5000)
  {
    dbmsg("keyUp:holdTime:%d", keyDown.holdTime);
  }
  if (keyDown.isPressed)
  {
    opnDown = 1;
    dbmsg("keyDown:isPressed");
  }
  else if (keyDown.isHold)
  {
    opnUp = 1;
    dbmsg("keyDown:isHold");
  }
  else if (keyDown.holdTime > 5000)
  {
    dbmsg("keyDown:holdTime:%d", keyDown.holdTime);
  }

  if (opnEnter || opnExit || opnUp || opnDown)
    return;

#if 0
  if(opnEnter != 0)
    dbmsg("opnEnter:%d", opnEnter);
  if(opnExit != 0)
    dbmsg("opnExit:%d", opnExit);
  if(opnUp != 0)
    dbmsg("opnUp:%d", opnUp);
  if(opnDown != 0)
    dbmsg("opnDown:%d", opnDown);
#endif
}


char *EasyUIVersion = "Ver. 1.3.3";
void PageAbout(EasyUIPage_t *page)
{
    static uint8_t pageIndex = 0;
    const uint8_t totalLines = 3;
    const uint8_t linesPerPage = 2;

    char tempstr[128];
    const char *lines[3];

    // 构建字符串
    snprintf(tempstr, sizeof(tempstr), "Rel. %s", __DATE__);

    lines[0] = "Author: DPJ";
    lines[1] = EasyUIVersion;
    lines[2] = tempstr;

    // 按键翻页
    if (opnDown)
    {
        if ((pageIndex + 1) * linesPerPage < totalLines)
            pageIndex++;
        opnDown = false;
    }

    if (opnUp)
    {
        if (pageIndex > 0)
            pageIndex--;
        opnUp = false;
    }

    // 显示当前页
    int screen_delta = 0;

    for (int i = 0; i < linesPerPage; i++)
    {
        int lineIndex = pageIndex * linesPerPage + i;

        if (lineIndex >= totalLines)
            break;

        EasyUIDisplayStr(10, screen_delta, lines[lineIndex]);
        screen_delta += page->rowheight;
    }
}
typedef enum
{
  SetYear,
  SetMonth,
  SetDay,
  SetWeek,
  SetHour,
  SetMin,
} SetTimeMode;

extern struct tm tm_local;
static const char weekdays[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
void PageSetTime(EasyUIPage_t *page)
{
  static bool syncstate = false;
  static struct tm tm_setting;
  char tempstr[64];
  int screen_delta = 1;
  static SetTimeMode mode = SetYear;
  switch (mode)
  {
  case SetYear:
    if (!syncstate)
    {
      syncstate = true;
      tm_setting = tm_local;
    }

    EasyUIDrawFrame(10, 0, 4 * 6 + 1, 8 + 1, GUI_DEFAULT_ACTCOLOR);
    if (opnDown)
    {
      tm_setting.tm_year += 1;
      if (tm_setting.tm_year > 99)
        tm_setting.tm_year = 0;
      opnDown = false;
    }
    if (opnUp)
    {
      tm_setting.tm_year -= 1;
      if (tm_setting.tm_year < 0)
        tm_setting.tm_year = 99;
      opnUp = false;
    }
    if (opnEnter)
    {
      mode = SetMonth;
      opnEnter = false;
    }
    break;
  case SetMonth:
    EasyUIDrawFrame(10 + 4 * 6 + 6, 0, 2 * 6 + 1, 8 + 1, GUI_DEFAULT_ACTCOLOR);
    if (opnDown)
    {
      tm_setting.tm_mon += 1;
      if (tm_setting.tm_mon > 11)
        tm_setting.tm_mon = 0;
      opnDown = false;
    }
    if (opnUp)
    {
      tm_setting.tm_mon -= 1;
      if (tm_setting.tm_mon < 0)
        tm_setting.tm_mon = 11;
      opnUp = false;
    }
    if (opnEnter)
    {
      mode = SetDay;
      opnEnter = false;
    }
    break;
  case SetDay:
    EasyUIDrawFrame(10 + 4 * 6 + 2 * 6 + 6 * 2, 0, 2 * 6 + 1, 8 + 1, GUI_DEFAULT_ACTCOLOR);
    if (opnDown)
    {
      tm_setting.tm_mday += 1;
      if (tm_setting.tm_mday > 31)
        tm_setting.tm_mday = 1;
      opnDown = false;
    }
    if (opnUp)
    {
      tm_setting.tm_mday -= 1;
      if (tm_setting.tm_mday < 1)
        tm_setting.tm_mday = 31;
      opnUp = false;
    }
    if (opnEnter)
    {
      mode = SetWeek;
      opnEnter = false;
    }
    break;
  case SetWeek:
    EasyUIDrawFrame(10 + 4 * 6 + 2 * 6 + 6 * 4 + 2 * 6, 0, 3 * 6 + 1, 8 + 1, GUI_DEFAULT_ACTCOLOR);
    if (opnDown)
    {
      tm_setting.tm_wday = (tm_setting.tm_wday + 1) % 7;
      opnDown = false;
    }
    if (opnUp)
    {
      tm_setting.tm_wday = (tm_setting.tm_wday + 6) % 7;
      opnUp = false;
    }
    if (opnEnter)
    {
      mode = SetHour;
      opnEnter = false;
    }
    break;

  case SetHour:
    EasyUIDrawFrame(10, 0 + 9, 2 * 6 + 1, 8 + 1, GUI_DEFAULT_ACTCOLOR);
    if (opnDown)
    {
      tm_setting.tm_hour = (tm_setting.tm_hour + 1) % 24;
      opnDown = false;
    }
    if (opnUp)
    {
      tm_setting.tm_hour = (tm_setting.tm_hour + 23) % 24;
      opnUp = false;
    }
    if (opnEnter)
    {
      mode = SetMin;
      opnEnter = false;
    }
    break;
  case SetMin:
    EasyUIDrawFrame(10 + 3 * 6, 0 + 9, 2 * 6 + 1, 8 + 1, GUI_DEFAULT_ACTCOLOR);
    if (opnDown)
    {
      tm_setting.tm_min = (tm_setting.tm_min + 1) % 60;
      opnDown = false;
    }
    if (opnUp)
    {
      tm_setting.tm_min = (tm_setting.tm_min + 59) % 60;
      opnUp = false;
    }
    if (opnEnter)
    {
      RX8900_SetTime(&tm_setting);
      mode = SetYear;
      syncstate = false;
      EasyUIBackgroundBlur();
      opnEnter = false;
      opnExit = true;
    }
    break;
  }
  snprintf(tempstr, sizeof tempstr, "%04d-%02d-%02d, %s", tm_setting.tm_year + 2000, tm_setting.tm_mon + 1, tm_setting.tm_mday, weekdays[tm_setting.tm_wday % 7]);
  EasyUIDisplayStr(10, screen_delta, tempstr);
  screen_delta += 9;
  snprintf(tempstr, sizeof tempstr, "%02d:%02d", tm_setting.tm_hour % 100, tm_setting.tm_min % 100);
  EasyUIDisplayStr(10, screen_delta, tempstr);
  if (opnExit)
  {
    mode = SetYear;
    syncstate = false;
  }
}

void PageSensor(EasyUIPage_t *page)
{
  char tempstr[64];
  int screen_delta = 1;
  static long last_update_time = 0;
  long now_tick = HAL_GetTick();
  if (now_tick - last_update_time > 500)
  {
    snprintf(tempstr, sizeof tempstr, "%04d-%02d-%02d, %s", tm_local.tm_year + 2000, tm_local.tm_mon + 1, tm_local.tm_mday, weekdays[tm_local.tm_wday % 7]);
    EasyUIDisplayStr(10, screen_delta, tempstr);
    screen_delta += 9;
    snprintf(tempstr, sizeof tempstr, "%02d:%02d:%02d", tm_local.tm_hour % 100, tm_local.tm_min % 100, tm_local.tm_sec % 100);
    EasyUIDisplayStr(10, screen_delta, tempstr);
  }
}
#define W1 0.06f
#define W2 0.04f
#define W3 0.03f

#define SPEED1 0.15f
#define SPEED2 0.10f
#define SPEED3 0.07f

static float t1 = 0;
static float t2 = 0;
static float t3 = 0;
extern uint8_t frame_wave[MAX_X][MAX_Y]; // 6-4:red, 2-0:blue

static void flush_frame(void)
{
    for (int y = 0; y < MAX_Y; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            uint8_t p = frame_wave[x][y];

            uint8_t red  = (p >> 4) & 0x07;
            uint8_t blue =  p       & 0x07;

            mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED,  red);
            mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, blue);
        }
    }
}
static void gradient_flow_frame(void)
{
    for (int y = 0; y < MAX_Y; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            // ===== 多波叠加 =====
            float v =
                sinf(x * W1 + t1) +
                sinf(y * W2 + t2) +
                sinf((x + y) * W3 + t3);
            // 归一化到 0~1
            v = (v + 3.0f) / 6.0f;

            if (v < 0) v = 0;
            if (v > 1) v = 1;

            /* 映射 */
            uint8_t blue = (uint8_t)(v * 7.0f);   // 3bit
            uint8_t red  = (uint8_t)((1.0f - v) * 7.0f); // 3bit

            frame_wave[x][y] = (red << 4) | (blue & 0x07);
        }
    }

    t1 += SPEED1;
    t2 += SPEED2;
    t3 += SPEED3;
		
    flush_frame();
}

static uint8_t fire[MAX_Y][MAX_X]= {0};
void fire_update(void)
{
    // ===== 1. 强化底部火源 =====
    for (int x = 0; x < MAX_X; x++)
    {
        fire[MAX_Y - 1][x] = 5 + (rand() & 0x03);   // 5~7
    }
		if (rand() % 20 == 0)
		{
				int rx = rand() % MAX_X;
				int ry = rand() % (MAX_Y / 2);
				fire[ry][rx] = 7;
		}
    // ===== 2. 向上传播 =====
    for (int y = 0; y < MAX_Y - 1; y++)
    {
        for (int x = 1; x < MAX_X - 1; x++)
        {
            int sum =
                fire[y + 1][x] +
                fire[y + 1][x - 1] +
                fire[y + 1][x + 1] +
                fire[y + 2 < MAX_Y ? y + 2 : y + 1][x];

            uint8_t val = sum >> 2;   // 平均

            // ===== 减少衰减 =====
            if (val > 0 && (rand() & 4))
                val--;

            // ===== 左右漂移 =====
            int shift = (rand() % 3) - 1;   // -1 0 +1
            int nx = x + shift;

            if (nx >= 0 && nx < MAX_X)
                fire[y][nx] = val;
        }
    }
}
void fire_draw(void)
{
    for (int y = 0; y < MAX_Y; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            uint8_t v = fire[y][x];  // 原始温度

            if (v == 0)
                continue;

            // ===== 计算高度冷却 =====
            // 顶部冷却最大，底部冷却为0
						uint8_t cool = ((MAX_Y - 1 - y) * 2) / MAX_Y;

            uint8_t effective = (v > cool) ? (v - cool) : 0;

            if (effective == 0)
                continue;

            uint8_t redGray = 0;
            uint8_t cyanGray = 0;

            // ===== 灰度渐变映射 =====
            uint8_t t = effective * 2;  // 0~14

            if (t <= 7)
            {
                // 蓝 → 红
                cyanGray = 7 - t;
                redGray  = t;
            }
            else
            {
                // 红 → 白
                cyanGray = t - 7;
                redGray  = 7;
            }

            if (cyanGray > 7) cyanGray = 7;
            if (redGray > 7) redGray = 7;

            if (cyanGray)
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, cyanGray);

            if (redGray)
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, redGray);
        }
    }
}

void PageDiagnostic(EasyUIItem_t *page)
{
    static uint8_t mode = 0;
    static uint8_t gray = 0;
    static int8_t  dir  = 1;
    static uint32_t last_tick = 0;

    // ==========================
    // 上下键都切换 mode
    // ==========================
    if (opnUp)
    {
        mode++;
        if (mode > 5)
            mode = 0;
        opnUp = false;
    }

    if (opnDown)
    {
        if (mode == 0)
            mode = 5;
        else
            mode--;
        opnDown = false;
    }

    uint32_t now = HAL_GetTick();

    // 每40ms更新动画
    if (now - last_tick >= 40)
    {
        last_tick = now;

        gray += dir;

        if (gray == 7 || gray == 0)
            dir = -dir;
    }

    mn11236_clear_buffers();

    // ====================================================
    // mode 0：蓝色呼吸
    // ====================================================
    if (mode == 0)
    {
        for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
        {
            for (uint8_t y = 0; y < SCREEN_HEIGHT; y++)
            {
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_BLUE, gray);
            }
        }
    }

    // ====================================================
    // mode 1：红色呼吸
    // ====================================================
    else if (mode == 1)
    {
        for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
        {
            for (uint8_t y = 0; y < SCREEN_HEIGHT; y++)
            {
                mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, gray);
            }
        }
    }

    // ====================================================
    // mode 2：横向蓝0~7 红0~7 渐变
    // ====================================================
    else if (mode == 2)
    {
        uint8_t section_width = SCREEN_WIDTH / 16;

        for (uint8_t i = 0; i < 16; i++)
        {
            uint8_t color = (i < 8) ? MN11236_COLOR_BLUE : MN11236_COLOR_RED;
            uint8_t g     = (i < 8) ? i : (i - 8);

            for (uint8_t x = i * section_width; x < (i + 1) * section_width; x++)
            {
                for (uint8_t y = 0; y < SCREEN_HEIGHT; y++)
                {
                    mn11236_draw_pixel_gray(x, y, (mn11236_color_t)color, g);
                }
            }
        }
    }

    // ====================================================
    // mode 3：动态棋盘灰度
    // ====================================================
    else if (mode == 3)
    {
        for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
        {
            for (uint8_t y = 0; y < SCREEN_HEIGHT; y++)
            {
                uint8_t g = ((x / 4 + y / 4 + gray) % 8);
                uint8_t c = (x / 8) % 2;

                mn11236_draw_pixel_gray(x, y, (mn11236_color_t)c, g);
            }
        }
    }
    else if (mode == 4)
		{    
			fire_update();
			fire_draw();
		}
    else if (mode == 5)
		{    
			gradient_flow_frame();
		}

    mn11236_send_buffer();
}

UserInfo_t *pUserInfo;
extern long lBufMagArray[NPT];
extern uint8_t layer;
static SemaphoreHandle_t flashMutex;

static bool firstsave = true;

void SaveInfo()
{
  HAL_GPIO_WritePin(VFD_BK_GPIO_Port, VFD_BK_Pin, GPIO_PIN_SET);
  xSemaphoreTake(flashMutex, portMAX_DELAY);
  taskENTER_CRITICAL();
  STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)pUserInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
  taskEXIT_CRITICAL();
  xSemaphoreGive(flashMutex);
  HAL_GPIO_WritePin(VFD_BK_GPIO_Port, VFD_BK_Pin, GPIO_PIN_RESET);
  if (firstsave)
  {
    firstsave = false;
    SaveInfo();
  }
}

void InitInfo()
{
  flashMutex = xSemaphoreCreateMutex();
}

void EventChangeValue(EasyUIItem_t *item)
{
  if (opnDown)
  {
    if (*item->param + 1 <= item->paramMax)
      *item->param += 1;
    else
      *item->param = 0;
    opnDown = false;
  }
  if (opnUp)
  {
    if (*item->param - 1 >= 0)
      *item->param -= 1;
    else
      *item->param = item->paramMax;
    opnUp = false;
  }

  if (opnEnter)
  {
    item->paramBackup = *item->param;
    EasyUIBackgroundBlur();
    functionIsRunning = false;
    opnEnter = false;
  }
  if (opnExit)
  {
    *item->param = item->paramBackup;
    EasyUIBackgroundBlur();
    functionIsRunning = false;
    opnExit = false;
  }
  // Clear the states of key to monitor next key action
  opnEnter = opnUp = opnDown = false;
}

bool UserInfo_AutoSave()
{
  static UserInfo_t last;
  static bool inited = false;

  if (!inited)
  {
    memcpy(&last, pUserInfo, sizeof(UserInfo_t));
    inited = true;
    return false;
  }

  if (memcmp(&last, pUserInfo, sizeof(UserInfo_t)) != 0)
  {
    memcpy(&last, pUserInfo, sizeof(UserInfo_t));
    SaveInfo();
    return true;
  }

  return false;
}

typedef enum
{
  DISP_MODE_CLOCK = 0,
  DISP_MODE_FFT,
  DISP_MODE_SNAKE,
  DISP_MODE_MOTION,
  DISP_MODE_EXTERN,
  DISP_MODE_MENU,
} SysDisplayMode_e;

static uint32_t last_key_tick = 0;
#define MENU_TIMEOUT 10000

static uint32_t wake_until_tick = 0;   // 临时亮屏截止时间
#define WAKE_TIME_MS 10000             

static SysDisplayMode_e last_disp_mode = DISP_MODE_MOTION;
static SysDisplayMode_e disp_mode = DISP_MODE_MOTION;
void AppGUI_Init(UserInfo_t *userinfo)
{
  InitInfo();
  pUserInfo = userinfo;

  last_disp_mode = (SysDisplayMode_e)pUserInfo->defaultfunc;
  disp_mode = (SysDisplayMode_e)pUserInfo->defaultfunc;

  EasyUIAddPage(&pageMain, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSetting, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  //  EasyUIAddPage(&pageUSBForm, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageUSBForm);
  //  EasyUIAddPage(&pageDialog, GUI_6X8_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSensor, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageSensor);
  EasyUIAddPage(&pageAnimation, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageDiagnostic, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageDiagnostic);
  EasyUIAddPage(&pageAbout, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageAbout);
  EasyUIAddPage(&pageSetTime, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageSetTime);

  //  EasyUIAddItem(&pageMain, &itemUSBForm, "USBForm", ITEM_JUMP_PAGE, pageUSBForm.id);
  //  EasyUIAddItem(&pageMain, &itemDebug, "Debug", ITEM_JUMP_PAGE, pageDialog.id);
  EasyUIAddItem(&pageMain, &itemSensor, "Sensor", ITEM_JUMP_PAGE, pageSensor.id);
  EasyUIAddItem(&pageMain, &itemAnimation, "Animation", ITEM_JUMP_PAGE, pageAnimation.id);
  EasyUIAddItem(&pageMain, &itemSetting, "Setting", ITEM_JUMP_PAGE, pageSetting.id);
  EasyUIAddItem(&pageMain, &itemDiagnostic, "Diagnostic", ITEM_JUMP_PAGE, pageDiagnostic.id);
  EasyUIAddItem(&pageMain, &itemAbout, "<About>", ITEM_JUMP_PAGE, pageAbout.id);

  EasyUIAddItem(&pageSetting, &titleSetting, "[Setting]", ITEM_PAGE_DESCRIPTION);

  EasyUIAddItem(&pageSetting, &itemSetTime, "Set Time", ITEM_JUMP_PAGE, pageSetTime.id);
  EasyUIAddItem(&pageSetting, &itemMusicTheme, "FFT Theme", ITEM_PROGRESS_BAR, &pUserInfo->musictheme, 11, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemClockTheme, "Clock Theme", ITEM_PROGRESS_BAR, &pUserInfo->clocktheme, 4, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemDefaultFunc, "Default Func", ITEM_PROGRESS_BAR, &pUserInfo->defaultfunc, 3, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemClockDimming, "Clock Dimming", ITEM_PROGRESS_BAR, &pUserInfo->clockdimming, 4, EventChangeValue);
	EasyUIAddItem(&pageSetting, &itemScreenOnHour,
								"Screen ON", ITEM_PROGRESS_BAR,
								&pUserInfo->screen_on_hour, 23,
								EventChangeValue);

	EasyUIAddItem(&pageSetting, &itemScreenOffHour,
								"Screen OFF", ITEM_PROGRESS_BAR,
								&pUserInfo->screen_off_hour, 23,
								EventChangeValue);
								
  EasyUIAddItem(&pageAnimation, &titleAnimation, "[Animation]", ITEM_PAGE_DESCRIPTION);
  EasyUIAddItem(&pageAnimation, &itemMind, "Mind", ITEM_CHECKBOX, &pUserInfo->effectswitch.enMind);
  EasyUIAddItem(&pageAnimation, &itemCircle, "Circle", ITEM_CHECKBOX, &pUserInfo->effectswitch.enCircle);
  EasyUIAddItem(&pageAnimation, &itemSnowflake, "Snowflake", ITEM_CHECKBOX, &pUserInfo->effectswitch.enSnowflake);
  //  EasyUIAddItem(&pageAnimation, &itemMeteo, "Meteo", ITEM_CHECKBOX, &pUserInfo->effectswitch.enMeteo);
  EasyUIAddItem(&pageAnimation, &itemPlanet, "Planet", ITEM_CHECKBOX, &pUserInfo->effectswitch.enPlanet);
  EasyUIAddItem(&pageAnimation, &itemTriangle, "Triangle", ITEM_CHECKBOX, &pUserInfo->effectswitch.enTriangle);
  EasyUIAddItem(&pageAnimation, &itemStarwar, "Starwar", ITEM_CHECKBOX, &pUserInfo->effectswitch.enStarwar);
  //  EasyUIAddItem(&pageAnimation, &itemGCircle, "GCircle", ITEM_CHECKBOX, &pUserInfo->effectswitch.enGCircle);
  EasyUIAddItem(&pageAnimation, &itemFirework, "Firework", ITEM_CHECKBOX, &pUserInfo->effectswitch.enFirework);
  // Key init
  EasyKeyInit(&keyUp, GPIOB, KEY_SW1_Pin);
  EasyKeyInit(&keyDown, GPIOB, KEY_SW2_Pin);
  EasyUIInit(1);
  clock_init();
  setSnakeWidth(pUserInfo->snakewidth);
  beginSnake();
  Motion_Init();
	uint32_t now = HAL_GetTick();
	wake_until_tick = now + WAKE_TIME_MS;
}

const MusicTheme_t musicTheme[12] = {
    {Cyan, Red},
    {Cyan, White},
		{Red, Red},
    {Red, White},
    {White, Red},
    {White, Cyan},
		
    {CyanWithCyan, RedWithRed},
    {CyanWithCyan, WhiteWithWhite},
    {RedWithRed, CyanWithCyan},
    {RedWithRed, WhiteWithWhite},
    {WhiteWithWhite, RedWithRed},
    {WhiteWithWhite, CyanWithCyan},
};

void Motion_Run(EffectSwitch_t *effectswitch)
{
  if (effectswitch->enMind)
    Motion_Mind();
  if (effectswitch->enCircle)
    Motion_Circle();
  if (effectswitch->enSnowflake)
    Motion_Snowflake();
  //  if (effectswitch->enMeteo)
  //    Motion_Movmeteor();
  if (effectswitch->enPlanet)
    Motion_Planet();
  if (effectswitch->enTriangle)
    Motion_TriangleF();
  if (effectswitch->enStarwar)
    Motion_StarWar();
  //  if (effectswitch->enGCircle)
  //    Motion_GCFireworks();
  if (effectswitch->enFirework)
    Motion_Firework();
}
extern char timeStr[10];
extern char date[10];
extern char week[10];

const ClockTheme_t clockTheme[] = {
    {Red, Red, 0},
    {Red, Cyan, 0},
    {Cyan, White, 0},
//    {Cyan, White, 2},
    {Red, WhiteWithCyan, 1},
    {Cyan, WhiteWithRed, 1}};

void ExternUI_Init()
{
  disp_mode = DISP_MODE_EXTERN;
}

void UploadFrame()
{
  upload_frame_upload();
}

bool ScreenShouldOn(void)
{
    uint8_t now = tm_local.tm_hour;

    uint8_t on  = pUserInfo->screen_on_hour;
    uint8_t off = pUserInfo->screen_off_hour;

    if (on < off)
    {
        if (now >= on && now < off)
            return true;
        else
            return false;
    }
    else
    {
        // 跨午夜
        if (now >= on || now < off)
            return true;
        else
            return false;
    }
}

void AppGUI_Loop(uint8_t timer)
{
	static int rolling = 0;
  static char timeupdateflag;
//	mn11236_gray_test();
//	osDelay(5000);
//	return;

	uint32_t now = HAL_GetTick();

	bool night = !ScreenShouldOn();

	// 有按键 → 唤醒60秒
	if(opnUp || opnDown || opnEnter || opnExit)
	{
			last_key_tick = now;

			if(night)
			{
					wake_until_tick = now + WAKE_TIME_MS;
			}
	}

	// 夜间且未唤醒
	if(night && now > wake_until_tick && disp_mode != DISP_MODE_MENU)
	{
		if(HAL_GPIO_ReadPin(VFD_EN_GPIO_Port, VFD_EN_Pin))
		{
      EasyUITransitionAnim();
		}
		mn11236_clear_buffers();
		mn11236_send_buffer();
		HAL_GPIO_WritePin(VFD_EN_GPIO_Port, VFD_EN_Pin, GPIO_PIN_RESET);
		osDelay(200);
		return;
	}
	if(!HAL_GPIO_ReadPin(VFD_EN_GPIO_Port, VFD_EN_Pin))
	{
		opnUp = opnDown = opnEnter = opnExit = 0;
	}
  HAL_GPIO_WritePin(VFD_EN_GPIO_Port, VFD_EN_Pin, GPIO_PIN_SET);
	
  switch (disp_mode)
  { 
  case DISP_MODE_CLOCK:
    if (timeupdateflag != timeStr[4])
    {
      EasyUITransitionAnim();
      timeupdateflag = timeStr[4];
    }
    clear_gram(White);
		
		set_g_maxGray(pUserInfo->clockdimming + 3);
		if(clockTheme[pUserInfo->clocktheme].type == 0)
		{
			gradient_flow_frame_clock();
			set_g_clockWidth(3);
			draw_str_3x5_wave((SCREEN_WIDTH / 3 - (3 * 5)) / 2 - 3, 0, timeStr, clockTheme[pUserInfo->clocktheme].font);
			set_g_clockWidth(1);
			draw_str_3x5_wave(SCREEN_WIDTH - 24, 12, date, clockTheme[pUserInfo->clocktheme].font);
		}
		else if(clockTheme[pUserInfo->clocktheme].type == 1)
		{
			set_g_clockWidth(3);
			draw_str_3x5((SCREEN_WIDTH / 3 - (3 * 5)) / 2 - 3, 0, timeStr, clockTheme[pUserInfo->clocktheme].font);
//			set_g_clockWidth(1);
//			draw_str_3x5(SCREEN_WIDTH - 24, 12, date, clockTheme[pUserInfo->clocktheme].font);
		}
		else if(clockTheme[pUserInfo->clocktheme].type == 2)
		{
			set_g_clockWidth(3);
			draw_str_3x5_wave((SCREEN_WIDTH / 3 - (3 * 5)) / 2 - 3, 0, timeStr, clockTheme[pUserInfo->clocktheme].font);
//			set_g_clockWidth(1);
//			draw_str_3x5(SCREEN_WIDTH - 24, 12, date, clockTheme[pUserInfo->clocktheme].font);
		}
//		EasyUIDisplayStr(0,0,timeStr);
    roll_all_long_str();
    refrash_gram();
    break;
  case DISP_MODE_FFT:
    clear_gram(White);
    draw_fft(lBufMagArray, NPT / 2, 20, musicTheme[pUserInfo->musictheme].bar, musicTheme[pUserInfo->musictheme].dot);
    refrash_gram();
    break;
  case DISP_MODE_SNAKE:
    clear_gram(White);
    refrashSnake();
    osDelay(50);
    refrash_gram();
    break;
  case DISP_MODE_MOTION:
    clear_gram(White);
    Motion_Run(&pUserInfo->effectswitch);
    refrash_gram();
    break;
  case DISP_MODE_EXTERN:

    break;
  default:
    EasyUIEvent(5);
    break;
  }
//	if((rolling++)%2)
		UploadFrame();
	UserInfo_AutoSave();

	// 菜单超时退出
	if (disp_mode == DISP_MODE_MENU)
	{
			if (now - last_key_tick > MENU_TIMEOUT)
			{
					disp_mode = last_disp_mode;
					EasyUITransitionAnim();
					last_key_tick = now;
			}
	}

  if (disp_mode != DISP_MODE_MENU)
  {
    if (opnUp)
    {
      disp_mode++;
      if (disp_mode > DISP_MODE_MOTION)
        disp_mode = DISP_MODE_CLOCK;
      EasyUITransitionAnim();
      opnUp = false;
    }
    if (opnDown)
    {
      if (disp_mode == DISP_MODE_CLOCK)
        disp_mode = DISP_MODE_MOTION;
      else
        disp_mode--;
      EasyUITransitionAnim();
      opnDown = false;
    }
    if (opnEnter)
    {
      last_disp_mode = disp_mode;
      EasyUITransitionAnim();
      disp_mode = DISP_MODE_MENU;
      opnEnter = false;
    }
    if (opnExit)
    {
      if (disp_mode == DISP_MODE_FFT)
        pUserInfo->musictheme = (pUserInfo->musictheme + 1) % 12;
      else if (disp_mode == DISP_MODE_CLOCK)
        pUserInfo->clocktheme = (pUserInfo->clocktheme + 1) % 5;
      else if (disp_mode == DISP_MODE_SNAKE)
      {
        pUserInfo->snakewidth = (pUserInfo->snakewidth + 1) % 5;
        if (pUserInfo->snakewidth == 0)
          pUserInfo->snakewidth = 1;
        setSnakeWidth(pUserInfo->snakewidth);
      }
      opnExit = false;
    }
  }
  else if (layer == 0 && opnExit)
  {
    disp_mode = last_disp_mode;
    opnExit = false;
    EasyUITransitionAnim();
  }
}
