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
#define ADC_AUDIO_Pin GPIO_PIN_0
#define ADC_AUDIO_GPIO_Port GPIOA
#define ADC_DIM_Pin GPIO_PIN_2
#define ADC_DIM_GPIO_Port GPIOA
#define VFD_EN_Pin GPIO_PIN_3
#define VFD_EN_GPIO_Port GPIOA
#define SPI1_SCS_Pin GPIO_PIN_4
#define SPI1_SCS_GPIO_Port GPIOA
#define SPI1_SCS_EXTI_IRQn EXTI4_IRQn
#define VFD_LAT_Pin GPIO_PIN_12
#define VFD_LAT_GPIO_Port GPIOB
#define ESP_EN_Pin GPIO_PIN_14
#define ESP_EN_GPIO_Port GPIOB
#define VFD_BLK_Pin GPIO_PIN_15
#define VFD_BLK_GPIO_Port GPIOA
#define BUS_SCL_Pin GPIO_PIN_6
#define BUS_SCL_GPIO_Port GPIOB
#define BUS_SDA_Pin GPIO_PIN_7
#define BUS_SDA_GPIO_Port GPIOB
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

#define OBD_ITEM_COUNT 8

	typedef struct __attribute__((aligned(4)))
	{
		uint8_t enMind;
		uint8_t enCircle;
		uint8_t enSnowflake;
		uint8_t enMeteo;
		uint8_t enPlanet;
		uint8_t enTriangle;
		uint8_t enStarwar;
		uint8_t enGCircle;
		uint8_t enFirework;
		uint8_t reserved;
	} EffectSwitch_t;

  typedef struct __attribute__((aligned(4)))
  {
    uint8_t defaultfunc;
    uint8_t clocktheme;
    uint8_t musictheme;
    uint8_t snakewidth;
		EffectSwitch_t effectswitch;
    uint8_t clockdimming;
		uint8_t screen_on_hour;   
		uint8_t screen_off_hour;  
		uint8_t esp_en;  
    uint8_t valid;
  } UserInfo_t;

  typedef struct
  {
    uint16_t rpm;
    uint8_t speed;
    uint8_t coolant_temp;
    uint8_t intake_temp;
    uint8_t load_pct;
    uint8_t throttle_pct;
    uint16_t voltage;      // 0.01V units
    int8_t oil_temp;
    uint8_t valid;         // bitmask of valid data
    uint32_t last_update_tick[OBD_ITEM_COUNT];
  } obd_data_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
