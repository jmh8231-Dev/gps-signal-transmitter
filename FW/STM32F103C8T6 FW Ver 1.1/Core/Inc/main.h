/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define D4_Pin GPIO_PIN_13
#define D4_GPIO_Port GPIOC
#define EN_Pin GPIO_PIN_14
#define EN_GPIO_Port GPIOC
#define RS_Pin GPIO_PIN_15
#define RS_GPIO_Port GPIOC
#define LD_HIGH_Pin GPIO_PIN_0
#define LD_HIGH_GPIO_Port GPIOA
#define LD_LOW_Pin GPIO_PIN_1
#define LD_LOW_GPIO_Port GPIOA
#define LD_MIN_Pin GPIO_PIN_2
#define LD_MIN_GPIO_Port GPIOA
#define SW_4_Pin GPIO_PIN_3
#define SW_4_GPIO_Port GPIOA
#define ADC_VIN_Pin GPIO_PIN_4
#define ADC_VIN_GPIO_Port GPIOA
#define LD_MAX_Pin GPIO_PIN_0
#define LD_MAX_GPIO_Port GPIOB
#define SPI1_CS_Pin GPIO_PIN_1
#define SPI1_CS_GPIO_Port GPIOB
#define SPI1_CE_Pin GPIO_PIN_2
#define SPI1_CE_GPIO_Port GPIOB
#define ATmega328p_Tx_Pin GPIO_PIN_10
#define ATmega328p_Tx_GPIO_Port GPIOB
#define ATmega328p_Rx_Pin GPIO_PIN_11
#define ATmega328p_Rx_GPIO_Port GPIOB
#define SW_1_Pin GPIO_PIN_12
#define SW_1_GPIO_Port GPIOB
#define SW_2_Pin GPIO_PIN_13
#define SW_2_GPIO_Port GPIOB
#define SW_3_Pin GPIO_PIN_14
#define SW_3_GPIO_Port GPIOB
#define ATmega328p_Reset_Pin GPIO_PIN_15
#define ATmega328p_Reset_GPIO_Port GPIOB
#define LD_Debug_Pin GPIO_PIN_8
#define LD_Debug_GPIO_Port GPIOA
#define GPS_Tx_Pin GPIO_PIN_9
#define GPS_Tx_GPIO_Port GPIOA
#define GPS_Rx_Pin GPIO_PIN_10
#define GPS_Rx_GPIO_Port GPIOA
#define Status_Pin GPIO_PIN_15
#define Status_GPIO_Port GPIOA
#define USB_EN_Pin GPIO_PIN_3
#define USB_EN_GPIO_Port GPIOB
#define D7_Pin GPIO_PIN_4
#define D7_GPIO_Port GPIOB
#define ISP_Pin GPIO_PIN_5
#define ISP_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_8
#define D6_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_9
#define D5_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
