#include "app/input_history.h"
#include "common/ring_buffer.h"

static bool input_equal(const struct input *a, const struct input *b)
{
    return a->wall.positions == b->wall.positions
        && a->wall.range_front == b->wall.range_front
	&& a->wall.range_left == b->wall.range_left
	&& a->wall.range_right == b->wall.range_right;
}

void input_history_save(struct ring_buffer *history, const struct input *input)
{
    // Skip if no input detected
    if (input->wall.positions == WALL_POS_NONE) {
        return;
    }

    // Skip if identical input detected
    if (ring_buffer_count(history)) {
        struct input last_input;
        ring_buffer_peek_head(history, &last_input, 0);
        if (input_equal(input, &last_input)) {
            return;
        }
    }

    ring_buffer_put(history, input);
}

struct walls input_history_last_detected_walls(const struct ring_buffer *history)
{
    for (uint8_t offset = 0; offset < ring_buffer_count(history); offset++) {
        struct input input;
        ring_buffer_peek_head(history, &input, offset);
        if (wall_at_left(&input.wall) || wall_at_right(&input.wall)) { //what about wall at front?
            return input.wall;
        }
    }
    const struct walls wall_none = { WALL_POS_NONE, WALL_RANGE_NONE, WALL_RANGE_NONE, WALL_RANGE_NONE };
    return wall_none;
}
