#include "app_main.h"
#include "app_event.h"
#include "cmsis_os.h"
#include "syslog.h"
#include "string.h"
#include "app_state_controler.h"
#include "motor.h"

#define APP_QUEUE_SIZE 30
app_context_t app_context;

const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 128 * 10
};

osThreadId_t mainTaskHandle;

void event_callback(event_t event_id,event_param_t *param)
{
    void *event_params = NULL;

    if (NULL != param) {
        event_params = (event_param_t *)pvPortMalloc(sizeof((*param)));
        memcpy(event_params, param, sizeof(*param));
    }
    app_event_post(event_id, event_params, app_event_post_callback);
}

static void mainTask(void *arg)
{
    app_event_t event;
    LOG_I("Enter main task");
    memset(&app_context, 0, sizeof(app_context_t));
    app_state_manage_init();
    app_event_init();
    app_context.queue_handle = osMessageQueueNew(APP_QUEUE_SIZE, sizeof(app_event_t), NULL);
    app_event_register_callback(EVENT_ALL, app_event_handler);
    app_event_register_callback(EVENT_ALL, motor_event_handler);
    while (1) {
        if (osOK == osMessageQueueGet(app_context.queue_handle, &event, 0, osWaitForever)) {
           app_event_process(&event);
        }
    }
}

void app_task_create(void)
{
    mainTaskHandle = osThreadNew(mainTask, NULL, &mainTask_attributes);
}


