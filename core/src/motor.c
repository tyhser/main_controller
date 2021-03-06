#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "syslog.h"
#include "motor.h"
#include "app_event.h"
#include "cmsis_os.h"
#include "motor_controller.h"
#include "math.h"

#define SUB_STEP 32
#define LEAD_MM 2

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
        state_notify.motor_id = id;
        state_notify.pre_state = pre_state;
        state_notify.cur_state = s;
        //event_callback(EVENT_MOTOR_STATE_CHANGE, (event_param_t *)&state_notify);
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
            //motor_enable_disable(id, false);
        }
        break;
        case MOTOR_X_AXIS_ID:
        {
            pwm_stop_output(PWM_2);
            //motor_enable_disable(id, false);
        }
        break;
        case MOTOR_Z_AXIS_ID:
        {
            pwm_stop_output(PWM_3);
            //motor_enable_disable(id, false);
        }
        break;
        case MOTOR_RECEIVED_ID:
        {
            pwm_stop_output(PWM_4);
            //motor_enable_disable(id, false);
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
    set_subdriver_param(32);
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
            LOG_I("[motor] send motor controller action motor:%d dir:%d step:%d", m_step->motor_id, m_step->dir, m_step->step);

            motor_action_move_t move = {0};
            move.motor_id   = m_step->motor_id;
            move.dir        = m_step->dir;
            move.steps      = m_step->step;
            motor_send_action(MOTOR_ACT_MOVE, &move);
#if 0
            motor_run(m_step->motor_id, m_step->step, m_step->dir);
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
        case EVENT_MOTOR_STATE_CHANGE:
        {
            motor_state_change_t *state_notify = (motor_state_change_t *)parameters;
            LOG_I("[motor]EVENT_MOTOR_STATE_CHANGE motor:%d pre %d cur %d", state_notify->motor_id, state_notify-> pre_state, state_notify->cur_state);
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

#if 0
bool motor_run_with_sspeed(motor_id_t id, uint32_t distance, direction_t dir)
{
    if (is_motor_zero(id) && dir == DIRECTION_REV) {
        LOG_W("[motor] Can not rev when in zero position");
        return false;
    }
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
    return true;
}

#endif
/**
 * @brief: 【公式法】S型加减速曲线计算，公式：Y=Fstart+(Fend-Fstart)/(1+exp(-flexible*(x-num)/num))
 * @param   [OUT]   Fre[]       输出频率值
 *          [OUT]   Period[]    输出周期值
 *          [IN]    Len         变速脉冲点
 *          [IN]    StartFre    开始频率
 *          [IN]    EndFre      结束频率
 *          [IN]    Flexible    曲线参数
 * @return: none
 */
void calc_curve(uint16_t Fre[], uint32_t Period[],  float Len, float StartFre, float EndFre, float Flexible)
{
    UNUSED(Period);
    float melo;
    for (int i = 0; i < Len; i++) {
        melo = Flexible * (i-Len/2) / (Len/2);
        Fre[i] = (uint16_t)(round(StartFre + (EndFre - StartFre) / (1 + expf(-melo))));
        //Period[i] = (uint32_t)(TIM_CLK / TIM_FRESCALER / Fre[i]);
    }
}

void T_curve_fun(const float *u0,
			const float *u1,
			const float *u2,
			const float *t,
			float *y0,
			float *y1,
			float *y2)
{
    static float lasty1 = 0;
    float Am = u0[0];
    float Vm = u1[0];
    float Pf = u2[0];
    float T = t[0];
    
    /*acc time*/
    float Ta = Vm/Am;
    /*full speed time*/
    float Tm = (Pf - Am*Ta*Ta)/Vm;
    /*final time*/
    float Tf = 2*Ta+Tm;
    //LOG_I("Tf: %f",Tf);

    if (Tm > 0) {
        if (T <= Ta) {
            y0[0] = 0.5*Am*T*T;
            y1[0] = Am*T;
            y2[0] = Am;
        } else if (T<=(Ta+Tm)) {
            y0[0] = 0.5*Am*Ta*Ta + Vm*(T-Ta);
            y1[0] = Vm;
            y2[0] = 0;
        } else if (T<=(Ta+Tm+Ta)) {
            y0[0] = 0.5*Am*Ta*Ta + Vm*Tm + 0.5*Am*(T-Ta-Tm)*(T-Ta-Tm);
            y1[0] = Vm-Am*(T-Ta-Tm);
            y2[0] = -Am;
        }
    } else {

        Ta = sqrt(Pf/Am);
        if (T < Ta) {
            y0[0] = 0.5*Am*T*T;
            y1[0] = Am*T;
            y2[0] = Am;
        } else {
            y0[0] = 0.5*Am*Ta*Ta + 0.5*Am*(T-Ta)*(T-Ta);
            y1[0] = Am*Ta - Am*(T-Ta)+0.5;
            y2[0] = -Am;
        }
    }
}

bool motor_run_with_sspeed(motor_id_t id, uint32_t distance, direction_t dir)
{
    if (is_motor_zero(id) && dir == DIRECTION_REV) {
        LOG_W("[motor] Can not rev when in zero position");
        return false;
    }


}

static uint32_t dist_to_pulse(uint16_t dis)
{
    return SUB_STEP*200*dis/LEAD_MM;
}

static uint32_t speed_to_freq(uint16_t speed)
{
    return dist_to_pulse(speed);
}

/* unit: speed: mm/s time:ms acc:mm/s2 */
bool motor_run_liner_speed(motor_id_t id, uint32_t distance, direction_t dir)
{
    if (is_motor_zero(id) && dir == DIRECTION_REV) {
        LOG_W("[motor] Can not rev when in zero position");
        return false;
    }

    /*speed mm/s*/
    float max_speed = 30.0;
    float min_speed = 1.0;

    float Am = 40.0;
    float Vm = max_speed;
    float Pf = (float)distance;

    float P = 0;
    float V = 0;
    float A = 0;

    set_motor_direction (id, dir);
    motor_enable_disable(id, true);
    motor_run_steps     (id, dist_to_pulse(distance));

#if 1
    TickType_t last_tick = osKernelGetTickCount();
    for (float i = 0.0;; i+=0.001) {
        T_curve_fun(&Am, &Vm, &Pf, &i, &P, &V, &A);
        //LOG_I("set freq:%d", speed_to_freq(V));
        if (speed_to_freq(V) != 0) {
            motor_set_speed(id, speed_to_freq(V));
        }
        //LOG_I("motor %d state:%d i:%f", id, MOTOR(id).state, i);
        if (MOTOR(id).state == MOTOR_STOP) {
            break;
        }
        last_tick += 1;
        osDelayUntil(last_tick);
    }
#endif
}
