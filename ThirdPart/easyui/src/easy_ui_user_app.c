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

/* OBD data from main.c */
extern obd_data_t obd_data;

// Pages
EasyUIPage_t pageMain, pageUSBForm, pageDialog, pageSensor, pageAnimation, pageSetting, pageDiagnostic, pageAbout, pageSetTime;

// Items
EasyUIItem_t itemUSBForm;
EasyUIItem_t itemDebug;
EasyUIItem_t itemSensor;
EasyUIItem_t itemAnimation;
EasyUIItem_t itemSetting, titleSetting, itemMusicTheme, itemClockTheme, itemSetTime, itemDefaultFunc, itemClockDimming;
EasyUIItem_t itemEspEn;
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

char *EasyUIVersion = "Ver. 3.3.0";
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

      uint8_t red = (p >> 4) & 0x07;
      uint8_t blue = p & 0x07;

      mn11236_draw_pixel_gray(x, y, MN11236_COLOR_RED, red);
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

      if (v < 0)
        v = 0;
      if (v > 1)
        v = 1;

      /* 映射 */
      uint8_t blue = (uint8_t)(v * 7.0f);         // 3bit
      uint8_t red = (uint8_t)((1.0f - v) * 7.0f); // 3bit

      frame_wave[x][y] = (red << 4) | (blue & 0x07);
    }
  }

  t1 += SPEED1;
  t2 += SPEED2;
  t3 += SPEED3;

  flush_frame();
}

void PageDiagnostic(EasyUIItem_t *page)
{
  static uint8_t mode = 0;
  static uint8_t gray = 0;
  static int8_t dir = 1;
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
      uint8_t g = (i < 8) ? i : (i - 8);

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
  mn11236_setbrightness(0);
  xSemaphoreTake(flashMutex, portMAX_DELAY);
  taskENTER_CRITICAL();
  STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)pUserInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
  taskEXIT_CRITICAL();
  xSemaphoreGive(flashMutex);
  mn11236_setbrightness(100);
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

static uint32_t last_key_tick = 0;
#define MENU_TIMEOUT 10000

static uint32_t wake_until_tick = 0; // 临时亮屏截止时间
#define WAKE_TIME_MS 10000

static SysDisplayMode_e last_disp_mode = DISP_MODE_MOTION;
SysDisplayMode_e disp_mode = DISP_MODE_MOTION;
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
  EasyUIAddItem(&pageSetting, &itemEspEn, "ESP EN", ITEM_CHECKBOX, &pUserInfo->esp_en);
  EasyUIAddItem(&pageSetting, &itemMusicTheme, "FFT Theme", ITEM_PROGRESS_BAR, &pUserInfo->musictheme, 11, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemClockTheme, "Clock Theme", ITEM_PROGRESS_BAR, &pUserInfo->clocktheme, CLOCK_THEME_COUNT - 1, EventChangeValue);
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
    // 动画背景 + 波形文字（style: 0=标准字模 1=原生 9x15 字模）
    {Red, Red, 0, CLOCK_BG_WAVE, 0},       // 三波叠加渐变
    {Red, Red, 0, CLOCK_BG_WAVE, 1},       // 三波叠加渐变
    {Cyan, White, 0, CLOCK_BG_WAVE, 0},    // 三波叠加渐变（白字）
    {Cyan, White, 0, CLOCK_BG_WAVE, 1},    // 三波叠加渐变（白字）
    {Cyan, White, 0, CLOCK_BG_BANDS, 0},   // 流动光带
    {Cyan, White, 0, CLOCK_BG_BANDS, 1},   // 流动光带
    {Cyan, White, 0, CLOCK_BG_GLOW, 0},    // 中心光晕
    {Cyan, White, 0, CLOCK_BG_GLOW, 1},    // 中心光晕
    {Cyan, White, 0, CLOCK_BG_WIPE, 0},    // 光条擦除染色（蓝/红交替）
    {Cyan, White, 0, CLOCK_BG_WIPE, 1},    // 光条擦除染色（蓝/红交替）
    {Cyan, White, 0, CLOCK_BG_FLOW, 0},    // 流动渐变
    {Cyan, White, 0, CLOCK_BG_FLOW, 1},    // 流动渐变
    {Cyan, White, 0, CLOCK_BG_PLASMA, 0},  // 等离子斑块
    {Cyan, White, 0, CLOCK_BG_PLASMA, 1},  // 等离子斑块
    {Cyan, White, 0, CLOCK_BG_BREATHE, 0}, // 整屏明暗呼吸
    {Cyan, White, 0, CLOCK_BG_BREATHE, 1}, // 整屏明暗呼吸
    // 纯字主题（无背景动画）
    {Red, WhiteWithCyan, 1, CLOCK_BG_NONE, 0},
    {Red, WhiteWithCyan, 1, CLOCK_BG_NONE, 1},
    {Cyan, WhiteWithRed, 1, CLOCK_BG_NONE, 0},
    {Cyan, WhiteWithRed, 1, CLOCK_BG_NONE, 1},
	};

