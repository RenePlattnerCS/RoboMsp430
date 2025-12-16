#include "drivers/tb6612fng.h"
#include "drivers/io.h"

struct cc_pins
{
    io_e cc1;
    io_e cc2;
};

static struct cc_pins tb6612fng_cc_pins[] = {
    [TB6612FNG_LEFT] = { IO_MOTORS_LEFT_CC_1, IO_MOTORS_LEFT_CC_2 },
    [TB6612FNG_RIGHT] = { IO_MOTORS_RIGHT_CC_2 , IO_MOTORS_RIGHT_CC_2 }, //IO motor right CC!!!!! ^^
};

void tb6612fng_set_mode(tb6612fng_e tb, tb6612fng_mode_e mode)
{
    switch (mode) {
    case TB6612FNG_MODE_STOP:
        io_set_out(tb6612fng_cc_pins[tb].cc1, IO_OUT_LOW);
        io_set_out(tb6612fng_cc_pins[tb].cc2, IO_OUT_LOW);
        break;
    case TB6612FNG_MODE_FORWARD:
        io_set_out(tb6612fng_cc_pins[tb].cc1, IO_OUT_HIGH);
        io_set_out(tb6612fng_cc_pins[tb].cc2, IO_OUT_LOW);
        break;
    case TB6612FNG_MODE_REVERSE:
        io_set_out(tb6612fng_cc_pins[tb].cc1, IO_OUT_LOW);
        io_set_out(tb6612fng_cc_pins[tb].cc2, IO_OUT_HIGH);
        break;
    }
}

void tb6612fng_set_pwm(tb6612fng_e tb, pwm_speed_e speed)
{
   pwm_both_timers_set_duty_cycle(tb, speed);
}


void tb6612fng_init(void)
{
    pwm_both_timers_init();
}
