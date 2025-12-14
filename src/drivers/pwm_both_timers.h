#ifndef PWM_H
#define PWM_H

// Driver that emulates hardware PWM with timer peripheral

#include <stdint.h>

typedef enum {
    PWM_LEFT,
    PWM_RIGHT
} pwm_e;

typedef enum {
    PWM_MAX_SPEED,
    PWM_HALF_PLUS_SPEED,
    PWM_HALF_SPEED,
    PWM_QUARTER_PLUS_SPEED,
    PWM_QUARTER_SPEED,
    PWM_EIGHTH_PLUS_SPEED,
    PWM_EIGHTH_SPEED,
    PWM_STOP_SPEED
} pwm_speed_e;

void pwm_both_timers_init(void);
void pwm_both_timers_set_duty_cycle(pwm_e pwm, pwm_speed_e speed);

#endif // PWM_H