/* 主题数量必须与 CLOCK_THEME_COUNT 一致 */
typedef char clock_theme_count_check[
    (sizeof(clockTheme) / sizeof(clockTheme[0]) == CLOCK_THEME_COUNT) ? 1 : -1];

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

  uint8_t on = pUserInfo->screen_on_hour;
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
extern uint8_t obd_display_index;

static const char *obd_display_placeholders[8] = {
    "----", // RPM
    " ---", // SPEED
    " ---", // Coolant Temp
    " ---", // Intake Temp
    " ---", // Load
    " ---", // Throttle
    "--.-", // Voltage
    " ---"  // Oil Temp
};

typedef struct
{
  uint8_t icon;
  bool has_icon;
} OBD_DisplayCfg_t;

static const OBD_DisplayCfg_t obd_display_cfg[8] = {
    {ICON_RPM, true},
    {ICON_KMH, true},
    {ICON_TEMP, true},
    {ICON_TEMP, true},
    {ICON_LOAD, true},
    {ICON_LOAD, true},
    {ICON_VOLT, true},
    {ICON_TEMP, true},
};
#define SMOOTH_DURATION_MS 300.0f
#define MIN_UPDATE_MS 50  // 最小更新周期

static float linear_smooth(float current, float target, float dt_ms, float duration_ms)
{
    float diff = target - current;
    float delta = diff * dt_ms / duration_ms;

		if(dt_ms > duration_ms)
			return target;
    return current + delta;
}

static void OBD_FormatText(char *buf, size_t size, uint8_t index)
{
    static uint32_t last_time = 0;

    uint32_t now = HAL_GetTick();

    // 第一次调用，初始化 last_time
    if (last_time == 0)
        last_time = now;

    uint32_t dt_ms = now - last_time;

    // 如果距离上次更新小于最小更新周期，则不更新显示值
    if (dt_ms < MIN_UPDATE_MS)
        dt_ms = 0; // 线性平滑函数返回 current，不变化
    else
		{
				dt_ms = 60;
        last_time = now; // 更新时间戳
		}
		
    switch (index)
    {
    case 0: // RPM
        if (dt_ms > 0)
            obd_data.rpm_display = linear_smooth(obd_data.rpm_display, obd_data.rpm, dt_ms, SMOOTH_DURATION_MS);
        snprintf(buf, size, "%4d", (int)(obd_data.rpm_display + 0.5f));
        break;

    case 1: // Speed
        if (dt_ms > 0)
            obd_data.speed_display = linear_smooth(obd_data.speed_display, obd_data.speed, dt_ms, SMOOTH_DURATION_MS);
        snprintf(buf, size, "%4d", (int)(obd_data.speed_display + 0.5f));
        break;

    case 4: // Load %
        if (dt_ms > 0)
            obd_data.load_display = linear_smooth(obd_data.load_display, obd_data.load_pct, dt_ms, SMOOTH_DURATION_MS);
        snprintf(buf, size, "%4d", (int)(obd_data.load_display + 0.5f));
        break;

    case 5: // Throttle %
        if (dt_ms > 0)
            obd_data.throttle_display = linear_smooth(obd_data.throttle_display, obd_data.throttle_pct, dt_ms, SMOOTH_DURATION_MS);
        snprintf(buf, size, "%4d", (int)(obd_data.throttle_display + 0.5f));
        break;

    case 2: // Coolant temp
        snprintf(buf, size, "%4d", obd_data.coolant_temp);
        break;

    case 3: // Intake temp
        snprintf(buf, size, "%4d", obd_data.intake_temp);
        break;

    case 6: // Voltage
        snprintf(buf, size, "%2d.%d", obd_data.voltage / 100, (obd_data.voltage % 100) / 10);
        break;

    case 7: // Oil temp
        snprintf(buf, size, "%4d", obd_data.oil_temp);
        break;

    default:
        buf[0] = '\0';
        break;
    }
}
static void OBD_DrawText(char *text, uint8_t index)
{
  const ClockTheme_t *theme = &clockTheme[pUserInfo->clocktheme];
  const OBD_DisplayCfg_t *cfg = &obd_display_cfg[index];
  int x = (SCREEN_WIDTH / 3 - (3 * 5)) / 2 - 2;

  clock_set_font_style(theme->style);

  if (cfg->has_icon && theme->type == 0)
  {
    if (theme->bg != CLOCK_BG_NONE)
      clock_draw_background(theme->bg);
    set_g_clockWidth(3);
    draw_str_3x5_wave(x, 0, text, theme->font);
    set_g_clockWidth(1);
    draw_icon_wave(SCREEN_WIDTH - 30, 12 + 2, cfg->icon, theme->font);
  }
  else if (cfg->has_icon && theme->type == 1)
  {
    set_g_clockWidth(3);
    draw_str_3x5(x, 0, text, theme->font);
    set_g_clockWidth(1);
    draw_icon(SCREEN_WIDTH - 30, 12 + 2, cfg->icon, theme->font);
  }
  else if (theme->type == 2)
  {
    set_g_clockWidth(3);
    draw_str_3x5_wave(x, 0, text, theme->font);
  }
  else
  {
    set_g_clockWidth(3);
    draw_str_3x5(x, 0, text, theme->font);
  }
}
#define OBD_TIMEOUT_MS 2000
/* 冒号错时呼吸：1Hz 周期，上点先亮/先灭，下点滞后 100ms */
#define COLON_FADE_PERIOD_MS 1000
#define COLON_FADE_TIME_MS 100
/* 时间切换：旧数字碎掉落下，新数字从上方落下拼合（和菜单切换同一套动画） */
#define SHARD_FALL_MS 210
#define SHARD_BUILD_MS 210
#define SHARD_TOTAL_MS (SHARD_FALL_MS + SHARD_BUILD_MS)

