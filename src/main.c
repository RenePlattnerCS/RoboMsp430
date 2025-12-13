#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "trace.h"
#include "printf.h"
#include "drivers/ir_remote.h"
#include "common/defines.h"
#include "drivers/pwm_both_timers.h"
#include "drivers/tb6612fng.h"

SUPPRESS_UNUSED
static void test_motor(void)
{
    mcu_init();
    trace_init();
    
    tb6612fng_init();
    const uint8_t duty_c = 80; 
    const uint8_t duty_c2 = 25;   
//    tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_FORWARD);
//    tb6612fng_set_pwm(TB6612FNG_LEFT , duty_c);

    while (1) {
        TRACE("test motor 2");
	tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_FORWARD);
	tb6612fng_set_pwm(TB6612FNG_LEFT , duty_c);
	

	tb6612fng_set_mode(TB6612FNG_RIGHT, TB6612FNG_MODE_FORWARD);
        tb6612fng_set_pwm(TB6612FNG_RIGHT , duty_c2);

	BUSY_WAIT_ms(3000);
    }
}

SUPPRESS_UNUSED
static void test_pwm_timers(void)
{
    mcu_init();
    trace_init();
   // ir_remote_init();
   pwm_both_timers_init();
   
    const uint8_t duty_c = 50;
    const uint16_t wait_time = 3000;
    while (1) {
        TRACE("Duty Cicle: %d for %d ms", duty_c , wait_time);
        pwm_both_timers_set_duty_cycle(PWM_LEFT, duty_c);
        pwm_both_timers_set_duty_cycle(PWM_RIGHT, duty_c);
	BUSY_WAIT_ms(3000);
    }

}


int main(void)
{
   // test_pwm_timers();
   test_motor();
    return 0;
}
