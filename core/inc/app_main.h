#ifndef _APP_MAIN_H
#define APP_MAIN_H
#include <stdbool.h>
#include "cmsis_os2.h"
#include "app_event.h"
#include "app_state_controler.h"

typedef struct {
    state_manage_t      *state_manage;
    osMessageQueueId_t  queue_handle;
    app_state_value_t   state;
    app_event_node_t    dynamic_callback_header;
    event_t             invoking;
} app_context_t;
extern app_context_t app_context;
void app_task_create(void);

#endif