/* 计算冒号上下两点当前亮度（0..7）：上点先亮 100ms，再亮下点；熄灭时上点先灭 */
static void ColonLevels(uint32_t now, uint8_t *top, uint8_t *bot)
{
  uint32_t t = now % COLON_FADE_PERIOD_MS;

  if (t < COLON_FADE_TIME_MS) /* 上点渐亮 */
  {
    *top = (uint8_t)(7 * t / (COLON_FADE_TIME_MS - 1));
    *bot = 0;
  }
  else if (t < 2 * COLON_FADE_TIME_MS) /* 下点渐亮 */
  {
    *top = 7;
    *bot = (uint8_t)(7 * (t - COLON_FADE_TIME_MS) / (COLON_FADE_TIME_MS - 1));
  }
  else if (t < 550) /* 都亮 */
  {
    *top = 7;
    *bot = 7;
  }
  else if (t < 650) /* 上点渐灭 */
  {
    *top = (uint8_t)(7 * (650 - t) / (COLON_FADE_TIME_MS - 1));
    *bot = 7;
  }
  else if (t < 750) /* 下点渐灭 */
  {
    *top = 0;
    *bot = (uint8_t)(7 * (750 - t) / (COLON_FADE_TIME_MS - 1));
  }
  else /* 都灭 */
  {
    *top = 0;
    *bot = 0;
  }
}

/* 碎屏过渡：旧画面碎裂落地，下一帧再把新画面从上方落下拼合出来 */
static uint8_t pending_rebuild = 0;

static void TransitionShatter(void)
{
  clock_shatter_anim();
  pending_rebuild = 1;
  /* 新界面这一帧先不送屏，交给拼合动画自己送，避免“空屏后闪一帧完整的图” */
  mn11236_hold_send(1);
}

