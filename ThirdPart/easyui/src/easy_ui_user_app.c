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

// Pages
EasyUIPage_t pageMain, pageUSBForm, pageDialog, pageSensor, pageAnimation, pageSetting, pageAbout;

// Items
EasyUIItem_t itemUSBForm;
EasyUIItem_t itemDebug;
EasyUIItem_t itemSensor;
EasyUIItem_t itemAnimation;
EasyUIItem_t itemSetting, itemColor, itemReset, itemBrightness, titleSetting;
EasyUIItem_t itemAbout;
EasyUIItem_t itemMind, itemCircle, itemSnowflake, itemMeteo, itemPlanet, itemTriangle, itemStarwar, itemBlast, itemGCircle, itemFirework, titleAnimation;

float setting_brightness;

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

  EasyUIDisplayStr(10, screen_delta, "MCU: STM32F405RG");
  screen_delta += page->rowheight;
  EasyUIDisplayStr(10, screen_delta, "MPU: MPU6050");
  screen_delta += page->rowheight;
  EasyUIDisplayStr(10, screen_delta, "CLOCK: RX8900");
  screen_delta += page->rowheight;
  EasyUIDisplayStr(10, screen_delta, "SEN: BMP280");
  screen_delta += page->rowheight;
  EasyUIDisplayStr(10, screen_delta, "Author: DPJ");
  screen_delta += page->rowheight;
  EasyUIDisplayStr(10, screen_delta, EasyUIVersion);
  screen_delta += page->rowheight;
  snprintf(tempstr, sizeof tempstr, "Rel. %s", __DATE__);
  EasyUIDisplayStr(10, screen_delta, tempstr);
  screen_delta += page->rowheight;
}

void PageUSBForm(EasyUIPage_t *page)
{
  // if(usbinhibit)
  // {
  //   EasyUITransitionAnim();
  //   EasyUIClearBuffer();
  //   EasyUISendBuffer();
  //   usbinhibit = false;
  // }
  // if(opnExit)
  // {
  //   usbinhibit = true;

  // 	GUI_SetRotation(GUI_PORTAIT);
  // }
}

