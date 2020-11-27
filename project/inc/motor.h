#ifndef _MOTOR_H
#define _MOTOR_H
#include <stdbool.h>
#define MOTOR_SYRINGE_ID    1
#define MOTOR_X_AXIS_ID     2
#define MOTOR_Z_AXIS_ID     3
#define MOTOR_RECEIVED_ID   4
#define MOTOR_END_ID    5
typedef uint8_t motor_id_t;

#define DIRECTION_FWD 1
#define DIRECTION_REV 0
typedef bool direction_t;

#define VALVE_1 1
#define VALVE_2 2
#define VALVE_3 3
#define VALVE_4 4
#define VALVE_5 5
#define VALVE_6 6
#define VALVE_END 7
typedef uint8_t valve_id_t;

#define VALVE_STATE_OPEN  1
#define VALVE_STATE_CLOSE 0
typedef bool valve_state_t;

#define MOTOR_RUN 1
#define MOTOR_STOP 0
typedef bool motor_state_t;

bool is_motor_zero(motor_id_t motor_id);

void set_motor_direction(motor_id_t motor_id, direction_t direction);

void set_motor_run_stop(motor_id_t id, motor_state_t state);

void set_valve_state(valve_id_t valve, valve_state_t state);
#endif
