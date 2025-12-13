#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "printf.h"
#include "common/defines.h"
#include "drivers/ir_remote.h"
#include "common/trace.h"
#include "drivers/pwm.h"

SUPPRESS_UNUSED
static void test_pwm(void)
{
    mcu_init();
    trace_init();
   // ir_remote_init();
    pwm_init();
    const uint8_t duty_c = 50;
    const uint16_t wait_time = 3000;
    while (1) {
        TRACE("Duty Cicle: %d for %d ms", duty_c , wait_time);
        pwm_set_duty_cycle(PWM_TB6612FNG_LEFT, duty_c);
        BUSY_WAIT_ms(3000);
    }

}



SUPPRESS_UNUSED
static void test_ir(void)
{
    mcu_init();
    trace_init();
    ir_remote_init();
    while (1) {
        TRACE("cmd %d", ir_remote_get_cmd());
        BUSY_WAIT_ms(250);

        TRACE("------");
        BUSY_WAIT_ms(250);
    }

}



