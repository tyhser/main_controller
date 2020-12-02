#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "syslog.h"
#include "motor.h"

struct motor {
    motor_state_t state;
    direction_t direction;
    uint32_t volatile remain_steps;
    bool home;
} motor_context[4];

#define MOTOR(i) (motor_context[(i)-1])

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
    MOTOR(motor_id).direction = direction;
}

void set_motor_run_stop(motor_id_t id, motor_state_t state)
{
    LOG_I("set motor %d state %d", id, state);
    switch (id)
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

bool is_motor_id(uint8_t data)
{
    if (data < MOTOR_END_ID && data > 0) {
        return true;
    } else {
        LOG_I("this is not a motor id");
        return false;
    }
}

void set_drive_parameter_by_input(uint8_t data[5])
{
    if (is_motor_id(data[0])) {
        if (data[1] == 0xff) {
            set_motor_direction(data[0], DIRECTION_FWD);
        } else if (data[1] == 0x00) {
            set_motor_direction(data[0], DIRECTION_REV);
        } else {
            LOG_E("direction error");
        }
        MOTOR(data[0]).remain_steps = ((uint32_t)data[0]<<16)|((uint32_t)data[1]<<8)|data[2];
   }
}


