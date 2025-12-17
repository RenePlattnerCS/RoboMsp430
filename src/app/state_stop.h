#ifndef STATE_STOP_H
#define STATE_STOP_H

#include "app/state_common.h"

// Drive towards detected enemy

typedef enum {
    STOP_STATE_FORWARD,
    STOP_STATE_LEFT,
    STOP_STATE_RIGHT
} stop_state_e;

struct state_stop_data
{
    const struct state_common_data *common;
    stop_state_e state;
};

void state_stop_init(struct state_stop_data *data);
void state_stop_enter(struct state_stop_data *data, state_e from, state_event_e event);

#endif // STATE_STOP_H
