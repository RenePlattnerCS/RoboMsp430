#include "app/state_manual.h"
#include "common/defines.h"
#include "common/trace.h"

// No blocking code (e.g. busy wait) allowed in this function
void state_manual_enter(struct state_manual_data *data, state_e from, state_event_e event)
{
  // TRACE("manual entered");
   UNUSED(from);

#ifndef DISABLE_IR_REMOTE
    if (event != STATE_EVENT_COMMAND) {
        return;
    }

    switch (data->common->cmd) {

    case IR_CMD_UP:
        drive_set(DRIVE_DIR_FORWARD, data->curr_speed);
	break;
    case IR_CMD_DOWN:
        drive_set(DRIVE_DIR_REVERSE, data->curr_speed);
        break;
    case IR_CMD_LEFT:
        drive_set(DRIVE_DIR_ROTATE_LEFT, data->curr_speed);
        break;
    case IR_CMD_RIGHT:
        drive_set(DRIVE_DIR_ROTATE_RIGHT, data->curr_speed);
        break;
    case IR_CMD_0:
	drive_stop();
	break;
    case IR_CMD_OK:
	drive_stop();
        state_machine_post_internal_event(data->common->state_machine_data, STATE_EVENT_OK);
        break;

    case IR_CMD_1:
	TRACE("1");
	data->curr_speed = DRIVE_SPEED_SLOW;
	break;
    case IR_CMD_2:
	TRACE("2");
	data->curr_speed = DRIVE_SPEED_MEDIUM;
        break;
    case IR_CMD_3:
	TRACE("3");
	data->curr_speed = DRIVE_SPEED_FAST;
        break;
    case IR_CMD_4:
	TRACE("4");
	data->curr_speed = DRIVE_SPEED_MAX;
        break;
    case IR_CMD_5:
    case IR_CMD_6:
    case IR_CMD_7:
    case IR_CMD_8:
    case IR_CMD_9:
    case IR_CMD_STAR:
    case IR_CMD_HASH:
    case IR_CMD_NONE:
        break;
    }
#else
    UNUSED(data);
    UNUSED(event);
#endif
}

void state_manual_init(struct state_manual_data *data)
{
    data->curr_speed = DRIVE_SPEED_MEDIUM;
}

