#include "app/drive.h"
#include "drivers/tb6612fng.h"
#include "common/defines.h"
#include <stdbool.h>
#include "drivers/pwm_both_timers.h"

struct drive_speeds
{
    pwm_speed_e left;
    pwm_speed_e right;
};

/* Drive directions come in pair (e.g. FORWARD and REVERSE, ROTATE_LEFT and ROTATE_RIGHT).
 * To save flash space and minimize typos, only save the speeds for one direction (primary),
 * and create a macro to get the corresponding primary direction for every direction and
 * inverse the speeds when its not the primary direction. */

static const struct drive_speeds drive_primary_speeds[][4] =
{
    [DRIVE_DIR_FORWARD] = {
        [DRIVE_SPEED_SLOW] = {PWM_QUARTER_SPEED, PWM_QUARTER_SPEED},
        [DRIVE_SPEED_MEDIUM] = {PWM_QUARTER_PLUS_SPEED, PWM_QUARTER_PLUS_SPEED},
        [DRIVE_SPEED_FAST] = {PWM_HALF_SPEED, PWM_HALF_SPEED},
        [DRIVE_SPEED_MAX] = {PWM_MAX_SPEED, PWM_MAX_SPEED},
    },
    [DRIVE_DIR_ROTATE_LEFT] =
    {
        [DRIVE_SPEED_SLOW] = {PWM_QUARTER_SPEED, PWM_QUARTER_SPEED},
        [DRIVE_SPEED_MEDIUM] = {PWM_HALF_SPEED, PWM_HALF_SPEED},
        [DRIVE_SPEED_FAST] = {PWM_HALF_PLUS_SPEED, PWM_HALF_PLUS_SPEED},
        [DRIVE_SPEED_MAX] = {PWM_MAX_SPEED, PWM_MAX_SPEED},
    },
    [DRIVE_DIR_ARCTURN_SHARP_LEFT] =
    {
        [DRIVE_SPEED_SLOW] = {PWM_EIGHTH_SPEED, PWM_QUARTER_SPEED},
        [DRIVE_SPEED_MEDIUM] = {PWM_EIGHTH_SPEED, PWM_HALF_SPEED},
        [DRIVE_SPEED_FAST] = {PWM_QUARTER_SPEED, PWM_HALF_PLUS_SPEED},
        [DRIVE_SPEED_MAX] = {PWM_EIGHTH_PLUS_SPEED, PWM_MAX_SPEED},
    },
    [DRIVE_DIR_ARCTURN_MID_LEFT] =
    {
        [DRIVE_SPEED_SLOW] = {PWM_EIGHTH_PLUS_SPEED, PWM_QUARTER_SPEED},
        [DRIVE_SPEED_MEDIUM] = {PWM_QUARTER_SPEED, PWM_HALF_SPEED},
        [DRIVE_SPEED_FAST] = {PWM_QUARTER_PLUS_SPEED, PWM_HALF_PLUS_SPEED},
        [DRIVE_SPEED_MAX] = {PWM_HALF_SPEED, PWM_MAX_SPEED},
    },
    [DRIVE_DIR_ARCTURN_WIDE_LEFT] =
    {
        [DRIVE_SPEED_SLOW] = {PWM_EIGHTH_PLUS_SPEED, PWM_QUARTER_SPEED},
        [DRIVE_SPEED_MEDIUM] = {PWM_QUARTER_PLUS_SPEED, PWM_HALF_SPEED},
        [DRIVE_SPEED_FAST] = {PWM_HALF_SPEED, PWM_HALF_PLUS_SPEED},
        [DRIVE_SPEED_MAX] = {PWM_HALF_PLUS_SPEED, PWM_MAX_SPEED},
    },
};

static inline drive_dir_e drive_primary_direction(drive_dir_e dir)
{
    switch (dir) {
    case DRIVE_DIR_REVERSE:        return DRIVE_DIR_FORWARD;
    case DRIVE_DIR_ROTATE_RIGHT:   return DRIVE_DIR_ROTATE_LEFT;
    default:                       return dir;
    }
}


void drive_set(drive_dir_e direction, drive_speed_e speed)
{
bool left_forward  = true;
bool right_forward = true;

switch (direction) {
case DRIVE_DIR_FORWARD:
    break;

case DRIVE_DIR_REVERSE:
    left_forward = false;
    right_forward = false;
    break;

case DRIVE_DIR_ROTATE_LEFT:
    left_forward  = false;
    right_forward = true;
    break;

case DRIVE_DIR_ROTATE_RIGHT:
    left_forward  = true;
    right_forward = false;
    break;

default:
    left_forward = true;
    right_forward = true;
    break;
}

   
    drive_dir_e primary_direction = drive_primary_direction(direction);
    pwm_speed_e speed_left = drive_primary_speeds[primary_direction][speed].left;
    pwm_speed_e speed_right = drive_primary_speeds[primary_direction][speed].right;
    
    tb6612fng_set_mode(TB6612FNG_LEFT,
    left_forward ? TB6612FNG_MODE_FORWARD : TB6612FNG_MODE_REVERSE);

    tb6612fng_set_mode(TB6612FNG_RIGHT,
    right_forward ? TB6612FNG_MODE_FORWARD : TB6612FNG_MODE_REVERSE);

    
    tb6612fng_set_pwm(TB6612FNG_LEFT, speed_left);
    tb6612fng_set_pwm(TB6612FNG_RIGHT, speed_right);
}

void drive_stop(void)
{
    tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_STOP);
    tb6612fng_set_mode(TB6612FNG_RIGHT, TB6612FNG_MODE_STOP);
    tb6612fng_set_pwm(TB6612FNG_LEFT, PWM_STOP_SPEED);
    tb6612fng_set_pwm(TB6612FNG_RIGHT, PWM_STOP_SPEED);
}

void drive_init(void)
{
    tb6612fng_init();
}
