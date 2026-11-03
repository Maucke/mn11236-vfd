/*!
 * Copyright (c) 2023, ErBW_s
 * All rights reserved.
 *
 * @author  Baohan
 */

#ifndef _MENU_H
#define _MENU_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>
#include "easy_ui.h"
#include "easy_key.h"

typedef enum
{
  DISP_MODE_CLOCK = 0,
  DISP_MODE_OBD,
  DISP_MODE_FFT,
  DISP_MODE_SNAKE,
  DISP_MODE_TETRIS,
  DISP_MODE_TANKS,
  DISP_MODE_MOTION,
  DISP_MODE_EXTERN,
  DISP_MODE_MENU,
} SysDisplayMode_e;

extern EasyKey_t keyUp, keyDown;           // Used to control value up and down

void AppGUI_Init(UserInfo_t *userinfo);
void AppGUI_Loop(uint8_t timer);
void EasyUIKeyActionMonitor(void);
void EventJump(void);
void lowBatteryAction(void);
void EasyUIShutDown(void);
void ExternUI_Init(void);
	
#ifdef __cplusplus
}
#endif

#endif
