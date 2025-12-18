#include "app/wall.h"
#include "drivers/vl53l0x.h"
#include "common/trace.h"

#define RANGE_DETECT_THRESHOLD (600u) // mm
#define INVALID_RANGE (UINT16_MAX)
#define RANGE_CLOSE (100u) // mm
#define RANGE_MID (500u) // mm
#define RANGE_FAR (700u) // mm

struct walls wall_get(void)
{
    struct walls wall = { WALL_POS_NONE, WALL_RANGE_NONE, WALL_RANGE_NONE, WALL_RANGE_NONE };
    vl53l0x_ranges_t ranges;
    bool fresh_values = false;
    vl53l0x_result_e result = vl53l0x_read_range_multiple(ranges, &fresh_values);
    if (result) {
        TRACE("read range failed %u", result);
        return wall;
    }

    const uint16_t range_front = ranges[VL53L0X_IDX_FRONT];
    const uint16_t range_front_left = ranges[VL53L0X_IDX_FRONT_LEFT];
    const uint16_t range_front_right = ranges[VL53L0X_IDX_FRONT_RIGHT];
#if 0 // Skip left and right (badly mounted on the robot)
    const uint16_t range_left = ranges[VL53L0X_IDX_LEFT];
    const uint16_t range_right = ranges[VL53L0X_IDX_RIGHT];
#endif

    const bool front = range_front < RANGE_DETECT_THRESHOLD;
    const bool front_left = range_front_left < RANGE_DETECT_THRESHOLD;
    const bool front_right = range_front_right < RANGE_DETECT_THRESHOLD;
#if 0 // Skip left and right (badly mounted on the robot)
    const bool left = range_left < RANGE_DETECT_THRESHOLD;
    const bool right = range_right < RANGE_DETECT_THRESHOLD;
#endif

    uint16_t range_f = INVALID_RANGE;
    uint16_t range_left = INVALID_RANGE;
    uint16_t range_right = INVALID_RANGE;
#if 0 // Skip left and right (badly mounted on the robot)
    if (left) {
        if (front_right || right) {
            wall.positions = WALL_POS_IMPOSSIBLE;
        } else {
            wall.positions = WALL_POS_LEFT;
            range = range_left;
        }
    } else if (right) {
        if (front_left || left) {
            wall.positions = WALL_POS_IMPOSSIBLE;
        } else {
            wall.positions = WALL_POS_RIGHT;
            range = range_right;
        }
    }
#endif

    //TODO: front left not average but 2 ranges
    if (front_left && front && front_right) {
        wall.positions = WALL_POS_FRONT_ALL;

        range_f = range_front;
	range_left = range_front_left;
	range_right = range_front_right;

    } else if (front_left && front_right) {
        wall.positions = WALL_POS_LEFT_RIGHT;	
    } else if (front_left) {
        if (front) {
            wall.positions = WALL_POS_FRONT_AND_FRONT_LEFT;
	    
	    range_f = range_front;
            range_left = range_front_left;

        } else {
            wall.positions = WALL_POS_FRONT_LEFT;

            range_left = range_front_left;

        }
    } else if (front_right) {
        if (front) {
            wall.positions = WALL_POS_FRONT_AND_FRONT_RIGHT;
        
	    range_f = range_front;
            range_right = range_front_right;


	} else {
            wall.positions = WALL_POS_FRONT_RIGHT;

            range_right = range_front_right;

        }
    } else if (front) {
        wall.positions = WALL_POS_FRONT;

	range_f = range_front;

    } else {
        wall.positions = WALL_POS_NONE;
    }


    if (range_f < RANGE_CLOSE) {
        wall.range_front = WALL_RANGE_CLOSE;
    } else if (range_f < RANGE_MID) {
        wall.range_front = WALL_RANGE_MID;
    } else {
        wall.range_front = WALL_RANGE_FAR;
    }

    if (range_left < RANGE_CLOSE) {
        wall.range_left = WALL_RANGE_CLOSE;
    } else if (range_left < RANGE_MID) {
        wall.range_left = WALL_RANGE_MID;
    } else {
        wall.range_left = WALL_RANGE_FAR;
    }

    if (range_right < RANGE_CLOSE) {
        wall.range_right = WALL_RANGE_CLOSE;
    } else if (range_right < RANGE_MID) {
        wall.range_right = WALL_RANGE_MID;
    } else {
        wall.range_right = WALL_RANGE_FAR;
    }


    return wall;
}

bool wall_detected(const struct walls *walls)
{
    return walls->positions != WALL_POS_NONE;
}

//only return true if wall is MID range
bool wall_at_left(const struct walls *walls)
{
	if(walls->range_left == WALL_RANGE_FAR || walls->range_left == WALL_RANGE_NONE )
		return false;


    return walls->positions == WALL_POS_FRONT_LEFT
        || walls->positions == WALL_POS_FRONT_AND_FRONT_LEFT
	|| walls->positions == WALL_POS_FRONT_ALL;
}

bool wall_at_right(const struct walls *walls)
{
	if(walls->range_right == WALL_RANGE_FAR || walls->range_right == WALL_RANGE_NONE )
                return false;
    return walls->positions == WALL_POS_FRONT_RIGHT
        || walls->positions == WALL_POS_FRONT_AND_FRONT_RIGHT
	|| walls->positions == WALL_POS_FRONT_ALL;
}

bool wall_at_front(const struct walls *walls)
{
	if(walls->range_front == WALL_RANGE_FAR || walls->range_front == WALL_RANGE_NONE )
                return false;


    return walls->positions == WALL_POS_FRONT || walls->positions == WALL_POS_FRONT_ALL
	    || walls->positions == WALL_POS_FRONT_AND_FRONT_RIGHT
	    || walls->positions == WALL_POS_FRONT_AND_FRONT_LEFT;
}




void wall_init(void)
{
    vl53l0x_result_e result = vl53l0x_init();
    if (result) {
        TRACE("Failed to initialize vl53l0x %u", result);
        return;
    }
}
