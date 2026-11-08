/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nexlink_app.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SYS_LED_Pin GPIO_PIN_13
#define SYS_LED_GPIO_Port GPIOC
#define IIC_SDA_Pin GPIO_PIN_14
#define IIC_SDA_GPIO_Port GPIOC
#define IIC_SCL_Pin GPIO_PIN_15
#define IIC_SCL_GPIO_Port GPIOC
#define VFD_EN_Pin GPIO_PIN_3
#define VFD_EN_GPIO_Port GPIOA
#define VFD_BK_Pin GPIO_PIN_12
#define VFD_BK_GPIO_Port GPIOB
#define VFD_LAT_Pin GPIO_PIN_14
#define VFD_LAT_GPIO_Port GPIOB
#define KEY_SW2_Pin GPIO_PIN_8
#define KEY_SW2_GPIO_Port GPIOB
#define KEY_SW1_Pin GPIO_PIN_9
#define KEY_SW1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define NPT 256
#define dbmsg(fmt, args...) nexlink_log(fmt, ##args) //__FILE__,
  typedef enum
  {
    Time,
    Music,
    Snake,
    // Noty,
    Mode_Max,
  } Mode;


  typedef struct __attribute__((aligned(4)))
  {
    uint8_t defaultfunc;
    uint8_t clocktheme;
    uint8_t musictheme;
    uint8_t snakewidth;   /* 已废弃：贪吃蛇宽度固定 3，仅保留字段占位 */
    /* 动画编号 0..ANIM_COUNT-1（单选，对应菜单 Animation 进度条） */
    uint8_t motion;
    /* 游戏编号 0=贪吃蛇 1=俄罗斯方块 2=坦克大战（菜单 Game 或游戏界面长按上键切换） */
    uint8_t game;
    /* 自动演示模式：0=关，1=开。放在原来 motionpad 的第一个字节里，
     * 总长度和后面所有字段偏移都不变，旧配置读出来自动当成“关”。 */
    uint8_t automode;
    /* 原来 EffectSwitch_t 的 10 字节（motion + game + 8 字节占位），现在剩 7 字节占位 */
    uint8_t motionpad[7];
    uint8_t clockdimming;
		uint8_t screen_on_hour;   
		uint8_t screen_off_hour;  
    uint8_t valid;
    uint8_t musiccolor;   /* FFT 配色 0..5，与 musictheme（风格 0..15）独立 */
  } UserInfo_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
