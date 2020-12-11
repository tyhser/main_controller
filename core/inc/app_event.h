#ifndef _APP_EVENT_H
#define _APP_EVENT_H
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    STATUS_SUCCESS        =     0,    /**< status: success. */
    STATUS_FAIL           =    -1,    /**< status: fail. */
    STATUS_PENDING        =    -2,    /**< status: operation is pending. */
    STATUS_INVALID_PARAM  =    -3,    /**< status: invalid parameters. */
    STATUS_DB_NOT_FOUND   =    -4,    /**< status: database is not found. */
    STATUS_EVENT_STOP     =    -5,    /**< status: event stop looping. */
    STATUS_NO_REQUEST     =    -6,    /**< status: no request is found. */
    STATUS_LINK_EXIST     =    -7,    /**< status: link is already existed. */
    STATUS_MAX_LINK       =    -8,    /**< status: reach the max link number. */
    STATUS_NEED_RETRY     =    -9,    /**< status: the request need to be retried. */
    STATUS_REQUEST_EXIST  =    -10,   /**< status: the request is already existed. */
    STATUS_INVALID_STATUS =    -11,   /**< status: invalid status. */
    STATUS_USER_CANCEL    =    -12    /**< status: user cancel the action. */
} status_t;

/*==================app event define==================*/
typedef enum {
    EVENT_APP_STATE_CHANGE,
    EVENT_APP_TEST,
    EVENT_INPUT,
    EVENT_HOME,
    EVENT_FAULT,
    EVENT_MOTOR_STATE_CHANGE,
    EVENT_MOTOR_DIRECTION_CHANGE,
    EVENT_ALL,
} event_t;

typedef struct {
    uint8_t previous;
    uint8_t current;
} state_change_t;

typedef struct {
   uint8_t msg;
};

typedef struct {
    uint8_t num;
    uint8_t on_off;
} channel_id_t;

typedef struct {
    uint8_t pre_state;
    uint8_t cur_state;
} motor_state_change_t;

typedef struct {
    uint8_t pre_dir;
    uint8_t cur_dir;
} motor_direction_change_t;

typedef union {
    state_change_t state_change;            /**<  State change. */
    channel_id_t channel_id;
    motor_state_change_t motor_state_change;
    motor_direction_change_t motor_dir_change;
} event_param_t;
/*================app event define end================*/

typedef status_t (*app_event_callback_t)(event_t event_id, void *parameters);
typedef void (*app_event_post_result_t)(event_t event_id, status_t result, void *parameters);

typedef struct event_node_t {
    struct event_node_t *previous;
    struct event_node_t *next;
} app_event_node_t;

typedef struct {
    event_t event_id;
    app_event_callback_t callback;
} app_event_callback_table_t;

typedef struct {
    event_t event_id;
    void *parameters;
    app_event_post_result_t post_callback;
} app_event_t;

typedef struct {
    app_event_node_t pointer;
    event_t event_id;
    app_event_callback_t callback;
    bool dirty;
} app_event_callback_node_t;

void app_event_init(void);
void app_event_post(event_t event_id, void *parameters, app_event_post_result_t callback);
void app_event_register_callback(event_t event_id, app_event_callback_t callback);
void app_event_deregister_callback(event_t event_id, app_event_callback_t callback);
void app_event_process(app_event_t *event);
status_t app_event_handler(event_t event_id, void *parameters);
void app_event_post_callback(event_t event_id, status_t result, void *parameters);

#endif

