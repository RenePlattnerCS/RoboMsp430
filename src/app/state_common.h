#ifndef STATE_COMMON_H
#define STATE_COMMON_H

#include "app/wall.h"
#include "drivers/ir_remote_ta1.h"
#include <stdint.h>

// Definitions common between states

typedef enum {
    STATE_WAIT,
    STATE_EXPLORE,
    STATE_STOP,
    STATE_MANUAL
} state_e;

typedef enum {
    STATE_EVENT_TIMEOUT,
    STATE_EVENT_WALL,
    // STATE_EVENT_FINISHED,
    STATE_EVENT_COMMAND,
    STATE_EVENT_NONE
} state_event_e;

struct state_machine_data;
typedef uint32_t timer_t;
struct ring_buffer;
struct state_common_data
{
    struct state_machine_data *state_machine_data;
    timer_t *timer;
    struct walls wall;
    ir_cmd_e cmd;
    struct ring_buffer *input_history;
};

// Post event from inside a state
void state_machine_post_internal_event(struct state_machine_data *data, state_event_e event);

#endif // STATE_COMMON_H
