#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "syslog.h"
#include "motor.h"
#include "app_event.h"
#include "cmsis_os.h"

extern const uint16_t svalue[];
extern const uint16_t svalue_cnt;
extern const uint16_t svalue_diff;

struct motor {
    motor_state_t state;
    direction_t direction;
    bool zero;
    bool fault;
} motor_context[4];

#define MOTOR(i) (motor_context[(i)-1])

const osThreadAttr_t motorTask_attributes = {
    .name = "motorTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 128 * 8
};

static void motorTask(void *arg);
osThreadId_t motorTaskHandle;

void motor_task_create(void)
{
    motorTaskHandle = osThreadNew(motorTask, NULL, &motorTask_attributes);
}

void motor_zero_handler(uint8_t index, uint8_t on_off)
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
    MOTOR(id).zero = on_off;
    if (MOTOR(id).state == MOTOR_RUN && MOTOR(id).direction == DIRECTION_REV && MOTOR(id).zero == true) {
       motor_stop(id);
    }
#if 1
    static direction_t dir = DIRECTION_REV;
    set_motor_direction(MOTOR_SYRINGE_ID, dir);

    if (MOTOR(MOTOR_SYRINGE_ID).state == MOTOR_STOP && id != MOTOR_SYRINGE_ID && is_motor_zero(MOTOR_SYRINGE_ID)) {
        dir = !dir;
        motor_enable_disable(MOTOR_SYRINGE_ID, true);
        motor_run_steps(MOTOR_SYRINGE_ID, 10000);
    }
    static int i = 0;
    if ((MOTOR(MOTOR_SYRINGE_ID).state == MOTOR_STOP) && (id != MOTOR_SYRINGE_ID)) {
        LOG_I("trigger run");
        LOG_I("i=%d", i);
        motor_enable_disable(MOTOR_SYRINGE_ID, true);
        motor_run_steps(MOTOR_SYRINGE_ID, 10000);
        i = 1;
    }
#endif
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
    GPIO_PinState m0 = GPIO_PIN_RESET;
    GPIO_PinState m1 = GPIO_PIN_RESET;
    GPIO_PinState m2 = GPIO_PIN_RESET;

    if (denominator == 32) {
        m0 = GPIO_PIN_SET;
        m1 = GPIO_PIN_RESET;
        m2 = GPIO_PIN_SET;
    } else if (denominator == 16) {
        m0 = GPIO_PIN_RESET;
        m1 = GPIO_PIN_RESET;
        m2 = GPIO_PIN_SET;
    } else if (denominator == 8) {
        m0 = GPIO_PIN_SET;
        m1 = GPIO_PIN_SET;
        m2 = GPIO_PIN_RESET;
    } else if (denominator == 4) {
        m0 = GPIO_PIN_RESET;
        m1 = GPIO_PIN_SET;
        m2 = GPIO_PIN_RESET;
    } else if (denominator == 2) {
        m0 = GPIO_PIN_SET;
        m1 = GPIO_PIN_RESET;
        m2 = GPIO_PIN_RESET;
    } else if (denominator == 1) {
        m0 = GPIO_PIN_RESET;
        m1 = GPIO_PIN_RESET;
        m2 = GPIO_PIN_RESET;
    } else {
        LOG_I("Unknow motor driver denominator");
        return;
    }
    HAL_GPIO_WritePin(m_0_GPIO_Port, m_0_Pin, m0);
    HAL_GPIO_WritePin(m_1_GPIO_Port, m_1_Pin, m1);
    HAL_GPIO_WritePin(m_2_GPIO_Port, m_2_Pin, m2);
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
    set_motor_direction (id, dir);
    motor_enable_disable(id, true);
    motor_run_steps     (id, distance);
}

