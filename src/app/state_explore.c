#include "app/state_explore.h"
#include "app/timer.h"
#include "common/enum_to_string.h"
#include <stdbool.h>
#include "printf.h"
//#include "common/trace.h"
#define MOVE_MAX_CNT (3u)
#include "common/random.h"

struct move
{
    drive_dir_e dir;
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
        .moves = { { DRIVE_DIR_REVERSE, 2800 } },
    },
    [EXPLORE_FORWARD] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_FORWARD, 2800 } },
    },
    [EXPLORE_ROTATE_LEFT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ROTATE_LEFT,2850 } },
    },
    [EXPLORE_ROTATE_RIGHT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ROTATE_RIGHT,2850 } },
    },
    [EXPLORE_ARCTURN_LEFT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ARCTURN_SHARP_LEFT,2850 } },
    },
    [EXPLORE_ARCTURN_RIGHT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ARCTURN_SHARP_RIGHT,2850 } },}
};


SUPPRESS_UNUSED
static const struct move *current_move(const struct state_explore_data *data)
{
    return &explore_states[data->state].moves[data->move_idx];
}

static explore_state_e next_explore_state(const struct state_explore_data *data)
{

 	if(wall_detected(&data->common->wall)){
		bool wall_front = false;
		bool wall_left = false;
		bool wall_right = false;
		wall_front = wall_at_front(&data->common->wall);
		wall_left = wall_at_left(&data->common->wall);
		wall_right = wall_at_right(&data->common->wall);

		if(wall_front && wall_left && wall_right)
		{
			return EXPLORE_REVERSE;
		}
	        if(wall_left && wall_front){
                	return EXPLORE_ROTATE_RIGHT;
		}	
		if(wall_right && wall_front){
                        return EXPLORE_ROTATE_LEFT;
                }
		if(wall_front)
		{
			uint16_t dir = (rand_simple() >> 8) & 1;

			if(dir){
				return EXPLORE_ARCTURN_LEFT;
			} else {
				return EXPLORE_ARCTURN_RIGHT;
			}
		}
		if(wall_left){
			printf("wl: go right");
			return EXPLORE_ARCTURN_RIGHT;
		}
		if(wall_right){
			printf("wr: go left");
                        return EXPLORE_ARCTURN_LEFT;
                }
	}


    uint16_t dir = (rand_simple() >> 8) & 3;

    switch (dir) {
    case 0: 
	    return EXPLORE_FORWARD;
	    break;
    case 1: 
	    return EXPLORE_ARCTURN_LEFT;
	    break;
    case 2: 
	    return EXPLORE_ARCTURN_RIGHT;
	    break;
    default:return EXPLORE_FORWARD;


    }
}
SUPPRESS_UNUSED
static void start_explore_move(const struct state_explore_data *data)
{
    const struct move move = explore_states[data->state].moves[data->move_idx];
    timer_start(data->common->timer, move.duration);
    drive_set(move.dir, data->curr_speed);
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
                printf("explore entered\n");

switch(from){
case STATE_MANUAL:
	data->move_idx = 0;
        data->state = EXPLORE_FORWARD;
        state_explore_run(data);
		break;
	default:
		break;


case STATE_EXPLORE:	
	       	switch (event) {
    case STATE_EVENT_WALL:
	if(data->handling_wall == false){//react immeaditely to wall detection BUT not if we are already handling the wall
        	data->handling_wall = true;
		data->move_idx = 0;
		state_explore_run(data);
	}
        break;

    case STATE_EVENT_TIMEOUT:
	data->handling_wall = false;
        data->move_idx = 0;
        state_explore_run(data);
        break;

    case STATE_EVENT_COMMAND:
	switch(data->common->cmd){
	case IR_CMD_1:
        	data->curr_speed = DRIVE_SPEED_SLOW;
        break;
        case IR_CMD_2:
        	data->curr_speed = DRIVE_SPEED_MEDIUM;

        break;
        case IR_CMD_3:
        	data->curr_speed = DRIVE_SPEED_FAST;

        break;
        case IR_CMD_4:
        	data->curr_speed = DRIVE_SPEED_MAX;
		break;
	case IR_CMD_OK:
        drive_stop();
        state_machine_post_internal_event(data->common->state_machine_data, STATE_EVENT_OK);
	default:
		break;
	}

    case STATE_EVENT_NONE:
        break;

    default:
        break;
    }
}
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



void state_explore_init(struct state_explore_data *data)
{
    data->state = EXPLORE_FORWARD;
    data->move_idx = 0;
    data->curr_speed = DRIVE_SPEED_FAST;
    data->handling_wall = false;
}
