#ifndef STATE_MANUAL_H
#define STATE_MANUAL_H

#include "app/state_common.h"
#include "app/drive.h"

// Manual control with IR remote

struct state_manual_data
{
    const struct state_common_data *common;
    drive_speed_e curr_speed;
};

void state_manual_enter(struct state_manual_data *data, state_e from, state_event_e event);

void state_manual_init(struct state_manual_data *data);
#endif // STATE_MANUAL_H