static const char weekdays[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
void PageSensor(EasyUIPage_t *page)
{
  static int levelrun = 0;
  char tempstr[64];
  int screen_delta = 10;
  static struct tm time_user;
  //  static float temperature;
  static long last_update_time = 0;
  long now_tick = HAL_GetTick();
  if (now_tick - last_update_time > 500)
  {
    RX8900_GetTime(&time_user);
    screen_delta += page->rowheight;
    GUI_DrawBMP565(165, screen_delta, 36, 36, gImage_Battery[levelrun]);

    snprintf(tempstr, sizeof tempstr, "%04d-%02d-%02d, %s", time_user.tm_year + 1900, time_user.tm_mon + 1, time_user.tm_mday, weekdays[time_user.tm_wday % 7]);
    EasyUIDisplayStr(10, screen_delta, tempstr);
    screen_delta += page->rowheight;
    snprintf(tempstr, sizeof tempstr, "%02d:%02d:%02d", time_user.tm_hour % 100, time_user.tm_min % 100, time_user.tm_sec % 100);
    EasyUIDisplayStr(10, screen_delta, tempstr);
    screen_delta += page->rowheight;
  }
}

void PageDialog(EasyUIItem_t *page)
{
}

UserInfo_t *_userinfo;
void AppGUI_Init(UserInfo_t *userinfo)
{
  _userinfo = userinfo;
  EasyUIAddPage(&pageMain, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSetting, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageUSBForm, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageUSBForm);
  EasyUIAddPage(&pageDialog, GUI_6X8_FONT, PAGE_LIST);
  EasyUIAddPage(&pageSensor, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageSensor);
  EasyUIAddPage(&pageAnimation, GUI_DEFAULT_DISPLAY_FONT, PAGE_LIST);
  EasyUIAddPage(&pageAbout, GUI_DEFAULT_DISPLAY_FONT, PAGE_CUSTOM, PageAbout);

  EasyUIAddItem(&pageMain, &itemUSBForm, "USBForm", ITEM_JUMP_PAGE, pageUSBForm.id);
  EasyUIAddItem(&pageMain, &itemDebug, "Debug", ITEM_JUMP_PAGE, pageDialog.id);
  EasyUIAddItem(&pageMain, &itemSensor, "Sensor", ITEM_JUMP_PAGE, pageSensor.id);
  EasyUIAddItem(&pageMain, &itemAnimation, "Animation", ITEM_JUMP_PAGE, pageAnimation.id);
  EasyUIAddItem(&pageMain, &itemSetting, "Setting", ITEM_JUMP_PAGE, pageSetting.id);
  EasyUIAddItem(&pageMain, &itemAbout, "<About>", ITEM_JUMP_PAGE, pageAbout.id);

  EasyUIAddItem(&pageSetting, &titleSetting, "[Setting]", ITEM_PAGE_DESCRIPTION);

  EasyUIAddItem(&pageAnimation, &titleAnimation, "[Animation]", ITEM_PAGE_DESCRIPTION);
  EasyUIAddItem(&pageAnimation, &itemMind, "Mind", ITEM_CHECKBOX, &_userinfo->effectswitch.enMind);
  EasyUIAddItem(&pageAnimation, &itemCircle, "Circle", ITEM_CHECKBOX, &_userinfo->effectswitch.enCircle);
  EasyUIAddItem(&pageAnimation, &itemSnowflake, "Snowflake", ITEM_CHECKBOX, &_userinfo->effectswitch.enSnowflake);
  EasyUIAddItem(&pageAnimation, &itemMeteo, "Meteo", ITEM_CHECKBOX, &_userinfo->effectswitch.enMeteo);
  EasyUIAddItem(&pageAnimation, &itemPlanet, "Planet", ITEM_CHECKBOX, &_userinfo->effectswitch.enPlanet);
  EasyUIAddItem(&pageAnimation, &itemTriangle, "Triangle", ITEM_CHECKBOX, &_userinfo->effectswitch.enTriangle);
  EasyUIAddItem(&pageAnimation, &itemStarwar, "Starwar", ITEM_CHECKBOX, &_userinfo->effectswitch.enStarwar);
  EasyUIAddItem(&pageAnimation, &itemGCircle, "GCircle", ITEM_CHECKBOX, &_userinfo->effectswitch.enGCircle);
  EasyUIAddItem(&pageAnimation, &itemFirework, "Firework", ITEM_CHECKBOX, &_userinfo->effectswitch.enFirework);

  //		EasyUIItemOperationResponse(&pageAnimation, &itemAnimation, &itemAnimation.id);
  // Key init
  EasyKeyInit(&keyUp, GPIOB, KEY_SW1_Pin);
  EasyKeyInit(&keyDown, GPIOB, KEY_SW2_Pin);
  EasyUIInit(1);
  clock_init();
  beginSnake();
  Motion_Init();
  clear_all_long_str();
}

const ClockTheme_t clockTheme[4] = {
    {Red, WhiteWithCyan, 0},
    {Cyan, WhiteWithRed, 0},
    {Red, WhiteWithCyan, 1},
    {Cyan, WhiteWithRed, 1}};

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
extern long lBufMagArray[NPT];

void Motion_Run(EffectSwitch_t *effectswitch)
{
  if (effectswitch->enMind)
    Motion_Mind();
  if (effectswitch->enCircle)
    Motion_Circle();
  if (effectswitch->enSnowflake)
    Motion_Snowflake();
  if (effectswitch->enMeteo)
    Motion_Movmeteor();
  if (effectswitch->enPlanet)
    Motion_Planet();
  if (effectswitch->enTriangle)
    Motion_Triangle();
  if (effectswitch->enStarwar)
    Motion_StarWar();
  if (effectswitch->enGCircle)
    Motion_GCFireworks();
  if (effectswitch->enFirework)
    Motion_Firework();
}

extern uint8_t layer;

void AppGUI_Loop(uint8_t timer)
{
  static int mode = 1;
  
  switch (mode)
  {
  case 0:
    clear_gram(White);
    draw_fft(lBufMagArray, NPT / 2, 20, musicTheme[_userinfo->musictheme].bar, musicTheme[_userinfo->musictheme].dot);
    refrash_gram();
    break;
  case 1:
    clear_gram(White);
    refrashSnake();
    osDelay(50);
    refrash_gram();
    break;
  case 2:
    clear_gram(White);
    Motion_Run(&_userinfo->effectswitch);
    refrash_gram();
    break;
  default:
    EasyUIEvent(5);
    break;
  }

	if(mode != 3)
	{
		if (opnUp)
		{
			dbmsg("opnUp");
			mode++;
			if (mode > 2)
				mode = 0;
			opnUp = false;
		}
		if (opnDown)
		{
			dbmsg("opnDown");
			mode--;
			if (mode < 0)
				mode = 2;
			opnDown = false;
		}
		if (opnEnter)
		{
			dbmsg("opnEnter");
			mode = 3;
			opnEnter = false;
		}
	}
  if(layer == 0 && opnExit)
  {
    dbmsg("opnExit");
    mode = 0;
    opnExit = false;
  }
}
