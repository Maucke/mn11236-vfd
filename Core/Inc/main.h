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

#define dbmsg(fmt, args...) nexlink_log(fmt, ##args) //__FILE__,
  typedef enum
  {
    Time,
    Music,
    Snake,
    SetDimm,
    SetYear,
    SetMonth,
    SetDay,
    SetWeek,
    SetHour,
    SetMin,
    // Noty,
    Mode_Max,
  } Mode;

  typedef struct __attribute__((aligned(4)))
  {
    uint8_t dimming;
    uint8_t clocktheme;
    uint8_t musictheme;
    Mode sysmode;
    uint8_t valid;
  } UserInfo_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
