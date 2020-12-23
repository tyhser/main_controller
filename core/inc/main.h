/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "cmsis_os.h"

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
#define valve_a13_Pin GPIO_PIN_2
#define valve_a13_GPIO_Port GPIOE
#define valve_b13_Pin GPIO_PIN_3
#define valve_b13_GPIO_Port GPIOE
#define valve_a12_Pin GPIO_PIN_4
#define valve_a12_GPIO_Port GPIOE
#define valve_b12_Pin GPIO_PIN_5
#define valve_b12_GPIO_Port GPIOE
#define valve_a11_Pin GPIO_PIN_6
#define valve_a11_GPIO_Port GPIOE
#define valve_b11_Pin GPIO_PIN_13
#define valve_b11_GPIO_Port GPIOC
#define valve_a1_Pin GPIO_PIN_0
#define valve_a1_GPIO_Port GPIOA
#define valve_b1_Pin GPIO_PIN_1
#define valve_b1_GPIO_Port GPIOA
#define valve_a2_Pin GPIO_PIN_2
#define valve_a2_GPIO_Port GPIOA
#define valve_2b_Pin GPIO_PIN_3
#define valve_2b_GPIO_Port GPIOA
#define valve_a3_Pin GPIO_PIN_4
#define valve_a3_GPIO_Port GPIOA
#define valve_b3_Pin GPIO_PIN_5
#define valve_b3_GPIO_Port GPIOA
#define valve_a4_Pin GPIO_PIN_6
#define valve_a4_GPIO_Port GPIOA
#define valve_b4_Pin GPIO_PIN_7
#define valve_b4_GPIO_Port GPIOA
#define valve_a5_Pin GPIO_PIN_4
#define valve_a5_GPIO_Port GPIOC
#define valve5_b_Pin GPIO_PIN_5
#define valve5_b_GPIO_Port GPIOC
#define valve_a6_Pin GPIO_PIN_0
#define valve_a6_GPIO_Port GPIOB
#define valve_6b_Pin GPIO_PIN_1
#define valve_6b_GPIO_Port GPIOB
#define valve_a7_Pin GPIO_PIN_7
#define valve_a7_GPIO_Port GPIOE
#define valve_b7_Pin GPIO_PIN_8
#define valve_b7_GPIO_Port GPIOE
#define valve_a8_Pin GPIO_PIN_9
#define valve_a8_GPIO_Port GPIOE
#define valve_b8_Pin GPIO_PIN_10
#define valve_b8_GPIO_Port GPIOE
#define valve_a9_Pin GPIO_PIN_11
#define valve_a9_GPIO_Port GPIOE
#define valve_b9_Pin GPIO_PIN_12
#define valve_b9_GPIO_Port GPIOE
#define valve_a10_Pin GPIO_PIN_13
#define valve_a10_GPIO_Port GPIOE
#define valve_b10_Pin GPIO_PIN_14
#define valve_b10_GPIO_Port GPIOE
#define input_3_Pin GPIO_PIN_15
#define input_3_GPIO_Port GPIOE
#define input_2_Pin GPIO_PIN_10
#define input_2_GPIO_Port GPIOB
#define input_1_Pin GPIO_PIN_11
#define input_1_GPIO_Port GPIOB
#define rs485_Pin GPIO_PIN_8
#define rs485_GPIO_Port GPIOD
#define rs485D9_Pin GPIO_PIN_9
#define rs485D9_GPIO_Port GPIOD
#define input_4_Pin GPIO_PIN_11
#define input_4_GPIO_Port GPIOD
#define input_5_Pin GPIO_PIN_12
#define input_5_GPIO_Port GPIOD
#define input_6_Pin GPIO_PIN_13
#define input_6_GPIO_Port GPIOD
#define input_7_Pin GPIO_PIN_14
#define input_7_GPIO_Port GPIOD
#define home_4_Pin GPIO_PIN_15
#define home_4_GPIO_Port GPIOD
#define enable_4_Pin GPIO_PIN_7
#define enable_4_GPIO_Port GPIOC
#define dir_4_Pin GPIO_PIN_8
#define dir_4_GPIO_Port GPIOC
#define fault_4_Pin GPIO_PIN_9
#define fault_4_GPIO_Port GPIOC
#define home_3_Pin GPIO_PIN_8
#define home_3_GPIO_Port GPIOA
#define syslog_tx_Pin GPIO_PIN_9
#define syslog_tx_GPIO_Port GPIOA
#define syslog_rx_Pin GPIO_PIN_10
#define syslog_rx_GPIO_Port GPIOA
#define led_Pin GPIO_PIN_12
#define led_GPIO_Port GPIOA
#define enable_3_Pin GPIO_PIN_1
#define enable_3_GPIO_Port GPIOD
#define dir_3_Pin GPIO_PIN_2
#define dir_3_GPIO_Port GPIOD
#define fault_3_Pin GPIO_PIN_3
#define fault_3_GPIO_Port GPIOD
#define home_2_Pin GPIO_PIN_4
#define home_2_GPIO_Port GPIOD
#define m_2_Pin GPIO_PIN_5
#define m_2_GPIO_Port GPIOD
#define m_1_Pin GPIO_PIN_6
#define m_1_GPIO_Port GPIOD
#define m_0_Pin GPIO_PIN_7
#define m_0_GPIO_Port GPIOD
#define enable_2_Pin GPIO_PIN_4
#define enable_2_GPIO_Port GPIOB
#define dir_2_Pin GPIO_PIN_5
#define dir_2_GPIO_Port GPIOB
#define fault_2_Pin GPIO_PIN_6
#define fault_2_GPIO_Port GPIOB
#define home_1_Pin GPIO_PIN_7
#define home_1_GPIO_Port GPIOB
#define enable_1_Pin GPIO_PIN_9
#define enable_1_GPIO_Port GPIOB
#define dir_1_Pin GPIO_PIN_0
#define dir_1_GPIO_Port GPIOE
#define fault_1_Pin GPIO_PIN_1
#define fault_1_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */
#define INT_PRI_LOW 10
#define INT_PRI_MIDDLE 9
#define INT_PRI_HIGH 8
/* USER CODE END Private defines */
#define ASSERT(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

void show_task_stack_state(osThreadId_t threadId);
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
