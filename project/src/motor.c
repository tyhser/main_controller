#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "syslog.h"
#include "motor.h"

struct motor {
    direction_t direction;

}

bool is_motor_zero(motor_id_t motor_id)
{
    bool ret = false;

    switch (motor_id)
    {
        case MOTOR_SYRINGE_ID:
            ret = !(bool)HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4);
            break;
        case MOTOR_X_AXIS_ID:
            ret = !(bool)HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);
            break;
        case MOTOR_Z_AXIS_ID:
            ret = !(bool)HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0);
            break;
        case MOTOR_RECEIVED_ID:
            ret = !(bool)HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1);
            break;
        default:
            break;
    }
    return ret;
}

void set_motor_direction(motor_id_t motor_id, direction_t direction)
{
    LOG_I("set motor %d direction %d", motor_id, direction);
    switch (motor_id)
    {
        case MOTOR_SYRINGE_ID:
        {
            if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        case MOTOR_X_AXIS_ID:
        {
           if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        case MOTOR_Z_AXIS_ID:
        {
           if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        case MOTOR_RECEIVED_ID:
        {
           if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        default:
            LOG_E("unknow motor");
            break;
    }
}

void set_motor_run_stop(motor_id_t id, motor_state_t state)
{
    LOG_I("set motor %d state %d", motor_id, state);
    switch (motor_id)
    {
        case MOTOR_SYRINGE_ID:
        {
            if (state == MOTOR_RUN) {
                HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
            } else if (state == MOTOR_STOP) {
                HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            } else {
                LOG_E("unknow motor state");
            }
            break;
        }
        case MOTOR_X_AXIS_ID:
        {
            if (state == MOTOR_RUN) {
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
            } else if (state == MOTOR_STOP) {
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
            } else {
                LOG_E("unknow motor state");
            }
            break;
        }
        case MOTOR_Z_AXIS_ID:
        {
             if (state == MOTOR_RUN) {
                    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
                } else if (state == MOTOR_STOP) {
                    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
                } else {
                    LOG_E("unknow motor state");
                }
                break;
        }
        case MOTOR_RECEIVED_ID:
        {
            if (state == MOTOR_RUN) {
                    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);
                } else if (state == MOTOR_STOP) {
                    HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_1);
                } else {
                    LOG_E("unknow motor state");
                }
                break;
        }
        default:
            LOG_E("unknow motor");
            break;
    }
}

void set_valve_state(valve_id_t valve, valve_state_t state)
{
    switch (valve)
    {
        case VALVE_1:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (GPIO_PinState)state);
            break;
        case VALVE_2:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (GPIO_PinState)state);
            break;
        case VALVE_3:
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, (GPIO_PinState)state);
            break;
        case VALVE_4:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (GPIO_PinState)state);
            break;
        case VALVE_5:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (GPIO_PinState)state);
            break;
        case VALVE_6:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (GPIO_PinState)state);
            break;
        default:
            LOG_E("unknow valve");
            break;
    }
}
