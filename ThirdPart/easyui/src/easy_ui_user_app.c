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


typedef struct
{
  bool enMind;
  bool enCircle;
  bool enSnowflake;
  bool enMeteo;
  bool enPlanet;
  bool enTriangle;
  bool enStarwar;
  bool enGCircle;
  bool enFirework;
  bool reserved;
} motion_status;

motion_status mt;

float setting_brightness;

EasyKey_t keyUp, keyDown;

void EasyUIShutDown()
{
}

/*!
 * @brief   Sync the operation bool value
 *
 * @param   void
 * @return  void
 */
void EasyUIKeyActionMonitor() //Interrupt trigger, No HAL_Delay(xx)
{
  if(keyUp.isPressed)
  {
		opnEnter=1;
    dbmsg("keyUp:isPressed");
  }
  else if(keyUp.isHold)
  {
		opnExit=1;
    dbmsg("keyUp:isHold");
  }
  else if(keyUp.holdTime > 5000)
  {
    dbmsg("keyUp:holdTime:%d", keyDown.holdTime);
  }
  if(keyDown.isPressed)
  {
		opnDown = 1;
		opnForward = 1;
    dbmsg("keyDown:isPressed");
  }
  else if(keyDown.isHold)
  {
		opnUp = 1;
		opnBackward = 1;
    dbmsg("keyDown:isHold");
  }
  else if(keyDown.holdTime > 5000)
  {
    dbmsg("keyDown:holdTime:%d", keyDown.holdTime);
  }

  if(opnForward || opnBackward || opnEnter || opnExit || opnUp || opnDown)
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

#define DIALOGITEMSIZE 200

EasyUIItem_t itemDialog[DIALOGITEMSIZE];
char* itemDialogStr[DIALOGITEMSIZE];
int itemDialogIndex = 0;
int itemDialogCount = 0;

typedef enum
{
  normal = 0,
  warn,
  error
} MsgType_e;

void EventJump()
{
}

void EventMotion()
{
  if(mt.enMind)
    Motion_Mind();
  if(mt.enCircle)
    Motion_Circle();
  if(mt.enSnowflake)
    Motion_Snowflake();
  if(mt.enMeteo)
    Motion_Movmeteor();
  if(mt.enPlanet)
    Motion_Planet();
  if(mt.enTriangle)
    Motion_Triangle();
  if(mt.enStarwar)
    Motion_StarWar();
  if(mt.enGCircle)
    Motion_GCFireworks();
  if(mt.enFirework)
    Motion_Firework();
}

void PageAbout(EasyUIPage_t* page)
{
  char tempstr[128];
  int screen_delta = 10;

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

void PageUSBForm(EasyUIPage_t* page)
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
void PageSensor(EasyUIPage_t* page)
{
  static int levelrun = 0;
  char tempstr[64];
  int screen_delta = 10;
  static struct tm time_user;
  static float temperature;
  static long last_update_time = 0;
  long now_tick = HAL_GetTick();
  if(now_tick - last_update_time > 500)
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

void PageDialog(EasyUIItem_t* page)
{

}

void MenuInit()
{
  mt.enFirework = true;
  mt.enStarwar = true;
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
  EasyUIAddItem(&pageAnimation, &itemMind, "Mind", ITEM_CHECKBOX, &mt.enMind);
  EasyUIAddItem(&pageAnimation, &itemCircle, "Circle", ITEM_CHECKBOX, &mt.enCircle);
  EasyUIAddItem(&pageAnimation, &itemSnowflake, "Snowflake", ITEM_CHECKBOX, &mt.enSnowflake);
  EasyUIAddItem(&pageAnimation, &itemMeteo, "Meteo", ITEM_CHECKBOX, &mt.enMeteo);
  EasyUIAddItem(&pageAnimation, &itemPlanet, "Planet", ITEM_CHECKBOX, &mt.enPlanet);
  EasyUIAddItem(&pageAnimation, &itemTriangle, "Triangle", ITEM_CHECKBOX, &mt.enTriangle);
  EasyUIAddItem(&pageAnimation, &itemStarwar, "Starwar", ITEM_CHECKBOX, &mt.enStarwar);
  EasyUIAddItem(&pageAnimation, &itemGCircle, "GCircle", ITEM_CHECKBOX, &mt.enGCircle);
  EasyUIAddItem(&pageAnimation, &itemFirework, "Firework", ITEM_CHECKBOX, &mt.enFirework);

//		EasyUIItemOperationResponse(&pageAnimation, &itemAnimation, &itemAnimation.id);
  Motion_Init();
  // Key init
  EasyKeyInit(&keyUp, GPIOB, KEY_SW1_Pin);
  EasyKeyInit(&keyDown, GPIOB, KEY_SW2_Pin);
}
