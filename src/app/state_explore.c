#include "app/state_explore.h"
#include "app/drive.h"
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
        .moves = { { DRIVE_DIR_REVERSE, DRIVE_SPEED_MAX, 2800 } },
    },
    [EXPLORE_FORWARD] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_FORWARD, DRIVE_SPEED_FAST, 2800 } },
    },
    [EXPLORE_ROTATE_LEFT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ROTATE_LEFT, DRIVE_SPEED_FAST,2850 } },
    },
    [EXPLORE_ROTATE_RIGHT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ROTATE_RIGHT, DRIVE_SPEED_FAST,2850 } },
    },
    [EXPLORE_ARCTURN_LEFT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ARCTURN_MID_LEFT, DRIVE_SPEED_FAST,2850 } },
    },
    [EXPLORE_ARCTURN_RIGHT] =
    {
        .move_cnt = 1,
        .moves = { { DRIVE_DIR_ARCTURN_MID_RIGHT, DRIVE_SPEED_FAST,2850 } },}
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
			printf("wall reverse");
			return EXPLORE_REVERSE;
		}
	        if(wall_left){
			printf("Wall move R\n");
                	return EXPLORE_ROTATE_RIGHT;
		}	
		if(wall_right){
                        printf("Wall move L\n");
                        return EXPLORE_ROTATE_LEFT;
                }
		if(wall_front)
		{
			printf("wall in front\n");
			uint16_t dir = (rand_simple() >> 8) & 1;

			if(dir){
				printf("wall in front go left\n");
				return EXPLORE_ARCTURN_LEFT;
			} else {
				printf("wall in front go right\n");
				return EXPLORE_ARCTURN_RIGHT;
			}
		}
	}


    uint16_t dir = (rand_simple() >> 8) & 3;

    switch (dir) {
    case 0: 
	    printf("rand: F\n");
	    return EXPLORE_FORWARD;
	    break;
    case 1: 
	    printf("rand: L\n");
	    return EXPLORE_ARCTURN_LEFT;
	    break;
    case 2: 
	    printf("rand: R\n");
	    return EXPLORE_ARCTURN_RIGHT;
	    break;
    default:return EXPLORE_FORWARD;

	//return EXPLORE_FORWARD;

    }
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
    case STATE_EVENT_COMMAND:
        // handled by top-level FSM
        break;

    case STATE_EVENT_WALL:
	if(data->handling_wall == false){//react immeaditely to wall detection BUT not if we are already handling the wall
        	data->handling_wall = true;
		data->move_idx = 0;
		state_explore_run(data);
	}
        break;

    case STATE_EVENT_TIMEOUT:
	printf("T\n");
	data->handling_wall = false;
        data->move_idx = 0;
        //data->state = next_explore_state(data);
        state_explore_run(data);
        break;

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
    data->handling_wall = false;
}
