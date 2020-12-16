/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;

/* USER CODE BEGIN Private defines */
#define PWM_1 1
#define PWM_2 2
#define PWM_3 3
#define PWM_4 4
typedef uint8_t pwm_id_t;

/* USER CODE END Private defines */

void pwm_output_init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

void pwm_output(pwm_id_t pwm_id, uint32_t cycle, uint32_t pulse_num);
void set_pwm_freq(pwm_id_t id, uint32_t freq);

void MX_TIM7_Init(uint16_t usTim1Timerout50us);

void timer7_enable(void);
void timer7_disable(void);
void timer7_deinit(void);
/* USER CODE BEGIN Prototypes */
#define PWM1(i) pwm_output(PWM_1, 10000, (i))
#define PWM2(i) pwm_output(PWM_2, 50000, (i))
#define PWM3(i) pwm_output(PWM_3, 10000, (i))
#define PWM4(i) pwm_output(PWM_4, 5000, (i))

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
