#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "drivers/led.h"
#include "trace.h"
#include "printf.h"
#include "drivers/ir_remote.h"
#include "common/defines.h"
#include "drivers/pwm.h"
#include "drivers/tb6612fng.h"

static void test_motor(void)
{
    mcu_init();
    trace_init();
    
    tb6612fng_init();
    const uint8_t duty_c = 100; 
       
//    tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_FORWARD);
//    tb6612fng_set_pwm(TB6612FNG_LEFT , duty_c);

    while (1) {
        TRACE("test motor");
	tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_FORWARD);
	tb6612fng_set_pwm(TB6612FNG_LEFT , duty_c);
	BUSY_WAIT_ms(3000);
    }
}

int main(void)
{
    test_motor();
    return 0;
}
