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
// Pages
EasyUIPage_t pageMain, pageUSBForm, pageDialog, pageSensor, pageAnimation, pageSetting, pageDiagnostic, pageAbout;

// Items
EasyUIItem_t itemUSBForm;
EasyUIItem_t itemDebug;
EasyUIItem_t itemSensor;
EasyUIItem_t itemAnimation;
EasyUIItem_t itemSetting, titleSetting, itemMusicTheme, itemClockTheme;
EasyUIItem_t itemDiagnostic;
EasyUIItem_t itemAbout;
EasyUIItem_t itemMind, itemCircle, itemSnowflake, itemMeteo, itemPlanet, itemTriangle, itemStarwar, itemBlast, itemGCircle, itemFirework, titleAnimation;

EasyKey_t keyUp, keyDown;

/*!
 * @brief   Sync the operation bool value
 *
 * @param   void
 * @return  void
 */
void EasyUIKeyActionMonitor() // Interrupt trigger, No HAL_Delay(xx)
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

void PageAbout(EasyUIPage_t *page)
{
  char tempstr[128];
  int screen_delta = 0;

  EasyUIDisplayStr(10, screen_delta, "Author: DPJ");
  screen_delta += page->rowheight;
  EasyUIDisplayStr(10, screen_delta, EasyUIVersion);
  screen_delta += page->rowheight;
  snprintf(tempstr, sizeof tempstr, "Rel. %s", __DATE__);
  EasyUIDisplayStr(10, screen_delta, tempstr);
  screen_delta += page->rowheight;
}

extern struct tm tm_local;
static const char weekdays[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
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
    screen_delta += page->rowheight;
    snprintf(tempstr, sizeof tempstr, "%02d:%02d:%02d", tm_local.tm_hour % 100, tm_local.tm_min % 100, tm_local.tm_sec % 100);
    EasyUIDisplayStr(10, screen_delta, tempstr);
  }
}

void PageDiagnostic(EasyUIItem_t *page)
{
	static uint16 color = 0;
  if(opnDown)
  {
    if(color + 1 <= 3)
      color += 1;
    else
      color = 0;
    opnDown = false;
  }
  if(opnUp)
  {
    if(color - 1 >= 0)
      color -= 1;
    else
      color = 3;
    opnUp = false;
  }
	EasyUIDrawBox(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, color);
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
	if(firstsave)
	{
		firstsave = false;
		SaveInfo();
	}
}

void InitInfo()
{
	flashMutex = xSemaphoreCreateMutex();
}

void EventChangeValue(EasyUIItem_t* item)
{
  if(opnDown)
  {
    if(*item->param + 1 <= item->paramMax)
      *item->param += 1;
    else
      *item->param = 0;
    opnDown = false;
  }
  if(opnUp)
  {
    if(*item->param - 1 >= 0)
      *item->param -= 1;
    else
      *item->param = item->paramMax;
    opnUp = false;
  }

  if(opnEnter)
  {
    item->paramBackup = *item->param;
    EasyUIBackgroundBlur();
    functionIsRunning = false;
    opnEnter = false;
  }
  if(opnExit) 
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

void AppGUI_Init(UserInfo_t *userinfo)
{
	InitInfo();
  pUserInfo = userinfo;
  EasyUIAddPage(&pageMain, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSetting, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
//  EasyUIAddPage(&pageUSBForm, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageUSBForm);
//  EasyUIAddPage(&pageDialog, GUI_6X8_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSensor, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageSensor);
  EasyUIAddPage(&pageAnimation, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageDiagnostic, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageDiagnostic);
  EasyUIAddPage(&pageAbout, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageAbout);

//  EasyUIAddItem(&pageMain, &itemUSBForm, "USBForm", ITEM_JUMP_PAGE, pageUSBForm.id);
//  EasyUIAddItem(&pageMain, &itemDebug, "Debug", ITEM_JUMP_PAGE, pageDialog.id);
  EasyUIAddItem(&pageMain, &itemSensor, "Sensor", ITEM_JUMP_PAGE, pageSensor.id);
  EasyUIAddItem(&pageMain, &itemAnimation, "Animation", ITEM_JUMP_PAGE, pageAnimation.id);
  EasyUIAddItem(&pageMain, &itemSetting, "Setting", ITEM_JUMP_PAGE, pageSetting.id);
  EasyUIAddItem(&pageMain, &itemDiagnostic, "Diagnostic", ITEM_JUMP_PAGE, pageDiagnostic.id);
  EasyUIAddItem(&pageMain, &itemAbout, "<About>", ITEM_JUMP_PAGE, pageAbout.id);

  EasyUIAddItem(&pageSetting, &titleSetting, "[Setting]", ITEM_PAGE_DESCRIPTION);
	
  EasyUIAddItem(&pageSetting, &itemMusicTheme, "FFT Theme", ITEM_PROGRESS_BAR, &pUserInfo->musictheme, 7, EventChangeValue);
  EasyUIAddItem(&pageSetting, &itemClockTheme, "Clock Theme", ITEM_PROGRESS_BAR, &pUserInfo->clocktheme, 3, EventChangeValue);

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
}

const MusicTheme_t musicTheme[8] = {
    {Cyan, Red},
    {Cyan, White},
    {Red, White},
    {White, Red},
    {CyanWithCyan, RedWithRed},
    {CyanWithCyan, WhiteWithWhite},
    {RedWithRed, WhiteWithWhite},
    {WhiteWithWhite, RedWithRed},
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

typedef enum
{
		DISP_MODE_CLOCK = 0,
		DISP_MODE_FFT,
		DISP_MODE_SNAKE,
		DISP_MODE_MOTION,
		DISP_MODE_MENU,
} SysDisplayMode_e;

extern char timeStr[10];
const ClockTheme_t clockTheme[4] = {
    {Red, WhiteWithCyan, 0},
    {Cyan, WhiteWithRed, 0},
    {Red, WhiteWithCyan, 1},
    {Cyan, WhiteWithRed, 1}};

void AppGUI_Loop(uint8_t timer)
{
	static SysDisplayMode_e last_disp_mode = DISP_MODE_MOTION;
	static SysDisplayMode_e disp_mode = DISP_MODE_MOTION;
  switch (disp_mode)
  {
  case DISP_MODE_CLOCK:
    clear_gram(White);
		draw_str_3x5((SCREEN_WIDTH/3-(3*5))/2 - 3, 0, timeStr, clockTheme[pUserInfo->clocktheme].font);
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
  default:
    EasyUIEvent(5);
    break;
  }
	UserInfo_AutoSave();

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
			if(disp_mode == DISP_MODE_FFT)
				pUserInfo->musictheme = (pUserInfo->musictheme + 1) % 8;
			else if(disp_mode == DISP_MODE_CLOCK)
				pUserInfo->clocktheme = (pUserInfo->clocktheme + 1) % 4;
			else if(disp_mode == DISP_MODE_SNAKE)
			{
				pUserInfo->snakewidth = (pUserInfo->snakewidth + 1) % 5;
				if(pUserInfo->snakewidth == 0) pUserInfo->snakewidth = 1;
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
