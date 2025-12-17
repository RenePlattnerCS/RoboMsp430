#include "app/state_stop.h"
#include "app/drive.h"
#include "app/timer.h"
#include "common/defines.h"

#define STOP_STATE_TIMEOUT (5000u)

SUPPRESS_UNUSED
static void state_stop_run(const struct state_stop_data *data)
{
    switch (data->state) {
    case STOP_STATE_FORWARD:
        drive_set(DRIVE_DIR_FORWARD, DRIVE_SPEED_FAST);
        break;
    case STOP_STATE_LEFT:
        drive_set(DRIVE_DIR_ARCTURN_WIDE_LEFT, DRIVE_SPEED_FAST);
        break;
    case STOP_STATE_RIGHT:
        drive_set(DRIVE_DIR_ARCTURN_WIDE_RIGHT, DRIVE_SPEED_FAST);
        break;
    }
    timer_start(data->common->timer, STOP_STATE_TIMEOUT);
}

SUPPRESS_UNUSED
static stop_state_e next_stop_state(const struct walls *wall)
{
    if (wall_at_front(wall)) {
        return STOP_STATE_FORWARD;
    } else if (wall_at_left(wall)) {
        return STOP_STATE_LEFT;
    } else if (wall_at_right(wall)) {
        return STOP_STATE_RIGHT;
    } else {
	   // TRACE("something went wrong")
    }
    return STOP_STATE_FORWARD;
}

// No blocking code (e.g. busy wait) allowed in this function

void state_stop_enter(struct state_stop_data *data, state_e from, state_event_e event)
{
	UNUSED(data);
	UNUSED(from);
	UNUSED(event);
	/*
    const stop_state_e prev_stop_state = data->state;
    data->state = next_stop_state(&data->common->wall);

    switch (from) {
    case STATE_SEARCH:
        switch (event) {
        case STATE_EVENT_ENEMY:
            state_stop_run(data);
            break;
        case STATE_EVENT_TIMEOUT:
        case STATE_EVENT_LINE:
        case STATE_EVENT_FINISHED:
        case STATE_EVENT_COMMAND:
        case STATE_EVENT_NONE:
            ASSERT(0);
            break;
        }
        break;
    case STATE_STOP:
        switch (event) {
        case STATE_EVENT_ENEMY:
            if (prev_stop_state != data->state) {
                state_stop_run(data);
            }
            break;
        case STATE_EVENT_TIMEOUT:
            // TODO: Consider adding a breakout strategy instead of asserting
            ASSERT(0);
            break;
        case STATE_EVENT_LINE:
        case STATE_EVENT_FINISHED:
        case STATE_EVENT_COMMAND:
        case STATE_EVENT_NONE:
            ASSERT(0);
            break;
        }
        break;
    case STATE_RETREAT:
        // Should always go via search state
        ASSERT(0);
        break;
    case STATE_WAIT:
    case STATE_MANUAL:
        ASSERT(0);
        break;


    }

    */
}

void state_stop_init(struct state_stop_data *data)
{
    data->state = STOP_STATE_FORWARD;
}
