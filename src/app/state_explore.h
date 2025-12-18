#ifndef STATE_EXPLORE_H
#define STATE_EXPLORE_H

// Drive away from the detected line

#include "app/state_common.h"

typedef enum {
    EXPLORE_REVERSE,
    EXPLORE_FORWARD,
    EXPLORE_ROTATE_LEFT,
    EXPLORE_ROTATE_RIGHT,
   // EXPLORE_STATE_ARCTURN_LEFT,
   // EXPLORE_STATE_ARCTURN_RIGHT,
} explore_state_e;

struct state_explore_data
{
    const struct state_common_data *common;
    explore_state_e state;
    int move_idx;
};

void state_explore_init(struct state_explore_data *data);
void state_explore_enter(struct state_explore_data *data, state_e from, state_event_e event);

#endif // STATE_EXPLORE_H
