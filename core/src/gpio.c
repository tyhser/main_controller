/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "app_event.h"
#include "syslog.h"
#include "main.h"
#include "cmsis_os.h"
#include "interrupt_handler.h"

/* USER CODE BEGIN 0 */

volatile uint32_t interrupt_mask;
extern osThreadId_t interruptTaskHandle;
extern osSemaphoreId_t interrupt_sem;

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
     PC6   ------> S_TIM3_CH1
     PB3   ------> S_TIM2_CH2
     PB8   ------> S_TIM4_CH3
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, valve_a13_Pin|valve_b13_Pin|valve_a12_Pin|valve_b12_Pin
                          |valve_a11_Pin|valve_a7_Pin|valve_b7_Pin|valve_a8_Pin
                          |valve_b8_Pin|valve_a9_Pin|valve_b9_Pin|valve_a10_Pin
                          |valve_b10_Pin|dir_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, valve_b11_Pin|valve_a5_Pin|valve5_b_Pin|enable_4_Pin
                          |dir_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, valve_a1_Pin|valve_b1_Pin|valve_a2_Pin|valve_2b_Pin
                          |valve_a3_Pin|valve_b3_Pin|valve_a4_Pin|valve_b4_Pin
                          |led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, valve_a6_Pin|valve_6b_Pin|enable_2_Pin|dir_2_Pin
                          |enable_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, enable_3_Pin|dir_3_Pin|m_2_Pin|m_1_Pin
                          |m_0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PEPin PEPin PEPin PEPin
                           PEPin PEPin PEPin PEPin
                           PEPin PEPin PEPin PEPin
                           PEPin PEPin */
  GPIO_InitStruct.Pin = valve_a13_Pin|valve_b13_Pin|valve_a12_Pin|valve_b12_Pin
                          |valve_a11_Pin|valve_a7_Pin|valve_b7_Pin|valve_a8_Pin
                          |valve_b8_Pin|valve_a9_Pin|valve_b9_Pin|valve_a10_Pin
                          |valve_b10_Pin|dir_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PCPin PCPin PCPin PCPin
                           PCPin */
  GPIO_InitStruct.Pin = valve_b11_Pin|valve_a5_Pin|valve5_b_Pin|enable_4_Pin
                          |dir_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin PAPin
                           PAPin PAPin PAPin PAPin
                           PAPin */
  GPIO_InitStruct.Pin = valve_a1_Pin|valve_b1_Pin|valve_a2_Pin|valve_2b_Pin
                          |valve_a3_Pin|valve_b3_Pin|valve_a4_Pin|valve_b4_Pin
                          |led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin PBPin PBPin
                           PBPin */
  GPIO_InitStruct.Pin = valve_a6_Pin|valve_6b_Pin|enable_2_Pin|dir_2_Pin
                          |enable_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Fault Pin*/
  /*Configure GPIO pins : PE1 fault1*/
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PB6 fault2*/
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD3 fault3*/
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PC9 fault4*/
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Fault Pin end*/

    /*INPUT Pin*/
 /*Configure GPIO pin : PE15 input3*/
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 PB11 input1 input2*/
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 input5 input6 input7*/
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    /*INPUT Pin end*/

  /*home Pin*/
  /*Configure GPIO pins : PB7 home1*/
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD4 PD15 home2 home 4*/
  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 home3*/
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /*home Pin end*/

  /*Configure GPIO pins : PDPin PDPin PDPin PDPin
                           PDPin */
  GPIO_InitStruct.Pin = enable_3_Pin|dir_3_Pin|m_2_Pin|m_1_Pin
                          |m_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, INT_PRI_MIDDLE, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, INT_PRI_MIDDLE, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, INT_PRI_MIDDLE, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, INT_PRI_MIDDLE, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, INT_PRI_MIDDLE, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  interrupt_mask = 0U;
}

void led_toggle(void)
{
    HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    channel_id_t id = {0};
    switch (GPIO_Pin)
    {
    case input_1_Pin:
    {
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)) {
            interrupt_mask |= INT_MASK_ZERO_1_OFF;
        } else {
            interrupt_mask |= INT_MASK_ZERO_1_ON;
        }
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    case input_2_Pin: {
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) {
            interrupt_mask |= INT_MASK_ZERO_2_OFF;
        } else {
            interrupt_mask |= INT_MASK_ZERO_2_ON;
        }
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    case input_3_Pin:
    {
        if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15)) {
            interrupt_mask |= INT_MASK_ZERO_3_OFF;
        } else {
            interrupt_mask |= INT_MASK_ZERO_3_ON;
        }
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    /*because input_4_Pin is duplicate Pin11 so i change to use input_5_Pin*/
#if 0
    case input_4_Pin:
    {
        if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11)) {
            interrupt_mask |= INT_MASK_ZERO_4_OFF;
        } else {
            interrupt_mask |= INT_MASK_ZERO_4_ON;
        }
        osSemaphoreRelease(interrupt_sem);
    }
    break;
#endif
    case input_5_Pin:
    {
        if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_12)) {
            interrupt_mask |= INT_MASK_ZERO_5_OFF;
        } else {
            interrupt_mask |= INT_MASK_ZERO_5_ON;
        }
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    case input_6_Pin:
    {
        if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_13)) {
            interrupt_mask |= INT_MASK_ZERO_6_OFF;
        } else {
            interrupt_mask |= INT_MASK_ZERO_6_ON;
        }
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    case input_7_Pin:
    {
        if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14)) {
            interrupt_mask |= INT_MASK_ZERO_7_OFF;
        } else {
            interrupt_mask |= INT_MASK_ZERO_7_ON;
        }
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    case fault_1_Pin:
    {
        interrupt_mask |= INT_MASK_FAULT_1;
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    case fault_2_Pin:
    {
        interrupt_mask |= INT_MASK_FAULT_2;
        osSemaphoreRelease(interrupt_sem);
    }
    break;
   case fault_3_Pin:
   {
        interrupt_mask |= INT_MASK_FAULT_3;
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    case fault_4_Pin:
    {
        interrupt_mask |= INT_MASK_FAULT_4;
        osSemaphoreRelease(interrupt_sem);
    }
    break;
    default:
    break;
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
