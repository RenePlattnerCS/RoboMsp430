#ifndef INPUT_HISTORY
#define INPUT_HISTORY

#include "app/wall.h"

struct ring_buffer;

struct input
{
    struct walls wall;
};

void input_history_save(struct ring_buffer *history, const struct input *input);
struct walls input_history_last_detected_walls(const struct ring_buffer *history);

#endif // INPUT_HISTORY
