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
#include "iwdg.h"

// Pages
EasyUIPage_t pageMain, pageUSBForm, pageDialog, pageSensor, pageSetting, pageDiagnostic, pageAbout, pageSetTime;

// Items
EasyUIItem_t itemUSBForm;
EasyUIItem_t itemDebug;
EasyUIItem_t itemSensor;
EasyUIItem_t itemSetting, titleSetting, itemMusicTheme, itemMusicColor, itemClockTheme, itemSetTime, itemDefaultFunc, itemClockDimming;
EasyUIItem_t itemEspEn;
EasyUIItem_t itemDiagnostic;
EasyUIItem_t itemAbout;
EasyUIItem_t itemMotion;
EasyUIItem_t itemGame;
EasyUIItem_t itemScreenOnHour;
EasyUIItem_t itemScreenOffHour;

EasyKey_t keyUp, keyDown;

extern UserInfo_t *pUserInfo;

/* 自动演示模式：中断里只置请求，真正的页面切换和 FLASH 保存放在显示任务里做。
 * auto_enable_req 由“按住下键 5 秒”触发；auto_cancel_req 由任意按键触发。
 * auto_enable_release 用来忽略“开启自动模式那一次长按”的松手事件，
 * 否则松手产生的 opnUp 会立刻把刚打开的自动模式取消掉。 */
volatile uint8_t auto_enable_req = 0;
volatile uint8_t auto_cancel_req = 0;
static uint8_t auto_hold_armed = 0;
static uint8_t auto_enable_release = 0;
static uint8_t keyup_reset_armed = 0;
static uint8_t auto_cancel_swallow = 0; /* 取消自动模式的那次按键要吞到松手 */

/*!
 * @brief   Sync the operation bool value
 *
 * @param   void
 * @return  void
 */
void EasyUIKeyActionMonitor() // Interrupt trigger, No EasyUIDelay_ms(xx)
{
  uint8_t automode = (pUserInfo != NULL) ? pUserInfo->automode : 0;

  /* 取消自动模式的那一次按键整次吞掉：按下/长按/松手都不触发
   * 翻页、进菜单等功能，直到两个键都回到 released。 */
  if (auto_cancel_swallow)
  {
    if (keyUp.state == released && keyDown.state == released)
      auto_cancel_swallow = 0;
    else
      return;
  }

  if (keyUp.isPressed)
  {
    if (automode)
    {
      auto_cancel_req = 1;
      auto_cancel_swallow = 1;
    }
    else
      opnEnter = 1;
    dbmsg("keyUp:isPressed");
  }
  else if (keyUp.isHold)
  {
    if (automode)
    {
      auto_cancel_req = 1;
      auto_cancel_swallow = 1;
    }
    else
      opnExit = 1;
    dbmsg("keyUp:isHold");
  }
  else if (keyUp.state == pressed && keyUp.holdTime > 5000)
  {
    if (automode)
    {
      /* 自动模式下“任意键取消”，长按上键也按取消处理，不重启。
       * 这里同时置 keyup_reset_armed：取消后如果手还没松，
       * 下一轮不能因为 automode 已经变 0 就重启。 */
      auto_cancel_req = 1;
      auto_cancel_swallow = 1;
      keyup_reset_armed = 1;
    }
    else if (!keyup_reset_armed)
    {
      keyup_reset_armed = 1;
      dbmsg("keyUp:holdTime:%d", keyDown.holdTime);
      HAL_NVIC_SystemReset();
    }
  }
  if (keyDown.isPressed)
  {
    if (automode)
    {
      auto_cancel_req = 1;
      auto_cancel_swallow = 1;
    }
    else
      opnDown = 1;
    dbmsg("keyDown:isPressed");
  }
  else if (keyDown.isHold)
  {
    if (auto_enable_release)
    {
      /* 这是“长按 5 秒开启自动模式”之后的松手，吞掉，不当作换页 */
      auto_enable_release = 0;
    }
    else if (automode)
    {
      auto_cancel_req = 1;
      auto_cancel_swallow = 1;
    }
    else
      opnUp = 1;
    dbmsg("keyDown:isHold");
  }
  else if (keyDown.state == pressed && keyDown.holdTime > 5000)
  {
    if (!auto_hold_armed)
    {
      auto_hold_armed = 1;
      auto_enable_release = 1;
      if (automode)
      {
        auto_cancel_req = 1; /* 已经在自动模式里：5 秒长按也当取消 */
        auto_cancel_swallow = 1;
      }
      else
        auto_enable_req = 1;
      dbmsg("keyDown:holdTime:%d", keyDown.holdTime);
    }
  }

  /* 松开后重新武装，下一次长按才能再次触发 */
  if (keyDown.state != pressed)
    auto_hold_armed = 0;
  if (keyDown.state == down || keyDown.state == released)
    auto_enable_release = 0; /* 防止漏掉松手事件后一直吞按键 */
  if (keyUp.state == down)
    keyup_reset_armed = 0; /* 只有新按下才重新允许 5 秒重启 */

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

char *EasyUIVersion = "Ver. 3.6.0";
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
  /* 存数据期间不刷屏：scan_stop 内部会先刷一帧全 0 再停，
   * 所以屏上定格的是一片黑，不会留亮线。写完恢复扫描并立刻把画面送回。 */
  mn11236_scan_stop();

  xSemaphoreTake(flashMutex, portMAX_DELAY);
  STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)pUserInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
  if (firstsave)
  {
    firstsave = false;
    /* 第一次写有失败的情况，补写一次；仍在同一次停扫窗口里 */
    STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)pUserInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
  }
  xSemaphoreGive(flashMutex);

  /* 恢复扫描，并把原来那一帧重新写回 DMA 缓冲 */
  mn11236_scan_start();
  mn11236_send_buffer();
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

