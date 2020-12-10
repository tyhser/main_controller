#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "syslog.h"
#include "motor.h"
#include "app_event.h"

struct motor {
    motor_state_t state;
    direction_t direction;
    bool zero;
    bool fault;
} motor_context[4];

#define MOTOR(i) (motor_context[(i)-1])

void motor_zero_handler(uint8_t index)
{
    motor_id_t id;
    switch (index)
    {
        case 1:
            id = MOTOR_SYRINGE_ID;
        break;
        case 2:
            id = MOTOR_X_AXIS_ID;
        break;
        case 3:
            id = MOTOR_Z_AXIS_ID;
        break;
        case 5:
            id = MOTOR_RECEIVED_ID;
        break;
        default:
        break;
    }
    MOTOR(id).zero = true;
    if (MOTOR(id).state == MOTOR_RUN && MOTOR(id).direction == DIRECTION_FWD) {
       motor_stop(id);
    }
}

void motor_fault_handler(uint8_t index)
{
    motor_id_t id;
    switch (index)
    {
        case 1:
            id = MOTOR_SYRINGE_ID;
        break;
        case 2:
            id = MOTOR_X_AXIS_ID;
        break;
        case 3:
            id = MOTOR_Z_AXIS_ID;
        break;
        case 5:
            id = MOTOR_RECEIVED_ID;
        break;
        default:
        break;
    }
    MOTOR(id).fault = true;
    motor_stop(id);
}

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

bool set_motor_state(motor_id_t id, motor_state_t s)
{
    bool ret = false;
    motor_state_t pre_state = MOTOR(id).state;
    if (MOTOR(id).state != s) {
        MOTOR(id).state = (s);

        motor_state_change_t state_notify = {0};
        state_notify.pre_state = pre_state;
        state_notify.cur_state = s;
        event_callback(EVENT_MOTOR_STATE_CHANGE, (event_param_t *)&state_notify);
        LOG_I("[motor]motor[%d] previous state:[%d] current state:[%d]", id, pre_state, s);
        ret = true;
    }
    return ret;
}

bool set_motor_direction_state(motor_id_t id, direction_t dir)
{
    bool ret = false;
    direction_t pre_dir = MOTOR(id).direction;
    if (MOTOR(id).direction != dir) {
        MOTOR(id).direction = (dir);

        motor_direction_change_t state_notify = {0};
        state_notify.pre_dir = pre_dir;
        state_notify.cur_dir = dir;
        event_callback(EVENT_MOTOR_DIRECTION_CHANGE, (event_param_t *)&state_notify);
        LOG_I("[motor]motor[%d] previous dir:[%d] current dir:[%d]", id, pre_dir, dir);
        ret = true;
    }
    return ret;
}

void set_motor_direction(motor_id_t motor_id, direction_t direction)
{
    bool error = false;
    LOG_I("[motor]set motor %d direction %d", motor_id, direction);
    if (MOTOR(motor_id).state == MOTOR_RUN) {
        motor_stop(motor_id);
    }
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
        set_motor_direction_state(motor_id, direction);
    }
}

void motor_run_steps(motor_id_t id, uint32_t step)
{
    bool error = false;
    LOG_I("[motor]motor[%d] start to run [%d]steps", id, step);
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
        set_motor_state(id, MOTOR_RUN);
    }
}

void motor_stop(motor_id_t id)
{
    bool error = false;
    LOG_I("[motor]stop motor[%d]", id);
    switch (id)
    {
        case MOTOR_SYRINGE_ID:
        {
            pwm_stop_output(PWM_1);
            motor_enable_disable(id, false);
        }
        break;
        case MOTOR_X_AXIS_ID:
        {
            pwm_stop_output(PWM_2);
            motor_enable_disable(id, false);
        }
        break;
        case MOTOR_Z_AXIS_ID:
        {
            pwm_stop_output(PWM_3);
            motor_enable_disable(id, false);
        }
        break;
        case MOTOR_RECEIVED_ID:
        {
            pwm_stop_output(PWM_4);
            motor_enable_disable(id, false);
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
        set_motor_state(id, MOTOR_STOP);
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
        LOG_I("[motor]this is not a motor id");
        return false;
    }
}

void syringe_absorb(uint32_t step)
{
    if (!is_motor_zero(MOTOR_SYRINGE_ID)) {
        set_motor_direction (MOTOR_SYRINGE_ID, DIRECTION_REV);
        motor_run_steps     (MOTOR_SYRINGE_ID, 10000);
        
    } else {

    }

}

void set_subdriver_param(uint8_t denominator)
{
    HAL_GPIO_WritePin(m_0_GPIO_Port, m_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_1_GPIO_Port, m_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_2_GPIO_Port, m_2_Pin, GPIO_PIN_SET);
}

void motor_enable_disable(motor_id_t id, bool value)
{
    switch (id)
    {
        case MOTOR_SYRINGE_ID:
            HAL_GPIO_WritePin(enable_1_GPIO_Port, enable_1_Pin, (GPIO_PinState)(!value)); 
            break;
        case MOTOR_X_AXIS_ID:
            HAL_GPIO_WritePin(enable_1_GPIO_Port, enable_1_Pin, (GPIO_PinState)(!value)); 
            break;
        case MOTOR_Z_AXIS_ID:
            HAL_GPIO_WritePin(enable_1_GPIO_Port, enable_1_Pin, (GPIO_PinState)(!value)); 
            break;
        case MOTOR_RECEIVED_ID:
            HAL_GPIO_WritePin(enable_1_GPIO_Port, enable_1_Pin, (GPIO_PinState)(!value)); 
            break;
        default:
            break;
    }
}

void motor_run(motor_id_t id, uint32_t distance, direction_t dir)
{
    set_motor_direction (id, DIRECTION_FWD);
    motor_enable_disable(id, true);
    motor_run_steps     (id, distance);
}

void motor_init(void)
{
    motor_enable_disable(MOTOR_SYRINGE_ID, false);
    motor_enable_disable(MOTOR_X_AXIS_ID, false);
    motor_enable_disable(MOTOR_Z_AXIS_ID, false);
    motor_enable_disable(MOTOR_RECEIVED_ID, false);
    set_subdriver_param(32);
    MOTOR(MOTOR_SYRINGE_ID).fault = false;
    MOTOR(MOTOR_X_AXIS_ID).fault = false;
    MOTOR(MOTOR_Z_AXIS_ID).fault = false;
    MOTOR(MOTOR_RECEIVED_ID).fault = false;

    MOTOR(MOTOR_SYRINGE_ID).state  = MOTOR_STOP;
    MOTOR(MOTOR_X_AXIS_ID).state   = MOTOR_STOP;
    MOTOR(MOTOR_Z_AXIS_ID).state   = MOTOR_STOP;
    MOTOR(MOTOR_RECEIVED_ID).state = MOTOR_STOP;

}

void motor_return_zero(motor_id_t id)
{
    motor_enable_disable(id, true);
    set_motor_direction(id, DIRECTION_REV);
    motor_run_steps(id, 100000);
}

status_t motor_event_handler(event_t event_id, void *parameters)
{
    event_param_t *event = (event_param_t *)parameters;
    switch (event_id)
    {
        case EVENT_INPUT:
        {
            channel_id_t *id = (channel_id_t *)parameters;
            LOG_I("[motor]Channel %d back to zero", id->num);
            motor_zero_handler(id->num);
        }
        break;
        case EVENT_FAULT:
        {
            channel_id_t *id = (channel_id_t *)parameters;
            LOG_I("[motor]Channel:%d fault", id->num);
            motor_fault_handler(id->num);
        }
        break;

    }
}
