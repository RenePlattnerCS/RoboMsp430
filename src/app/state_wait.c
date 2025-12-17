#include "app/state_wait.h"
#include "common/defines.h"
#include "common/trace.h"

// No blocking code (e.g. busy wait) allowed in this function
void state_wait_enter(struct state_wait_data *data, state_e from, state_event_e event)
{
//	TRACE("entered wait state!");
    UNUSED(from);
    UNUSED(data);
    UNUSED(event);
    // Command triggers transition
    // Note in actual sumobot competition this signal would come from another IR transceiver
    // than the one used here.
}
