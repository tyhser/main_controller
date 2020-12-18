#ifndef _MOTOR_CONTROLLER_H
#define _MOTOR_CONTROLLER_H

typedef enum {
    MC_STATUS_SUCCESS  = 0,
    MC_STATUS_FAIL     = -1,

} motor_status_t;

typedef enum {
    MOTOR_ACT_MOVE,
    MOTOR_ACT_STOP,
    MOTOR_ACT_ALL,

} motor_action_id_t;

typedef struct {
    uint8_t motor_id;
    uint8_t dir;
    uint32_t steps;

} motor_action_move_t;

typedef union {
    motor_action_move_t act_move;
    uint8_t motor_id;

} motor_action_param_t;

typedef void (*motor_action_send_result_t)(motor_action_id_t act, status_t result, void *parameters);

typedef struct {
    motor_action_id_t act_id;
    void *parameters;
    motor_action_send_result_t send_callback;

} motor_action_t;

bool motor_controller_init(void);
void motor_controller_close(void);

void motor_send_action(motor_action_id_t id, motor_action_param_t *param);
#endif
