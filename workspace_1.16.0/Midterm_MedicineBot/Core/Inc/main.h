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
#include "stm32f4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define LefttPair_DirectionR_Pin GPIO_PIN_0
#define LefttPair_DirectionR_GPIO_Port GPIOC
#define LeftPair_DirectionF_Pin GPIO_PIN_1
#define LeftPair_DirectionF_GPIO_Port GPIOC
#define RightPair_DirectionR_Pin GPIO_PIN_2
#define RightPair_DirectionR_GPIO_Port GPIOC
#define RightPair_DirectionF_Pin GPIO_PIN_3
#define RightPair_DirectionF_GPIO_Port GPIOC
#define Buzzer_PWM_Pin GPIO_PIN_0
#define Buzzer_PWM_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define LeftPair_PWM_Pin GPIO_PIN_6
#define LeftPair_PWM_GPIO_Port GPIOA
#define RightPair_PWM_Pin GPIO_PIN_7
#define RightPair_PWM_GPIO_Port GPIOA
#define LED_Module_Pin GPIO_PIN_13
#define LED_Module_GPIO_Port GPIOB
#define Green_Button_Pin GPIO_PIN_15
#define Green_Button_GPIO_Port GPIOB
#define Ultrasonic_Echo_Pin GPIO_PIN_8
#define Ultrasonic_Echo_GPIO_Port GPIOA
#define Ultrasonic_Trig_Pin GPIO_PIN_9
#define Ultrasonic_Trig_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
