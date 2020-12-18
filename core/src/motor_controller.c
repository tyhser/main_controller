#include "cmsis_os.h"
#include "motor.h"
#include "syslog.h"
#include "motor_controller.h"

#define MOTOR_ACT_QUEUE_SIZE 10

const osThreadAttr_t motor_controller_task_attributes = {
    .name = "motorControllerTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 128 * 5
};

osThreadId_t motor_controller_task_handle;


static osMessageQueueId_t motor_queue_handle;

static void motor_controller_task(void *arg);
static void motor_action_send_callback(motor_action_id_t action_id, motor_status_t result, void *parameters);
motor_status_t motor_action_handler(motor_action_id_t id, void *parameters);

bool motor_controller_init(void)
{
    bool  status = false;
    if (motor_controller_task_handle == NULL) {
        motor_controller_task_handle = osThreadNew(motor_controller_task, NULL, &motor_controller_task_attributes);
    } else {
        LOG_E("[motor_controller] motor controller have been inited or bad motor_controller_task_handle date");
    }
    if (motor_controller_task_handle == NULL) {
        LOG_E("[motor_controller] create motor conntroller task fail!");
    }

    if (0 != (motor_queue_handle = osMessageQueueNew(MOTOR_ACT_QUEUE_SIZE, sizeof(motor_action_t), NULL)))
    {
        status = true;
    }
    return status;
}

void motor_controller_close(void)
{
    if (motor_queue_handle != NULL) {
        osMessageQueueDelete(motor_queue_handle);
        motor_queue_handle = 0;
    }
}

static void motor_action_send(motor_action_id_t action_id, void *parameters, motor_action_send_result_t callback)
{
    motor_action_t action;
    LOG_I("[motor_controller] action:%04x", action_id);
    memset(&action, 0, sizeof(motor_action_t));
    action.act_id = action_id;
    action.parameters = parameters;
    action.send_callback = callback;

    if (motor_queue_handle == NULL) {
        LOG_E("[motor_controller] queue is not ready.");
    }

    if (osOK != osMessageQueuePut(motor_queue_handle, &action, 0, 0)) {
        if (NULL != callback) {
            callback(action_id, MC_STATUS_FAIL, parameters);
        }
        LOG_E("[motor_controller][Fatal Error]action lost:0x%02x", action_id);
    }
}

void motor_send_action(motor_action_id_t id, motor_action_param_t *param)
{
    void *action_params = NULL;
    if (NULL != param) {
        action_params = (motor_action_param_t *)pvPortMalloc(sizeof((*param)));
        memcpy(action_params, param, sizeof(*param));
    }
    motor_action_send(id, action_params, motor_action_send_callback);
}

static bool motor_action_process(motor_action_t *action)
{
    motor_status_t result;

    if (NULL != action) {
        LOG_I("[motor_controller] motor process: 0x%04x", action->act_id);

        result = motor_action_handler(action->act_id, action->parameters);

        if (action->send_callback) {
            action->send_callback(action->act_id, result, action->parameters);
        }
    }
}

static void motor_action_send_callback(motor_action_id_t action_id, motor_status_t result, void *parameters)
{
    if (NULL != parameters) {
        vPortFree(parameters);
        parameters = NULL;
    }
}

motor_status_t motor_action_handler(motor_action_id_t id, void *parameters)
{
    motor_action_param_t *action = (motor_action_param_t *)parameters;
    switch (id)
    {
        case MOTOR_ACT_MOVE:
        {
            motor_action_move_t *move = (motor_action_move_t *)parameters;

            if (motor_run_with_sspeed(move->motor_id, move->steps, move->dir)) {
                LOG_I("[motor_controller] action motor %d move with sspeed steps:%d dir:%d", move->motor_id, move->steps, move->dir);
            }
        }
            break;
        case MOTOR_ACT_STOP:
        {
            uint8_t *id = (uint8_t *)parameters;
            motor_stop(id);
            LOG_I("[motor_controller] action motor %d stop", (uint8_t)*id);
        }
            break;
        default:
            break;
    }
}

static void motor_controller_task(void *arg)
{
    motor_action_t action;
    LOG_I("enter motor controller task");
    while (1) {
        if(osOK == osMessageQueueGet(motor_queue_handle, &action, 0, osWaitForever)) {
            motor_action_process(&action);
        }
    }
}