void AppGUI_Loop(uint8_t timer)
{
  static char tempstr[16];
  static int rolling = 0;
  static int rpm = 0;
  /* 翻页（碎裂/拼合）动画状态 */
  static char shardPrev[6] = {0};
  static uint32_t shardStart = 0;
  static uint8_t shardActive = 0;

  //	mn11236_gray_test();
  //	osDelay(5000);
  //	return;

  uint32_t now = HAL_GetTick();

  bool night = !ScreenShouldOn();

  // 有按键 → 唤醒60秒
  if (opnUp || opnDown || opnEnter || opnExit)
  {
    last_key_tick = now;

    if (night)
    {
      wake_until_tick = now + WAKE_TIME_MS;
    }
  }

  // 夜间且未唤醒
  if (night && now > wake_until_tick && disp_mode != DISP_MODE_MENU && disp_mode != DISP_MODE_OBD)
  {
    if (HAL_GPIO_ReadPin(VFD_EN_GPIO_Port, VFD_EN_Pin))
    {
      EasyUITransitionAnim();
    }
    /* 熄屏前确保送屏没有被过渡动画挂起 */
    pending_rebuild = 0;
    mn11236_hold_send(0);
    mn11236_clear_buffers();
    mn11236_send_buffer();
    HAL_GPIO_WritePin(VFD_EN_GPIO_Port, VFD_EN_Pin, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_RESET);
    osDelay(200);
    return;
  }
  if (!HAL_GPIO_ReadPin(VFD_EN_GPIO_Port, VFD_EN_Pin))
  {
    opnUp = opnDown = opnEnter = opnExit = 0;
  }
  HAL_GPIO_WritePin(VFD_EN_GPIO_Port, VFD_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, pUserInfo->esp_en);

  switch (disp_mode)
  {
  case DISP_MODE_CLOCK:
  {
    clear_gram(White);

    set_g_maxGray(pUserInfo->clockdimming + 3);

    /* 冒号两点错时呼吸：上点先亮、先灭，下点滞后 100ms */
    uint8_t colonTop, colonBot;
    ColonLevels(now, &colonTop, &colonBot);

    int timeX = (SCREEN_WIDTH / 3 - (3 * 5)) / 2 - 3;
    const ClockTheme_t *theme = &clockTheme[pUserInfo->clocktheme];
    Color fontColor = theme->font;

    /* 主题字体：0=标准 3x5 放大，1=原生 9x15 */
    clock_set_font_style(theme->style);

    char timeBuf[8];
    timeBuf[0] = timeStr[0];
    timeBuf[1] = timeStr[1];
    timeBuf[2] = ' ';
    timeBuf[3] = timeStr[3];
    timeBuf[4] = timeStr[4];
    timeBuf[5] = '\0';

    /* 时间变化 → 碎裂动画（旧数字碎掉，新数字从上方拼合） */
    if (shardPrev[0] == '\0')
      memcpy(shardPrev, timeBuf, 5); /* 首次上电不播动画 */
    else if (!shardActive && memcmp(shardPrev, timeBuf, 5) != 0)
    {
      shardActive = 1;
      shardStart = now;
    }

    if (shardActive && (now - shardStart) >= SHARD_TOTAL_MS)
    {
      shardActive = 0;
      memcpy(shardPrev, timeBuf, 5);
    }

    uint8_t shardPhase = 0; /* 0=正常 1=旧字碎掉 2=新字拼合 */
    uint16_t shardEl = 0;

    if (shardActive)
    {
      uint32_t el = now - shardStart;

      if (el < SHARD_FALL_MS)
      {
        shardPhase = 1;
        shardEl = (uint16_t)el;
      }
      else
      {
        shardPhase = 2;
        shardEl = (uint16_t)(el - SHARD_FALL_MS);
        if (shardEl > SHARD_BUILD_MS)
          shardEl = SHARD_BUILD_MS;
      }
    }

    // 背景动画
    if (theme->bg != CLOCK_BG_NONE)
      clock_draw_background(theme->bg);

    if (theme->type == 0)
    {
      set_g_clockWidth(3);

      if (shardPhase == 0)
      {
        draw_str_3x5_wave(timeX, 0, timeBuf, fontColor);
      }
      else
      {
        for (int i = 0; i < 5; i++)
        {
          char shown = (shardPhase == 1) ? shardPrev[i] : timeBuf[i];

          if (shardPrev[i] != timeBuf[i])
            draw_char_shatter(timeX + i * 4, 0, shown, fontColor, 1,
                              (shardPhase == 1) ? 0 : 1, shardEl);
          else
            draw_char_3x5_wave(timeX + i * 4, 0, shown, fontColor);
        }
      }

      draw_colon_fade(timeX + 8, 0, fontColor, colonTop, colonBot, 1);

      /* 原生字体主题不显示日期，只留时间 */
      if (theme->style == 0)
      {
        set_g_clockWidth(1);
        draw_str_3x5_wave(SCREEN_WIDTH - 24, 12, date, fontColor);
      }
    }
    else
    {
      set_g_clockWidth(3);

      if (shardPhase == 0)
      {
        draw_str_3x5(timeX, 0, timeBuf, fontColor);
      }
      else
      {
        for (int i = 0; i < 5; i++)
        {
          char shown = (shardPhase == 1) ? shardPrev[i] : timeBuf[i];

          if (shardPrev[i] != timeBuf[i])
            draw_char_shatter(timeX + i * 4, 0, shown, fontColor, 0,
                              (shardPhase == 1) ? 0 : 1, shardEl);
          else
            draw_char_3x5(timeX + i * 4, 0, shown, fontColor);
        }
      }

      draw_colon_fade(timeX + 8, 0, fontColor, colonTop, colonBot, 0);
    }
    roll_all_long_str();
    refrash_gram();
    break;
  }
  case DISP_MODE_OBD:
  {
    uint8_t i = obd_display_index;

    if (obd_data.valid & (1u << i))
    {
      if (now - obd_data.last_update_tick[i] > OBD_TIMEOUT_MS)
      {
        obd_data.valid &= ~(1u << i);
      }
    }

    if (obd_data.valid & (1u << i))
    {
      OBD_FormatText(tempstr, sizeof(tempstr), i);
    }
    else
    {
      snprintf(tempstr, sizeof(tempstr), "%s", obd_display_placeholders[i]);
    }

    clear_gram(White);
    set_g_maxGray(pUserInfo->clockdimming + 3);
    OBD_DrawText(tempstr, i);

    roll_all_long_str();
    refrash_gram();
  }
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

  /* 碎屏之后：把新画面从上方落下拼合回来 */
  if (pending_rebuild)
  {
    pending_rebuild = 0;
    mn11236_hold_send(0); /* 恢复送屏，交给拼合动画 */
    clock_shatter_rebuild();
  }

  //	if((rolling++)%2)
  UploadFrame();
  UserInfo_AutoSave();

  // 菜单超时退出
  if (disp_mode == DISP_MODE_MENU)
  {
    if (now - last_key_tick > MENU_TIMEOUT)
    {
      /* 超时退出菜单也走碎裂落地过渡 */
      TransitionShatter();
      disp_mode = last_disp_mode;
      last_key_tick = HAL_GetTick();
    }
  }

  if (disp_mode != DISP_MODE_MENU)
  {
    if (opnUp)
    {
      /* 先让当前画面碎裂落地，再切到下一个界面 */
      TransitionShatter();

      disp_mode++;
      if (disp_mode > DISP_MODE_MOTION)
        disp_mode = DISP_MODE_CLOCK;

      if (!pUserInfo->esp_en)
      {
        if (disp_mode == DISP_MODE_OBD)
          disp_mode++;
      }

      opnUp = false;
    }
    if (opnDown)
    {
      /* 先让当前画面碎裂落地，再切到上一个界面 */
      TransitionShatter();

      if (disp_mode == DISP_MODE_CLOCK)
        disp_mode = DISP_MODE_MOTION;
      else
        disp_mode--;

      if (!pUserInfo->esp_en)
      {
        if (disp_mode == DISP_MODE_OBD)
          disp_mode--;
      }

      opnDown = false;
    }

    if (opnEnter)
    {
      last_disp_mode = disp_mode;
      /* 当前画面碎裂落地，再从空屏切进菜单 */
      TransitionShatter();
      disp_mode = DISP_MODE_MENU;
      last_key_tick = HAL_GetTick(); // 重置菜单超时计时器
      opnEnter = false;
    }
    if (opnExit)
    {
      if (disp_mode == DISP_MODE_FFT)
        pUserInfo->musictheme = (pUserInfo->musictheme + 1) % 12;
      else if (disp_mode == DISP_MODE_CLOCK || disp_mode == DISP_MODE_OBD)
        pUserInfo->clocktheme = (pUserInfo->clocktheme + 1) % CLOCK_THEME_COUNT;
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
    /* 从菜单退回原界面：同样用画面碎裂落地过渡 */
    TransitionShatter();
    disp_mode = last_disp_mode;
    opnExit = false;
    last_key_tick = HAL_GetTick();
  }
}
