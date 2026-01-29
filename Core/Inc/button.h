/**
  ******************************************************************************
  * @file    ButtonEvent.h
  * @author  FASTSHIFT
  * @version V1.3.0
  * @date    2018-12-23
  * @brief   非阻塞式按键事件驱动库，支持短摁、长摁、释放、状态改变事件的识别。
  * @Upgrade 2019.6.18  添加按键双击事件、长按单次触发事件。
  * @Upgrade 2019.8.26  添加GetClicked、GetPressed、GetLongPressed非事件模式支持。
  * @Upgrade 2019.12.4  使用了新的事件回调设计，所有事件统一用EventAttach。
                        添加Click、LongPressed、LongPressRepeat事件。
  * @Upgrade 2020.6.12  合并ButtonEvent_Type至ButtonEvent类
  ******************************************************************************
  * @attention
  * 需要提供一个精确到毫秒级的系统时钟，用户需要在ButtonEvent.cpp里定义ButtonEvent_Millis
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUTTON_H
#define __BUTTON_H

#include "buttonevent.h"

void Button_Init(void);
void Button_Update(void);

#endif
