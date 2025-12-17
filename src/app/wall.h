#ifndef WALL_H
#define WALL_H

/* A software layer that converts the range measuremetns into discrete
 * enemy position and distances to simplify the application code */

#include <stdbool.h>

typedef enum {
    WALL_POS_NONE,
    WALL_POS_FRONT_LEFT,
    WALL_POS_FRONT,
    WALL_POS_FRONT_RIGHT,
    WALL_POS_FRONT_AND_FRONT_LEFT,
    WALL_POS_FRONT_AND_FRONT_RIGHT,
    WALL_POS_LEFT_RIGHT,
    WALL_POS_FRONT_ALL
} wall_pos_e;

typedef enum {
    WALL_RANGE_NONE,
    WALL_RANGE_CLOSE,
    WALL_RANGE_MID,
    WALL_RANGE_FAR,
} wall_range_e;

struct walls
{
    wall_pos_e positions;
    wall_range_e range_front;
    wall_range_e range_left;
    wall_range_e range_right;
};

void wall_init(void);
struct walls wall_get(void);
bool wall_detected(const struct walls *walls);
bool wall_at_left(const struct walls *walls);
bool wall_at_right(const struct walls *walls);
bool wall_at_front(const struct walls *walls);

#endif // WALL_H
