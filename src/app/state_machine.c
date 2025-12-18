#include "app/state_machine.h"
#include "app/state_common.h"
#include "app/state_wait.h"
#include "app/state_explore.h"
#include "app/state_stop.h"
#include "app/state_manual.h"
#include "app/timer.h"
#include "app/input_history.h"
//#include "common/trace.h"
#include "common/defines.h"
#include "common/enum_to_string.h"
#include "common/ring_buffer.h"
#include "app/wall.h"
#include "printf.h"
/* A state machine implemented as a set of enums and functions. The states are linked through
 * transitions, which are triggered by events.
 *
 * Flow:
 *    1. Process input
 *        - Check input (e.g. sensors, timer, internal event...)
 *        - Return event
 *    2. Process event
 *        - State/Change state
 *        - Run state function
 *    3. Repeat
 *
 * The flow is continuous (never blocks), which avoids the need for event synchronization
 * mechanisms, since the input can be processed repeatedly at the beginning of each iteration
 * instead. No input is still treated as an event (STATE_EVENT_NONE), but treated as a NOOP
 * when processed. Of course, this means that the code inside the state machine can't block.
 */

struct state_transition
{
    state_e from;
    state_event_e event;
    state_e to;
};

// See docs/state_machine.png (docs/state_machine.uml)
static const struct state_transition state_transitions[] = {
    { STATE_WAIT, STATE_EVENT_NONE, STATE_WAIT },
    { STATE_WAIT, STATE_EVENT_COMMAND, STATE_MANUAL }, 
    { STATE_EXPLORE, STATE_EVENT_NONE, STATE_EXPLORE },
    { STATE_EXPLORE, STATE_EVENT_TIMEOUT, STATE_EXPLORE },
    { STATE_EXPLORE, STATE_EVENT_COMMAND, STATE_EXPLORE },
    { STATE_EXPLORE, STATE_EVENT_WALL, STATE_EXPLORE },
    { STATE_STOP, STATE_EVENT_TIMEOUT, STATE_STOP },
    { STATE_STOP, STATE_EVENT_NONE, STATE_STOP },
    { STATE_STOP, STATE_EVENT_COMMAND, STATE_STOP },
    { STATE_MANUAL, STATE_EVENT_COMMAND, STATE_MANUAL },
    { STATE_MANUAL, STATE_EVENT_NONE, STATE_MANUAL },
    { STATE_MANUAL, STATE_EVENT_OK, STATE_EXPLORE },
};

struct state_machine_data
{
    state_e state;
    struct state_common_data common;
    struct state_wait_data wait;
    struct state_explore_data explore;
    struct state_stop_data stop;
    struct state_manual_data manual;
    state_event_e internal_event;
    timer_t timer;
    struct ring_buffer input_history;
};

static inline bool has_internal_event(const struct state_machine_data *data)
{
    return data->internal_event != STATE_EVENT_NONE;
}

static inline state_event_e take_internal_event(struct state_machine_data *data)
{
    const state_event_e event = data->internal_event;
    data->internal_event = STATE_EVENT_NONE;
    return event;
}

void state_machine_post_internal_event(struct state_machine_data *data, state_event_e event)
{
    if(!has_internal_event(data))
    {
    	//TRACE("ERROR: pending internal event!");
    }
    if (data->internal_event == STATE_EVENT_NONE) {
        data->internal_event = event;
    }
}

static void state_enter(struct state_machine_data *data, state_e from, state_event_e event,
                        state_e to)
{
    if (from != to) {
        timer_clear(&data->timer);
        data->state = to;
        //TRACE("%s to %s (%s)", state_to_string(from), state_to_string(event),
    //          state_event_to_string(to));
    }
    switch (to) {
    case STATE_WAIT:
        state_wait_enter(&(data->wait), from, event);
        break;
    case STATE_EXPLORE:
        state_explore_enter(&(data->explore), from, event);
        break;
    case STATE_STOP:
        state_stop_enter(&(data->stop), from, event);
        break;
    case STATE_MANUAL:
        state_manual_enter(&(data->manual), from, event);
        break;
    }
}

static inline void process_event(struct state_machine_data *data, state_event_e next_event)
{
	//find a match in table => transitiont => enter next state
    for (uint16_t i = 0; i < ARRAY_SIZE(state_transitions); i++) {
        if (data->state == state_transitions[i].from && next_event == state_transitions[i].event) {
		
            state_enter(data, state_transitions[i].from, next_event, state_transitions[i].to);
            return;
        }
    }
}

static inline state_event_e process_input(struct state_machine_data *data)
{
    data->common.wall = wall_get(); //get walls position > save in history
    data->common.cmd = ir_remote_get_cmd_ta1();
    const struct input input = { .wall = data->common.wall};
    
    input_history_save(&data->input_history, &input);

    if (data->common.cmd != IR_CMD_NONE) {
             return STATE_EVENT_COMMAND;
        
    } else if (has_internal_event(data)) {
        return take_internal_event(data);
    } else if (timer_timeout(&data->timer)) {
        timer_clear(&data->timer);
        return STATE_EVENT_TIMEOUT;
    } else if (wall_detected(&data->common.wall)) {
        return STATE_EVENT_WALL;
    }
    return STATE_EVENT_NONE;
}

static inline void state_machine_init(struct state_machine_data *data)
{
    data->state = STATE_WAIT;
    data->common.state_machine_data = data;
    data->common.wall.positions = WALL_POS_NONE;
    data->common.wall.range_front = WALL_RANGE_NONE;
    data->common.wall.range_left = WALL_RANGE_NONE;
    data->common.wall.range_right =  WALL_RANGE_NONE;

    data->common.cmd = IR_CMD_NONE;
    data->common.timer = &data->timer;
    timer_clear(&data->timer);

    data->internal_event = STATE_EVENT_NONE;
    data->wait.common = &data->common;
    data->explore.common = &data->common;
    data->stop.common = &data->common;
    data->manual.common = &data->common;

    state_explore_init(&data->explore);
    state_stop_init(&data->stop);
    state_manual_init(&data->manual);
}

#define INPUT_HISTORY_BUFFER_SIZE (6u)
void state_machine_run(void)
{
    struct state_machine_data data;
    // Allocate input history here so the internal buffer remains allocated
    LOCAL_RING_BUFFER(input_history, INPUT_HISTORY_BUFFER_SIZE, struct input);
    data.input_history = input_history;
    data.common.input_history = &data.input_history;

    state_machine_init(&data);

    while (1) {
        const state_event_e next_event = process_input(&data);
        process_event(&data, next_event);
    }
}
