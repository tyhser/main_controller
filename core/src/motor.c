#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "syslog.h"
#include "motor.h"

struct motor {
    motor_state_t state;
    direction_t direction;
    bool home;
} motor_context[4];

#define MOTOR(i) (motor_context[(i)-1])

bool is_motor_zero(motor_id_t motor_id)
{
    bool ret = false;

    switch (motor_id)
    {
        case MOTOR_SYRINGE_ID:
            ret = !(bool)HAL_GPIO_ReadPin(input_1_GPIO_Port, input_1_Pin);
            break;
        case MOTOR_X_AXIS_ID:
            ret = !(bool)HAL_GPIO_ReadPin(input_2_GPIO_Port, input_2_Pin);
            break;
        case MOTOR_Z_AXIS_ID:
            ret = !(bool)HAL_GPIO_ReadPin(input_3_GPIO_Port, input_3_Pin);
            break;
        case MOTOR_RECEIVED_ID:
            ret = !(bool)HAL_GPIO_ReadPin(input_5_GPIO_Port, input_5_Pin);
            break;
        default:
            break;
    }
    return ret;
}

void set_motor_direction(motor_id_t motor_id, direction_t direction)
{
    bool error = false;
    LOG_I("set motor %d direction %d", motor_id, direction);
    switch (motor_id)
    {
        case MOTOR_SYRINGE_ID:
        {
            if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(dir_1_GPIO_Port, dir_1_Pin, GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(dir_1_GPIO_Port, dir_1_Pin, GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        case MOTOR_X_AXIS_ID:
        {
           if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(dir_2_GPIO_Port, dir_2_Pin, GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(dir_2_GPIO_Port, dir_2_Pin, GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        case MOTOR_Z_AXIS_ID:
        {
           if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(dir_3_GPIO_Port, dir_3_Pin, GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(dir_3_GPIO_Port, dir_3_Pin, GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        case MOTOR_RECEIVED_ID:
        {
           if (direction == DIRECTION_FWD) {
                HAL_GPIO_WritePin(dir_4_GPIO_Port, dir_4_Pin, GPIO_PIN_SET);
            } else if (direction == DIRECTION_REV) {
                HAL_GPIO_WritePin(dir_4_GPIO_Port, dir_4_Pin, GPIO_PIN_RESET);
            } else {
                LOG_E("unknow direction");
            }
            break;
        }
        default:
        {
            error = true;
            LOG_E("unknow motor");
        }
            break;
    }
    if (!error) {
        MOTOR(motor_id).direction = direction;
    }
}

void motor_run_steps(motor_id_t id, uint32_t step)
{
    bool error = false;
    LOG_I("motor[%d] start to run [%d]steps", id, step);
    switch (id)
    {
        case MOTOR_SYRINGE_ID:
        {
            PWM1(step);
        }
        break;
        case MOTOR_X_AXIS_ID:
        {
            PWM2(step);
        }
        break;
        case MOTOR_Z_AXIS_ID:
        {
            PWM3(step);
        }
        break;
        case MOTOR_RECEIVED_ID:
        {
            PWM4(step);
        }
        break;
        default:
        {
            LOG_E("unknow motor");
            error = true;
        }
        break;
    }
    if (!error) {
        MOTOR(id).state = MOTOR_RUN;
    }
}

void motor_stop(motor_id_t id)
{
    bool error = false;
    LOG_I("stop motor[%d]", id);
    switch (id)
    {
        case MOTOR_SYRINGE_ID:
        {
            pwm_step_output(PWM_1);
        }
        break;
        case MOTOR_X_AXIS_ID:
        {
            pwm_step_output(PWM_2);
        }
        break;
        case MOTOR_Z_AXIS_ID:
        {
            pwm_step_output(PWM_3);
        }
        break;
        case MOTOR_RECEIVED_ID:
        {
            pwm_step_output(PWM_4);
        }
        break;
        default:
        {
            LOG_E("unknow motor");
            error = true;
        }
        break;
    }
    if (!error) {
        MOTOR(id).state = MOTOR_STOP;
    }
}

void set_motor_state(motor_id_t id, motor_state_t s)
{
    MOTOR(id).state = (s);
    LOG_I("set motor[%d] state:[%d]", id, s);
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

void syringe_absorb(uint32_t step)
{
    if (!is_motor_zero(MOTOR_SYRINGE_ID)) {
        set_motor_direction(MOTOR_SYRINGE_ID, DIRECTION_REV);
        
    } else {

    }

}
