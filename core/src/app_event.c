#include "app_event.h"
#include "cmsis_os.h"
#include "syslog.h"
#include "app_main.h"
#include "string.h"
#include "motor.h"

static void app_event_node_init(app_event_node_t *event_node)
{
    event_node->previous = event_node;
    event_node->next = event_node;
}

static void app_event_node_insert(app_event_node_t *head, app_event_node_t *node)
{
    node->next = head;
    node->previous = head->previous;
    head->previous->next = node;
    head->previous = node;
}

static void app_event_node_remove(app_event_node_t *node)
{
    node->previous->next = node->next;
    node->next->previous = node->previous;
}

static app_event_callback_node_t *app_event_node_find_callback(app_event_node_t *head,
        event_t event_id,
        app_event_callback_t callback)
{
    app_event_node_t *current_node = head->next;
    app_event_callback_node_t *result = NULL;

    while (current_node != head) {
        if (((app_event_callback_node_t *)current_node)->event_id == event_id
                && ((app_event_callback_node_t *)current_node)->callback == callback) {
            result = (app_event_callback_node_t *)current_node;
            break;
        }
        current_node = current_node->next;
    }
    return result;
}

void app_event_init(void)
{
    app_context.invoking = EVENT_ALL;
    app_event_node_init(&app_context.dynamic_callback_header);
}

void app_event_post(event_t event_id, void *parameters, app_event_post_result_t callback)
{
    app_event_t event;

    LOG_I("[app event] app_event_post, event:%x", event_id);

    memset(&event, 0, sizeof(app_event_t));
    event.event_id = event_id;
    event.parameters = parameters;
    event.post_callback = callback;

    if (app_context.queue_handle == NULL) {
        LOG_E("[app event] queue is not ready.");
        return;
    }

    if (osOK != osMessageQueuePut (app_context.queue_handle, &event, 0, 0)) {
        if (NULL != callback) {
            callback(event_id, STATUS_FAIL, parameters);
        }
        LOG_E("[app evnet][Fatal Error] event lost:0x%x", event_id);
    }
}

void app_event_register_callback(event_t event_id, app_event_callback_t callback)
{
    app_event_callback_node_t *callback_node =
        app_event_node_find_callback(&app_context.dynamic_callback_header, event_id, callback);

    if (NULL == callback_node) {
        callback_node = (app_event_callback_node_t *)pvPortMalloc(sizeof(*callback_node));

        if (NULL != callback_node) {
            memset(callback_node, 0, sizeof(app_event_callback_node_t));

            callback_node->event_id = event_id;
            callback_node->callback = callback;
            app_event_node_insert(&app_context.dynamic_callback_header, &callback_node->pointer);
        }
    } else {
        callback_node->dirty = false;
    }
}

void app_event_deregister_callback(event_t event_id, app_event_callback_t callback)
{
    app_event_callback_node_t *callback_node =
        app_event_node_find_callback(&app_context.dynamic_callback_header, event_id, callback);
    if (NULL != callback_node) {
        if ((event_id == EVENT_ALL && EVENT_ALL != app_context.invoking)
                || event_id == app_context.invoking) {
            callback_node->dirty = true;
        } else {
            app_event_node_remove(&callback_node->pointer);
            vPortFree((void *)callback_node);
        }
    }
}

static status_t app_event_invoke(event_t event, void *parameters)
{
    status_t result = STATUS_SUCCESS;
    app_event_node_t *dynamic_callback = &app_context.dynamic_callback_header;

    app_context.invoking = event;
    while ((dynamic_callback = dynamic_callback->next) != &app_context.dynamic_callback_header) {
        if (EVENT_ALL == ((app_event_callback_node_t *)dynamic_callback)->event_id) {
            result = ((app_event_callback_node_t *)dynamic_callback)->callback(event, parameters);
            if (STATUS_EVENT_STOP == result) {
                // TRACE
                break;
            }
        } else if (event == ((app_event_callback_node_t *)dynamic_callback)->event_id) {
            result = ((app_event_callback_node_t *)dynamic_callback)->callback(event, parameters);
            if (STATUS_EVENT_STOP == result) {
                // TRACE
                break;
            }
        } else {
            // TRACE
        }
    }
    app_context.invoking = EVENT_ALL;

    dynamic_callback = app_context.dynamic_callback_header.next;

    while (dynamic_callback != &app_context.dynamic_callback_header) {
        if (((app_event_callback_node_t *)dynamic_callback)->dirty) {
            app_event_node_t *dirty_node = dynamic_callback;

            dynamic_callback = dynamic_callback->next;
            app_event_node_remove(dirty_node);
            vPortFree((void *)dirty_node);
            continue;
        }
        dynamic_callback = dynamic_callback->next;
    }
    return result;
}

void app_event_process(app_event_t *event)
{
    status_t result;

    if (NULL != event) {
        LOG_I("[app event] app_event_process:0x%04x" , event->event_id);

        result = app_event_invoke(event->event_id, event->parameters);

        if (event->post_callback) {
            event->post_callback(event->event_id, result, event->parameters);
        }
    }
}

void app_event_post_callback(event_t event_id, status_t result, void *parameters)
{
    if (NULL != parameters) {
        vPortFree(parameters);
        parameters = NULL;
    }
}

status_t app_event_handler(event_t event_id, void *parameters)
{
    event_param_t *event = (event_param_t *)parameters;
    switch (event_id) {
        case EVENT_APP_TEST:
            LOG_W("[app event] Handle event OK!!");
            break;
        case EVENT_INPUT:
        {
            channel_id_t *id = (channel_id_t *)parameters;
            LOG_I("received input zero channel:%d", id->num);
            if (id->num == 6) {
                motor_run_steps(MOTOR_SYRINGE_ID, 10000);
            }
        }
        break;
        case EVENT_FAULT:
        {
            channel_id_t *id = (channel_id_t *)parameters;
            LOG_I("received input fault channel:%d", id);
        }
            break;
        default:
            break;
    }

    return STATUS_SUCCESS;
}
