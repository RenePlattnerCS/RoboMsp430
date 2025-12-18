#include "app/state_explore.h"
#include "app/drive.h"
#include "app/timer.h"
#include "common/enum_to_string.h"
#include <stdbool.h>
#include "printf.h"
//#include "common/trace.h"
#define MOVE_MAX_CNT (3u)

struct move
{
    drive_dir_e dir;
    drive_speed_e speed;
    uint16_t duration;
};

struct explore_state
{
    struct move moves[MOVE_MAX_CNT];
    uint8_t move_cnt;
};

static const struct explore_state explore_states[] =
{
    [EXPLORE_REVERSE] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_REVERSE, DRIVE_SPEED_MAX, 800 } },
    },
    [EXPLORE_FORWARD] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_FORWARD, DRIVE_SPEED_FAST, 800 } },
    },
    [EXPLORE_ROTATE_LEFT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ROTATE_LEFT, DRIVE_SPEED_FAST, 850 } },
    },
    [EXPLORE_ROTATE_RIGHT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ROTATE_RIGHT, DRIVE_SPEED_FAST, 850 } },
    },
};

SUPPRESS_UNUSED
static const struct move *current_move(const struct state_explore_data *data)
{
    return &explore_states[data->state].moves[data->move_idx];
}

static explore_state_e next_explore_state(const struct state_explore_data *data)
{

	UNUSED(data);

	if (wall_at_right(&data->common->wall))
	{
		return EXPLORE_ROTATE_LEFT;
	} else if(wall_at_left(&data->common->wall)) {
		return EXPLORE_ROTATE_RIGHT;
	}

    //switch (rand() % 4) {
    //case 0: return EXPLORE_FORWARD;
    //case 1: return EXPLORE_TURN_LEFT;
    //case 2: return EXPLORE_TURN_RIGHT;
   // default:return EXPLORE_FORWARD;

	return EXPLORE_FORWARD;

    }

SUPPRESS_UNUSED
static void start_explore_move(const struct state_explore_data *data)
{
    const struct move move = explore_states[data->state].moves[data->move_idx];
    timer_start(data->common->timer, move.duration);
    drive_set(move.dir, move.speed);
}

SUPPRESS_UNUSED
static bool explore_state_done(const struct state_explore_data *data)
{
    return data->move_idx == explore_states[data->state].move_cnt;
}

SUPPRESS_UNUSED
static void state_explore_run(struct state_explore_data *data)
{
    data->move_idx = 0;
    data->state = next_explore_state(data);
    start_explore_move(data);
}

// No blocking code (e.g. busy wait) allowed in this function
void state_explore_enter(struct state_explore_data *data, state_e from, state_event_e event)
{
	if(from != STATE_EXPLORE)
                printf("explore entered");

switch(from){
case STATE_MANUAL:

		break;

case STATE_EXPLORE:
    switch (event) {

    case STATE_EVENT_COMMAND:
        // handled by top-level FSM
        break;

    case STATE_EVENT_WALL:
        data->move_idx = 0;
        data->state = next_explore_state(data);
        //start_explore_move(data);
	state_explore_run(data);
        break;

    case STATE_EVENT_TIMEOUT:
        data->move_idx = 0;
        data->state = next_explore_state(data);
        state_explore_run(data);
        break;

    case STATE_EVENT_NONE:
        //data->move_idx = 0;
        //data->state = EXPLORE_FORWARD;
        //state_explore_run(data);
        break;

    default:
        break;
    }
break;

default:
break;
/*
        case STATE_EVENT_TIMEOUT:
            data->move_idx++;
            if (retreat_state_done(data)) {
                state_machine_post_internal_event(data->common->state_machine_data,
                                                  STATE_EVENT_FINISHED);
            } else {
                start_retreat_move(data);
            }
    */
	}
}



void state_explore_init(struct state_explore_data *data)
{
    data->state = EXPLORE_FORWARD;
    data->move_idx = 0;
}
