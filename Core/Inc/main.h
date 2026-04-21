/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define BUZZER_Pin LL_GPIO_PIN_5
#define BUZZER_GPIO_Port GPIOE
#define DBG_TX_Pin LL_GPIO_PIN_10
#define DBG_TX_GPIO_Port GPIOB
#define DBG_RX_Pin LL_GPIO_PIN_11
#define DBG_RX_GPIO_Port GPIOB
#define USB_PENA_Pin LL_GPIO_PIN_14
#define USB_PENA_GPIO_Port GPIOB
#define USB_PFLT_Pin LL_GPIO_PIN_8
#define USB_PFLT_GPIO_Port GPIOD
#define USB_DM_Pin LL_GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin LL_GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define JTMS_Pin LL_GPIO_PIN_13
#define JTMS_GPIO_Port GPIOA
#define JTCK_Pin LL_GPIO_PIN_14
#define JTCK_GPIO_Port GPIOA
#define LCD_TX_Pin LL_GPIO_PIN_5
#define LCD_TX_GPIO_Port GPIOD
#define LCD_RX_Pin LL_GPIO_PIN_6
#define LCD_RX_GPIO_Port GPIOD
#define JTDO_Pin LL_GPIO_PIN_3
#define JTDO_GPIO_Port GPIOB
#define DBG_LED_Pin LL_GPIO_PIN_0
#define DBG_LED_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
