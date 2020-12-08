#ifndef _APP_STATE_CONTROLER_H
#define _APP_STATE_CONTROLER_H
#include <stdbool.h>
#include <stdint.h>
#include "app_event.h"

typedef enum {
    APP_STATE_IDLE,
    APP_STATE_STARTUP,
    APP_STATE_RUN,
    APP_STATE_SHUTDOWN,
    APP_STATE_ERROR,
    APP_STATE_END
} app_state_value_t;

typedef void (*app_state_init)(void);
typedef void (*app_state_exit)(void);
typedef void (*app_state_handler)(event_t event_id, void *parameters);

typedef struct {
    app_state_value_t state_value;
    app_state_init app_state_init;
    app_state_handler app_state_handler;
    app_state_exit app_state_exit;
} app_state_t;

typedef struct {
    app_state_t *app_state;
    bool state_manage_init_done;
    app_state_value_t current_state;
    uint32_t state_cnt;
} state_manage_t;

void app_state_manage_init(void);
state_manage_t *app_get_state_manage(void);
void app_state_change(app_state_value_t state);
status_t app_state_event_handler(event_t event_id, void *parameters);

#endif