void motor_init(void)
{
#if 0
    motor_enable_disable(MOTOR_SYRINGE_ID,  false);
    motor_enable_disable(MOTOR_X_AXIS_ID,   false);
    motor_enable_disable(MOTOR_Z_AXIS_ID,   false);
    motor_enable_disable(MOTOR_RECEIVED_ID, false);
#else
    motor_enable_disable(MOTOR_SYRINGE_ID,  true);
    motor_enable_disable(MOTOR_X_AXIS_ID,   true);
    motor_enable_disable(MOTOR_Z_AXIS_ID,   true);
    motor_enable_disable(MOTOR_RECEIVED_ID, true);
#endif
    set_subdriver_param(16);
    MOTOR(MOTOR_SYRINGE_ID).fault   = false;
    MOTOR(MOTOR_X_AXIS_ID).fault    = false;
    MOTOR(MOTOR_Z_AXIS_ID).fault    = false;
    MOTOR(MOTOR_RECEIVED_ID).fault  = false;

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
            LOG_I("[motor]Channel %d zero on off:[%d]", id->num, id->on_off);
            motor_zero_handler(id->num, id->on_off);
        }
        break;
        case EVENT_FAULT:
        {
            channel_id_t *id = (channel_id_t *)parameters;
            LOG_I("[motor]Channel:%d fault", id->num);
            motor_fault_handler(id->num);
        }
        break;
        case EVENT_MOTOR_STEPS:
        {
            motor_step_t *m_step = (motor_step_t *)parameters;
            LOG_I("[motor] motor:%d dir:%d step:%d", m_step->motor_id, m_step->dir, m_step->step);
#if 0
            motor_run(m_step->motor_id, m_step->step, m_step->dir);
#else
            motor_run_with_sspeed(m_step->motor_id, m_step->step, m_step->dir);
#endif
        }
        break;
        case EVENT_MOTOR_RUN_STOP:
        {
            motor_run_stop_t *m_rs = (motor_run_stop_t *)parameters;
            LOG_I("[motor] motor:%d state:%d", m_rs->motor_id, m_rs->state);
            if (m_rs->state == MOTOR_STOP) {
                motor_stop(m_rs->motor_id);
            } else {
                LOG_E("[motor] unknow steps, can't run");
            }
        }
        break;
        case EVENT_VALVE_OPEN_CLOSE:
        {
            valve_open_close_t *v_os = (valve_open_close_t *)parameters;
            LOG_I("[motor] valve:%d open close:%d", v_os->valve_id, v_os->state);

            set_valve_state(v_os->valve_id, v_os->state);
        }
        break;
        case EVENT_STOP_ALL:
        LOG_I("[motor] stop all motor and valve");
        motor_stop(MOTOR_SYRINGE_ID);
        motor_stop(MOTOR_X_AXIS_ID);
        motor_stop(MOTOR_Z_AXIS_ID);
        motor_stop(MOTOR_RECEIVED_ID);
        set_valve_state(VALVE_1, VALVE_STATE_CLOSE);
        set_valve_state(VALVE_2, VALVE_STATE_CLOSE);
        set_valve_state(VALVE_3, VALVE_STATE_CLOSE);
        set_valve_state(VALVE_4, VALVE_STATE_CLOSE);
        set_valve_state(VALVE_5, VALVE_STATE_CLOSE);
        set_valve_state(VALVE_6, VALVE_STATE_CLOSE);
        break;
        default:
            break;
    }
}

void motor_set_speed(motor_id_t id, uint32_t speed)
{
    switch (id)
    {
        case MOTOR_SYRINGE_ID:
            set_pwm_freq(PWM_1, speed);
        break;
        case MOTOR_X_AXIS_ID:
            set_pwm_freq(PWM_2, speed);
        break;
        case MOTOR_Z_AXIS_ID:
            set_pwm_freq(PWM_3, speed);
        break;
        case MOTOR_RECEIVED_ID:
            set_pwm_freq(PWM_4, speed);
        break;
        default:
        LOG_E("unknow motor id");
        break;
    }
}

void motor_run_with_sspeed(motor_id_t id, uint32_t distance, direction_t dir)
{
    uint8_t multiple = 100;

    /*denomination of distance for Acc*/
    uint8_t acc_part = 4;
    uint32_t pulse_perms = 0;

    set_motor_direction (id, dir);
    motor_enable_disable(id, true);
    motor_run_steps     (id, distance);

    pulse_perms = (svalue[svalue_cnt/4]*multiple)/1000;

    TickType_t last_tick = osKernelGetTickCount();
    for (int i = 0; i < svalue_cnt; i++) {
        motor_set_speed(id, multiple * svalue[i]);
        last_tick += (distance/(pulse_perms*svalue_cnt));
        last_tick += 5;
        osDelayUntil(last_tick);
    }
    LOG_I("delay %d ticks per svalue data", (distance/(pulse_perms*svalue_cnt))+10);
}

static void motorTask(void *arg)
{
    LOG_I("enter motorTask");
    while (1) {

    }
}
