#ifndef _MOTOR_H
#define _MOTOR_H
#include <stdbool.h>
#include "app_event.h"

#define DENOMINATOR 32

#define MOTOR_SYRINGE_ID    1
#define MOTOR_X_AXIS_ID     2
#define MOTOR_Z_AXIS_ID     3
#define MOTOR_RECEIVED_ID   4
#define MOTOR_END_ID  MOTOR_RECEIVED_ID+1
typedef uint8_t motor_id_t;

#define VALVE_1 5
#define VALVE_2 6
#define VALVE_3 7
#define VALVE_4 8
#define VALVE_5 9
#define VALVE_6 10
typedef uint8_t valve_id_t;

#define MOTOR_VALVE_ALL 0x0d

#define DIRECTION_FWD 1
#define DIRECTION_REV 0
typedef bool direction_t;

#define VALVE_STATE_OPEN  1
#define VALVE_STATE_CLOSE 0
typedef bool valve_state_t;

#define MOTOR_RUN 1
#define MOTOR_STOP 0
typedef bool motor_state_t;

void motor_init(void);
bool is_motor_zero(motor_id_t motor_id);
void motor_zero_handler(uint8_t index, uint8_t on_off);
void set_motor_direction(motor_id_t motor_id, direction_t direction);
void motor_stop(motor_id_t id);
void motor_run_steps(motor_id_t id, uint32_t step);
void set_valve_state(valve_id_t valve, valve_state_t state);
bool set_motor_state(motor_id_t id, motor_state_t s);

void motor_run(motor_id_t id, uint32_t distance, direction_t dir);

status_t motor_event_handler(event_t event_id, void *parameters);

bool motor_run_with_sspeed(motor_id_t id, uint32_t distance, direction_t dir);
#endif