static UserInfo_t userinfoLast;
static bool userinfoLastInited = false;

static void UserInfo_CacheSync(void)
{
  memcpy(&userinfoLast, pUserInfo, sizeof(UserInfo_t));
  userinfoLastInited = true;
}

/* 强制落盘：不走“和上次缓存比较”的路径。
 * 开启自动模式这种必须立刻保存的状态用它，避免第一次调用
 * UserInfo_AutoSave() 只建缓存、不写 FLASH 的问题。 */
static void UserInfo_SaveNow(void)
{
  if (pUserInfo == NULL)
    return;

  UserInfo_CacheSync();
  HAL_IWDG_Refresh(&hiwdg);
  SaveInfo();
}

bool UserInfo_AutoSave()
{
  if (pUserInfo == NULL)
    return false;

  if (!userinfoLastInited)
  {
    UserInfo_CacheSync();
    return false;
  }

  if (memcmp(&userinfoLast, pUserInfo, sizeof(UserInfo_t)) != 0)
  {
    UserInfo_CacheSync();
		HAL_IWDG_Refresh(&hiwdg);
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

/* ---------------- 自动演示模式 ---------------- */
#define AUTO_SWITCH_PERIOD_MS (3U * 60U * 1000U) /* 3 分钟换一次动画/频谱风格 */
#define AUTO_MUSIC_TIMEOUT_MS 10000U             /* 10 秒没音乐回 MOTION */
#define AUTO_MUSIC_CONFIRM_VOTES 8               /* 100ms 采样一次，连续 0.8 秒确认 */
#define AUTO_DETECT_PERIOD_MS 100U
#define AUTO_MUSIC_MIN_LEVEL 120U  /* 三个频段峰值之和的下限 */
#define AUTO_MUSIC_BAND_MIN 80U    /* 中频至少要有约 4 根柱子的峰值 */
#define AUTO_MUSIC_STRONG_LEVEL 300U /* 响度足够大时的快速确认门限 */
#define AUTO_MUSIC_VOTE_MAX 30

static uint32_t auto_switch_tick = 0;
static uint32_t auto_music_last = 0;
static uint32_t auto_detect_tick = 0;
static uint8_t auto_music_on = 0;
static uint8_t auto_music_votes = 0;
static uint8_t auto_styles_left = FFT_STYLE_COUNT;

/* 自动模式进出提示：0=无，1=AUTO ON，2=AUTO OFF */
static uint8_t auto_osd_req = 0;
static uint8_t auto_osd_snap[VFD_WIDTH][VFD_HEIGHT];

/* 动画转场：黑场里把新动画的第一帧画好（滑入时用这张快照） */
static uint8_t motion_build_id;
static void motion_build_first_frame(void)
{
  clear_gram(White);
  Motion_Run(motion_build_id);
}

/* 菜单里用文字显示的选项名（6x8 字模，最多 6 个字符：
 * 列表框里的高亮框宽度由标题长度决定，太长会压到右边的高亮边框上）
 * motionNames 的顺序必须和 animation.c 里的 anims[] 一一对应 */
static const char *const motionNames[ANIM_COUNT] = {
    "Glow", "Ripple", "Matrix", "Aurora", "City", "Warp", "Bubble", "Burst",
    "Helix", "Harbor", "Tunnel", "Runway", "Radar", "Scope", "Bridge", "Sky",
};

/* 0 = 贪吃蛇 1 = 俄罗斯方块 2 = 坦克大战 */
static const char *const gameNames[GAME_COUNT] = {"Snake", "Tetris", "Tanks"};

/* 这些名字右侧对齐显示，长度要控制住，别压到左边的标题：
 * FFT Style 标题结束在 x=65，最多 7 个字符；Clock Theme 结束在 x=77，最多 5 个；
 * Default Func 结束在 x=83，最多 4 个。 */
static const char *const fftStyleNames[FFT_STYLE_COUNT] = {
    "Bars", "Mirror", "Curve", "Dots", "Water", "Spark", "VU", "Split",
    "Fire", "Matrix", "Scope", "P-Bars", "Spiral", "Bounce", "Tunnel", "Firewrk"};
static const char *const fftColorNames[FFT_COLOR_COUNT] = {
    "CyanRed", "CyanWht", "Red", "RedWht", "WhtRed", "WhtCyan"};
static const char *const clockThemeNames[CLOCK_THEME_COUNT] = {
    "Wave", "Wave1", "WaveW", "WavW1", "Wipe", "Wipe1", "Flow",
    "Flow1", "Plas", "Plas1", "TextC", "TxtC1", "TextR", "TxtR1"};
static const char *const defaultFuncNames[4] = {"Clk", "FFT", "Game", "Anim"};

void AppGUI_Init(UserInfo_t *userinfo)
{
  InitInfo();
  pUserInfo = userinfo;

  last_disp_mode = (SysDisplayMode_e)pUserInfo->defaultfunc;
  disp_mode = (SysDisplayMode_e)pUserInfo->defaultfunc;

  if (pUserInfo->automode)
  {
    /* 上次是在自动模式里断电的：从 MOTION 继续演示 */
    uint32_t now = HAL_GetTick();
    disp_mode = DISP_MODE_MOTION;
    last_disp_mode = DISP_MODE_MOTION;
    auto_switch_tick = now;
    auto_detect_tick = now;
    auto_music_last = now;
    auto_music_votes = 0;
    auto_music_on = 0;
    auto_styles_left = FFT_STYLE_COUNT;
    auto_osd_req = 1; /* 上电就是自动模式：也提示一次 AUTO ON */
    /* 上电自动模式也直接从黑场进入 AUTO ON，不要先闪一帧 MOTION */
    mn11236_hold_send(1);
  }
  else
  {
    auto_switch_tick = HAL_GetTick();
    auto_detect_tick = auto_switch_tick;
  }

  EasyUIAddPage(&pageMain, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSetting, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  //  EasyUIAddPage(&pageUSBForm, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageUSBForm);
  //  EasyUIAddPage(&pageDialog, GUI_6X8_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSensor, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageSensor);
  EasyUIAddPage(&pageDiagnostic, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageDiagnostic);
  EasyUIAddPage(&pageAbout, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageAbout);
  EasyUIAddPage(&pageSetTime, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageSetTime);

  //  EasyUIAddItem(&pageMain, &itemUSBForm, "USBForm", ITEM_JUMP_PAGE, pageUSBForm.id);
  //  EasyUIAddItem(&pageMain, &itemDebug, "Debug", ITEM_JUMP_PAGE, pageDialog.id);
  EasyUIAddItem(&pageMain, &itemSensor, "Sensor", ITEM_JUMP_PAGE, pageSensor.id);
  EasyUIAddItem(&pageMain, &itemSetting, "Setting", ITEM_JUMP_PAGE, pageSetting.id);
  EasyUIAddItem(&pageMain, &itemDiagnostic, "Diagnostic", ITEM_JUMP_PAGE, pageDiagnostic.id);
  EasyUIAddItem(&pageMain, &itemAbout, "<About>", ITEM_JUMP_PAGE, pageAbout.id);

  EasyUIAddItem(&pageSetting, &titleSetting, "[Setting]", ITEM_PAGE_DESCRIPTION);

  EasyUIAddItem(&pageSetting, &itemSetTime, "Set Time", ITEM_JUMP_PAGE, pageSetTime.id);
  EasyUIAddItem(&pageSetting, &itemMusicTheme, "FFT Style", ITEM_PROGRESS_LABEL, &pUserInfo->musictheme, FFT_STYLE_COUNT - 1, fftStyleNames, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemMusicColor, "FFT Color", ITEM_PROGRESS_LABEL, &pUserInfo->musiccolor, FFT_COLOR_COUNT - 1, fftColorNames, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemClockTheme, "Clock Theme", ITEM_PROGRESS_LABEL, &pUserInfo->clocktheme, CLOCK_THEME_COUNT - 1, clockThemeNames, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemDefaultFunc, "Default Func", ITEM_PROGRESS_LABEL, &pUserInfo->defaultfunc, 3, defaultFuncNames, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemGame, "Game", ITEM_PROGRESS_LABEL, &pUserInfo->game, GAME_COUNT - 1, gameNames, EventChangeValue);
  /* 动画单选：0..ANIM_COUNT-1，长按上键也能在 MOTION 界面里循环切换 */
  EasyUIAddItem(&pageSetting, &itemMotion, "Animation", ITEM_PROGRESS_LABEL, &pUserInfo->motion, ANIM_COUNT - 1, motionNames, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemClockDimming, "Clock Dimming", ITEM_PROGRESS_BAR, &pUserInfo->clockdimming, 4, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemScreenOnHour,
                "Screen ON", ITEM_PROGRESS_BAR,
                &pUserInfo->screen_on_hour, 23,
                EventChangeValue);

  EasyUIAddItem(&pageSetting, &itemScreenOffHour,
                "Screen OFF", ITEM_PROGRESS_BAR,
                &pUserInfo->screen_off_hour, 23,
                EventChangeValue);

  // Key init
  EasyKeyInit(&keyUp, GPIOB, KEY_SW1_Pin);
  EasyKeyInit(&keyDown, GPIOB, KEY_SW2_Pin);
  EasyUIInit(1);
  clock_init();
  snakeRestart(); /* 贪吃蛇宽度固定 3，顺便把第一局铺好 */
  Motion_Init();
  uint32_t now = HAL_GetTick();
  wake_until_tick = now + WAKE_TIME_MS;
}

/* FFT 配色：bar 是柱身/主色，dot 是峰值点/强调色
 * 8 种风格都按 1 像素列设计，所以这里不再有 2 像素宽的 *With* 组合 */
const MusicTheme_t musicTheme[FFT_COLOR_COUNT] = {
    {Cyan, Red},
    {Cyan, White},
    {Red, Red},
    {Red, White},
    {White, Red},
    {White, Cyan},
};

/* 频谱换风格转场：黑场里把新风格的第一帧画好（滑入时用这张快照）
 * 和 MOTION 的 motion_build_first_frame 一样，这里只画不送屏 */
static void fft_build_first_frame(void)
{
  const MusicTheme_t *mt = &musicTheme[pUserInfo->musiccolor % FFT_COLOR_COUNT];

  clear_gram(White);
  set_g_maxGray(pUserInfo->clockdimming + 3);
  draw_fft(lBufMagArray, NPT / 2, 20,
           (uint8_t)(pUserInfo->musictheme % FFT_STYLE_COUNT), mt->bar, mt->dot);
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
//    {Cyan, White, 0, CLOCK_BG_BANDS, 0},   // 流动光带
//    {Cyan, White, 0, CLOCK_BG_BANDS, 1},   // 流动光带
//    {Cyan, White, 0, CLOCK_BG_GLOW, 0},    // 中心光晕
//    {Cyan, White, 0, CLOCK_BG_GLOW, 1},    // 中心光晕
    {Cyan, White, 0, CLOCK_BG_WIPE, 0},    // 光条擦除染色（蓝/红交替）
    {Cyan, White, 0, CLOCK_BG_WIPE, 1},    // 光条擦除染色（蓝/红交替）
    {Cyan, White, 0, CLOCK_BG_FLOW, 0},    // 流动渐变
    {Cyan, White, 0, CLOCK_BG_FLOW, 1},    // 流动渐变
    {Cyan, White, 0, CLOCK_BG_PLASMA, 0},  // 等离子斑块
    {Cyan, White, 0, CLOCK_BG_PLASMA, 1},  // 等离子斑块
//    {Cyan, White, 0, CLOCK_BG_BREATHE, 0}, // 整屏明暗呼吸
//    {Cyan, White, 0, CLOCK_BG_BREATHE, 1}, // 整屏明暗呼吸
    // 纯字主题（无背景动画）
    {Cyan, WhiteWithCyan, 1, CLOCK_BG_NONE, 0},
    {Cyan, WhiteWithCyan, 1, CLOCK_BG_NONE, 1},
    {Red, WhiteWithRed, 1, CLOCK_BG_NONE, 0},
    {Red, WhiteWithRed, 1, CLOCK_BG_NONE, 1},
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

/* 覆写驱动里的 __weak 空实现：每送完一帧显存就同步给上位机。
 * 画在哪、什么时候画都不影响 —— 碎裂/拼合、菜单模糊等过渡动画
 * 的每一帧都会跟着上传，上位机看到的就是屏幕真实显示的内容。 */
void mn11236_on_frame_sent(void)
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

/* ---------------- 自动演示模式的音乐检测 ----------------
 * 频段划分（10kHz / 256 点，bin 宽约 39Hz）：
 *   bin 3~7    : 117~273Hz   贝斯、底鼓（跳过 50/60Hz 工频）
 *   bin 8~76   : 312~2969Hz  人声主要频段
 *   bin 77~119 : 3008~4648Hz 镲片、吉他/合成器高频
 * 说话时中频峰值明显占主导；音乐通常有低频鼓点或高频镲片，
 * 并且低频/高频峰值相对中频更高。安静房间用总峰值门限挡掉。 */
static uint8_t auto_music_detected(void)
{
  uint32_t low_max = 0, mid_max = 0, high_max = 0;

  for (int i = 3; i < 8; i++)
  {
    uint32_t v = (uint32_t)lBufMagArray[i];
    if (v > low_max)
      low_max = v;
  }
  for (int i = 8; i < 77; i++)
  {
    uint32_t v = (uint32_t)lBufMagArray[i];
    if (v > mid_max)
      mid_max = v;
  }
  for (int i = 77; i < 120; i++)
  {
    uint32_t v = (uint32_t)lBufMagArray[i];
    if (v > high_max)
      high_max = v;
  }

  uint32_t total_max = low_max + mid_max + high_max;

  if (total_max < AUTO_MUSIC_MIN_LEVEL)
    return 0;

  /* 中频几乎没有能量：更像低频嗡声/环境噪声，不可能是有旋律的音乐 */
  if (mid_max < AUTO_MUSIC_BAND_MIN)
    return 0;

  /* 纯说话：中频峰值远远压过低频和高频，尤其 3k 以上几乎没有能量。
   * 原来的 1.5 倍太容易把流行乐里正常的人声误判成说话，这里放宽到
   * 中频 > 5 倍低频、> 16 倍高频才否决。 */
  if (mid_max > low_max * 5 && mid_max > high_max * 16)
    return 0;

  /* 音乐：不再要求低频/高频“压过”中频，只要不弱太多就放行。
   * 满足任意一条即可。 */
  if (low_max >= AUTO_MUSIC_BAND_MIN &&
      low_max * 4 >= mid_max * 3 && high_max * 16 >= mid_max)
    return 1; /* 低频鼓点/贝斯够强，同时有一点高频，排除纯工频嗡声 */
  if (high_max >= AUTO_MUSIC_BAND_MIN / 2 &&
      high_max * 3 >= mid_max)
    return 1; /* 高频镲片/合成器 */
  if (low_max >= AUTO_MUSIC_BAND_MIN &&
      high_max >= AUTO_MUSIC_BAND_MIN / 2 &&
      low_max * 2 >= mid_max && high_max * 8 >= mid_max)
    return 1; /* 低频和高频都有能量：宽带音乐 */
  if (total_max >= AUTO_MUSIC_STRONG_LEVEL &&
      mid_max >= AUTO_MUSIC_BAND_MIN * 2 &&
      low_max >= AUTO_MUSIC_BAND_MIN &&
      high_max >= AUTO_MUSIC_BAND_MIN / 2)
    return 1; /* 响度足够，且低/中/高都有能量：大部分流行乐 */

  return 0;
}

static void auto_music_tick(uint32_t now)
{
  if (now - auto_detect_tick < AUTO_DETECT_PERIOD_MS)
    return;
  auto_detect_tick = now;

  if (auto_music_detected())
  {
    auto_music_last = now;
    if (auto_music_votes < AUTO_MUSIC_VOTE_MAX)
      auto_music_votes++;
    if (auto_music_votes >= AUTO_MUSIC_CONFIRM_VOTES)
      auto_music_on = 1;
  }
  else
  {
    if (auto_music_votes > 0)
      auto_music_votes--;
    if (auto_music_on && (now - auto_music_last >= AUTO_MUSIC_TIMEOUT_MS))
    {
      auto_music_on = 0;
      auto_music_votes = 0;
    }
  }
}

static void auto_mode_set(uint8_t on, uint32_t now)
{
  if (pUserInfo == NULL || pUserInfo->automode == (on ? 1 : 0))
    return;

  pUserInfo->automode = on ? 1 : 0;
  auto_osd_req = on ? 1 : 2; /* 进入/退出自动模式后显示 AUTO ON/OFF */

  if (on)
  {
    if (disp_mode == DISP_MODE_MENU)
      layer = 0;
    functionIsRunning = false;
    opnEnter = opnExit = opnUp = opnDown = 0;
    pending_rebuild = 0;

    /* 落盘会让屏幕先变黑。这里保持送屏挂起：SaveInfo() 恢复扫描后
     * 不要立刻把旧画面送回来，等 AUTO ON 提示的第一帧画好再恢复，
     * 避免“黑屏 -> 正常画面闪一下 -> OSD”的中间帧。 */
    mn11236_hold_send(1);

    /* 开启 auto 后第一时间强制落盘：不走 UserInfo_AutoSave() 的首次建缓存，
     * 确保刚打开 auto 就断电，重新上电仍然会进入自动模式。 */
    UserInfo_SaveNow();

    auto_switch_tick = now;
    auto_detect_tick = now;
    auto_music_last = now;
    auto_music_votes = 0;
    auto_music_on = 0;
    auto_styles_left = FFT_STYLE_COUNT;
    disp_mode = DISP_MODE_MOTION;
    last_disp_mode = DISP_MODE_MOTION;
  }
  else
  {
    /* 取消自动模式的那次按键已经被按键监视器吞掉；这里再清一次，
     * 防止同一轮里残留的按键动作继续触发翻页/进菜单。 */
    opnEnter = opnExit = opnUp = opnDown = 0;
    /* 如果退出时正好有碎屏转场挂着，也取消掉，否则 pending_rebuild
     * 会在 OSD 之前恢复送屏，又闪出一帧正常画面。 */
    pending_rebuild = 0;

    /* 和进入 auto 一样：落盘黑屏后挂起送屏，直接进入 AUTO OFF 提示 */
    mn11236_hold_send(1);

    /* 取消 auto 也立刻落盘：如果上电时本来就是 auto，UserInfo_AutoSave()
     * 的首次调用只会建缓存；这里强制写，保证断电后不会又回到 auto。 */
    UserInfo_SaveNow();
  }
  /* 开关 auto 都在这里立即落盘；自动运行中每 3 分钟的动画/风格切换
   * 不单独写 FLASH，取消时这一次强制写会把最新状态一起保存。 */
}

static void auto_mode_tick(uint32_t now)
{
  if (pUserInfo == NULL || !pUserInfo->automode)
    return;
  if (disp_mode == DISP_MODE_MENU || disp_mode == DISP_MODE_EXTERN)
    return;

  auto_music_tick(now);

  /* 自动模式只允许在 MOTION / FFT 两个页面里跑 */
  if (disp_mode != DISP_MODE_MOTION && disp_mode != DISP_MODE_FFT)
  {
    disp_mode = DISP_MODE_MOTION;
    last_disp_mode = DISP_MODE_MOTION;
    auto_switch_tick = now;
  }

  /* 有音乐 -> FFT；连续 10 秒没音乐 -> 回 MOTION */
  if (auto_music_on && disp_mode != DISP_MODE_FFT)
  {
    TransitionShatter();
    disp_mode = DISP_MODE_FFT;
    last_disp_mode = DISP_MODE_FFT;
    auto_switch_tick = now;
    fft_style_reset();
    return;
  }
  if (!auto_music_on && disp_mode == DISP_MODE_FFT)
  {
    TransitionShatter();
    disp_mode = DISP_MODE_MOTION;
    last_disp_mode = DISP_MODE_MOTION;
    auto_switch_tick = now;
    return;
  }

  if (now - auto_switch_tick < AUTO_SWITCH_PERIOD_MS)
    return;

  auto_switch_tick = now;

  if (disp_mode == DISP_MODE_MOTION)
  {
    /* 平常每 3 分钟换一个动画 */
    pUserInfo->motion = (pUserInfo->motion + 1) % ANIM_COUNT;
    motion_build_id = pUserInfo->motion;
    motion_switch_anim_out();
    motion_switch_anim_in(motion_build_first_frame);
  }
  else if (disp_mode == DISP_MODE_FFT)
  {
    /* 有音乐时每 3 分钟换一种频谱风格；一整轮风格换完后，
     * 下一次切换顺便换颜色，再从头循环。 */
    pUserInfo->musictheme = (pUserInfo->musictheme + 1) % FFT_STYLE_COUNT;
    if (auto_styles_left > 0)
      auto_styles_left--;
    if (auto_styles_left == 0)
    {
      pUserInfo->musiccolor = (pUserInfo->musiccolor + 1) % FFT_COLOR_COUNT;
      auto_styles_left = FFT_STYLE_COUNT;
    }
    fft_style_reset();
    motion_switch_anim_out();
    motion_switch_anim_in(fft_build_first_frame);
  }
}

/* 自动模式提示的外框 */
static void auto_osd_border(Color color)
{
  for (int x = 0; x < VFD_WIDTH; x++)
  {
    draw_point(x, 0, color);
    draw_point(x, VFD_HEIGHT - 1, color);
  }
  for (int y = 1; y < VFD_HEIGHT - 1; y++)
  {
    draw_point(0, y, color);
    draw_point(VFD_WIDTH - 1, y, color);
  }
}

/* 进入/退出自动模式的提示：
 * 清屏 -> 边框 + 文字从一侧滑入 -> 完整显示约 1 秒 -> 从另一侧滑出 -> 恢复原画面。
 * dir >= 0 从左边进、右边出（AUTO ON）；dir < 0 从右边进、左边出（AUTO OFF）。 */
static void auto_osd_show(const char *text, Color accent, int8_t dir)
{
  uint8_t *buf = mn11236_get_buffer();
  char line[12];
  int len = (int)strlen(text);

  if (len > (int)sizeof(line) - 1)
    len = (int)sizeof(line) - 1;
  memcpy(line, text, len);
  line[len] = '\0';

  memcpy(auto_osd_snap, buf, sizeof(auto_osd_snap));

  int textW = len * 6 - 1;
  int xEnd = (VFD_WIDTH - textW) / 2;
  int y = (VFD_HEIGHT - 8) / 2;
  int xStart = (dir >= 0) ? -textW : VFD_WIDTH;
  int xOut = (dir >= 0) ? VFD_WIDTH : -textW;
  const int steps = 9;

  /* 自动模式开关落盘时把送屏挂起了，屏幕现在是黑的。
   * 快照已经保存好，这里恢复送屏，让 OSD 第一帧直接从黑场开始，
   * 不会在中间闪出一帧正常画面。 */
  mn11236_hold_send(0);

  set_g_maxGray(7);
  /* draw_str() 内部的 5x7 字模会按 g_clockWidth 放大；提示文字必须用 1 倍，
   * 否则会变成 3 倍大小、超出屏幕，看起来像乱码。 */
  set_g_clockWidth(1);

  for (int i = 0; i <= steps; i++)
  {
    int x = xStart + (xEnd - xStart) * i / steps;
    mn11236_clear_buffers();
    auto_osd_border(accent);
    draw_str(x, y, line, White);
    mn11236_send_buffer();
    osDelay(20);
  }

  /* 字幕停留 1 秒，再滑出画面 */
  osDelay(1000);

  for (int i = 1; i <= steps; i++)
  {
    int x = xEnd + (xOut - xEnd) * i / steps;
    mn11236_clear_buffers();
    auto_osd_border(accent);
    draw_str(x, y, line, White);
    mn11236_send_buffer();
    osDelay(18);
  }

  memcpy(buf, auto_osd_snap, sizeof(auto_osd_snap));
  mn11236_send_buffer();

  /* 恢复时钟主题默认的 3 倍字宽；下一次 CLOCK 绘制会按主题重新设置。 */
  set_g_clockWidth(3);
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
  if (opnUp || opnDown || opnEnter || opnExit || auto_enable_req || auto_cancel_req)
  {
    last_key_tick = now;

    if (night)
    {
      wake_until_tick = now + WAKE_TIME_MS;
    }
  }

  // 夜间且未唤醒
  if (night && now > wake_until_tick && disp_mode != DISP_MODE_MENU)
  {
    if (HAL_GPIO_ReadPin(VFD_EN_GPIO_Port, VFD_EN_Pin))
    {
      EasyUITransitionAnim();
      /* 熄屏前确保送屏没有被过渡动画挂起 */
      pending_rebuild = 0;
      mn11236_hold_send(0);
      mn11236_clear_buffers();
      /* 显存已清空，顺手把黑屏同步给上位机（睡眠期间不再有送屏动作） */
      UploadFrame();
      /* scan_stop 内部先刷一帧全 0 再停扫，睡眠期间彻底不刷新 */
      mn11236_scan_stop();
      HAL_GPIO_WritePin(VFD_EN_GPIO_Port, VFD_EN_Pin, GPIO_PIN_RESET);
//      HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_RESET);
    }
    osDelay(200);
    return;
  }
  if (!HAL_GPIO_ReadPin(VFD_EN_GPIO_Port, VFD_EN_Pin))
  {
    opnUp = opnDown = opnEnter = opnExit = 0;
  }
  HAL_GPIO_WritePin(VFD_EN_GPIO_Port, VFD_EN_Pin, GPIO_PIN_SET);
  mn11236_scan_start(); /* 睡眠/停扫后恢复刷新（已在扫则直接返回） */

  /* 自动模式开关放在显示任务里处理：能安全写 FLASH，也不会在中断里做重活 */
  if (auto_enable_req)
  {
    auto_enable_req = 0;
    auto_mode_set(1, now);
  }
  if (auto_cancel_req)
  {
    auto_cancel_req = 0;
    auto_mode_set(0, now);
  }
  auto_mode_tick(now);

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

    /* 右半边有没有被日期占用：type==0 且 style==0 时右下角画日期 */
    uint8_t pet_right_ok = (uint8_t)(!(theme->type == 0 && theme->style == 0));

    if (theme->style == 1 || theme->type == 1)
      timeX += 1;

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
      /* 分钟跳变：宠物出来演一次，演完自己消失 */
      clock_pet_trigger(now, pet_right_ok);
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
      /* 桌面宠物：分钟跳变时出场一次，演完自动隐藏（不压数字） */
      clock_pet_draw(now, timeX * 3 + 60, pet_right_ok, theme->border);
    }

    roll_all_long_str();
    clock_wipe_scan_in(); /* 换主题时：新画面跟着扫描线长出来 */
    refrash_gram();
    break;
  }
  case DISP_MODE_FFT:
  {
    const MusicTheme_t *mt = &musicTheme[pUserInfo->musiccolor % FFT_COLOR_COUNT];

    clear_gram(White);
    set_g_maxGray(pUserInfo->clockdimming + 3);
    draw_fft(lBufMagArray, NPT / 2, 20,
             (uint8_t)(pUserInfo->musictheme % FFT_STYLE_COUNT), mt->bar, mt->dot);
    refrash_gram();
  }
  break;
  case DISP_MODE_GAME:
    /* 三个游戏共用这一个界面，长按上键（EXIT）循环切换 */
    clear_gram(White);
    if (pUserInfo->game == 0)
    {
      refrashSnake();
      osDelay(40); /* 25fps：3 帧走一格，每帧挪 1 像素（SNAKE_STEP_MS=120） */
    }
    else if (pUserInfo->game == 1)
    {
      refrashTetris();
      osDelay(40);
    }
    else
    {
      refrashTanks();
      osDelay(50);
    }
    refrash_gram();
    break;
  case DISP_MODE_MOTION:
    clear_gram(White);
    Motion_Run(pUserInfo->motion);
    osDelay(25); /* 动画按 30fps 走，太快会糊成一片 */
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

  /* 菜单里改的配置不当场写 FLASH：退出菜单时统一存一次（见下面两处退出）。
   * 自动模式运行中也不写：动画/风格每 3 分钟变一次，写 FLASH 太频繁；
   * 开关自动模式时 auto_mode_set() 已经强制存过，取消时会把最新状态一起写盘。 */
  if (disp_mode != DISP_MODE_MENU && !pUserInfo->automode)
    UserInfo_AutoSave();

  /* 自动模式进出提示：在当前画面渲染完之后叠加，显示完再恢复原画面 */
  if (auto_osd_req)
  {
    uint8_t req = auto_osd_req;
    auto_osd_req = 0;
    auto_osd_show(req == 1 ? "AUTO ON" : "AUTO OFF",
                  req == 1 ? Cyan : Red,
                  req == 1 ? 1 : -1);
  }

  // 菜单超时退出
  if (disp_mode == DISP_MODE_MENU)
  {
    if (now - last_key_tick > MENU_TIMEOUT)
    {
      /* 超时退出菜单也走碎裂落地过渡 */
      TransitionShatter();
      UserInfo_AutoSave(); /* 碎屏落地后的黑屏里统一存档 */
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
      {
        /* 换频谱风格：和换动画共用同一套滑屏转场
         * 旧画面滑走 -> 黑场里换风格 + 落盘 -> 新风格滑进来 */
        motion_switch_anim_out();
        pUserInfo->musictheme = (pUserInfo->musictheme + 1) % FFT_STYLE_COUNT;
        fft_style_reset();   /* 清掉上一种风格残留的瀑布/粒子 */
        UserInfo_AutoSave(); /* 黑屏时存档 */
        motion_switch_anim_in(fft_build_first_frame);
      }
      else if (disp_mode == DISP_MODE_MOTION)
      {
        /* 换动画：整屏横向滑走 -> 黑场里换动画并落盘 -> 新动画从右侧滑进来
         * FLASH 那几毫秒正好藏在黑屏里 */
        pUserInfo->motion = (pUserInfo->motion + 1) % ANIM_COUNT;
        motion_build_id = pUserInfo->motion;
        motion_switch_anim_out();
        UserInfo_AutoSave(); /* 黑屏时存档 */
        motion_switch_anim_in(motion_build_first_frame);
      }
      else if (disp_mode == DISP_MODE_CLOCK)
      {
        /* 换主题：扫描线把旧画面刷掉 → 黑场里换主题并落盘 → 新主题再刷进来 */
        clock_wipe_scan_out();
        pUserInfo->clocktheme = (pUserInfo->clocktheme + 1) % CLOCK_THEME_COUNT;
        UserInfo_AutoSave(); /* 黑场时存档（FLASH 写入正好藏在黑屏里） */
        clock_wipe_scan_begin_in();
      }
      else if (disp_mode == DISP_MODE_GAME)
      {
        /* 贪吃蛇 -> 俄罗斯方块 -> 坦克大战 -> 贪吃蛇
         * 用坦克换关那套冲击波过渡：旧画面被炸开，黑场里重开新游戏，再从中心涌出来
         * 存档压在黑场里写，FLASH 那几毫秒用户看不到 */
        pUserInfo->game = (pUserInfo->game + 1) % GAME_COUNT;
        game_switch_anim_out(pUserInfo->game);
        UserInfo_AutoSave(); /* 黑屏时存档 */
        game_switch_anim_in();
      }
      opnExit = false;
    }
  }
  else if (layer == 0 && opnExit)
  {
    /* 从菜单退回原界面：同样用画面碎裂落地过渡 */
    TransitionShatter();
    UserInfo_AutoSave(); /* 碎屏落地后的黑屏里统一存档 */
    disp_mode = last_disp_mode;
    opnExit = false;
    last_key_tick = HAL_GetTick();
  }
}
