#include "app_state_controler.h"
#include "syslog.h"

void app_state_idle_init(void)
{

}
void app_state_idle_handler(event_t event_id, void *parameters)
{
    LOG_I("[app state] idle state handle");
    switch (event_id)
    {
        default:
            app_state_change(APP_STATE_ERROR);
        break;
    }
}
void app_state_idle_exit(void)
{
    LOG_I("[app state] Exit app idle state");
}
void app_state_startup_init(void)
{

}
void app_state_startup_handler(event_t event_id, void *parameters)
{

}
void app_state_startup_exit(void)
{

}
void app_state_run_init(void)
{

}
void app_state_run_handler(event_t event_id, void *parameters)
{
}
void app_state_run_exit(void)
{

}
void app_state_shutdown_init(void)
{

}

void app_state_shutdown_handler(event_t event_id, void *parameters)
{
}

void app_state_shutdown_exit(void)
{

}
void app_state_error_init(void)
{
    LOG_I("[app state] Enter app error state");
}
void app_state_error_handler(event_t event_id, void *parameters)
{
    LOG_I("[app state] error handler received event:[0x%x]", event_id);
}
void app_state_error_exit(void)
{
}

static app_state_t app_state[] = {
    {APP_STATE_IDLE,        app_state_idle_init,        app_state_idle_handler,         app_state_idle_exit     },
    {APP_STATE_STARTUP,     app_state_startup_init,     app_state_startup_handler,      app_state_startup_exit  },
    {APP_STATE_RUN,         app_state_run_init,         app_state_run_handler,          app_state_run_exit      },
    {APP_STATE_SHUTDOWN,    app_state_shutdown_init,    app_state_shutdown_handler,     app_state_shutdown_exit },
    {APP_STATE_ERROR,       app_state_error_init,       app_state_error_handler,        app_state_error_exit    },
};
static app_state_t *app_get_app_state(void)
{
    return app_state;
}


static state_manage_t state_manage = {0};

state_manage_t *app_get_state_manage(void)
{
    return &state_manage;
}

void app_state_manage_init(void)
{
    if (state_manage.state_manage_init_done) {
        LOG_W("[state manage] state manage has been initted!");
        return;
    }
    state_manage.app_state = app_get_app_state();
    state_manage.current_state = APP_STATE_IDLE;
    state_manage.state_manage_init_done = true;
    state_manage.state_cnt = APP_STATE_END;
}

void app_state_change(app_state_value_t state)
{
    if (state >= APP_STATE_END)
        LOG_E("[state manage] state param invalid");
    if (state_manage.state_manage_init_done) {
        if (state_manage.current_state != state) {
            state_manage.app_state[state_manage.current_state].app_state_exit();
            state_manage.app_state[state].app_state_init();
            state_manage.current_state = state;
            /*TODO notify other parts*/
        }
    } else {
        LOG_E("[state manage] state manage not inited!");
    }
}

status_t app_state_event_handler(event_t event_id, void *parameters)
{
    LOG_I("[app state] app state event handler");
    state_manage.app_state[state_manage.current_state].app_state_handler(event_id, parameters);
    return STATUS_SUCCESS;
}
