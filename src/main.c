#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "drivers/led.h"

static void test_io_blink_led(void)
{
    led_state_e l_state = LED_STATE_OFF;
    const struct io_config led_config = { .dir = IO_DIR_OUTPUT,
                                          .select = IO_SELECT_GPIO,
                                          .resistor = IO_RESISTOR_DISABLED,
                                          .out = IO_OUT_LOW };
    io_configure(IO_TEST_LED, &led_config);
    // io_out_e out = IO_OUT_LOW;
    while (1) {
        l_state = (l_state == LED_STATE_OFF) ? LED_STATE_ON : LED_STATE_OFF;
        led_set(LED_TEST, l_state);
        __delay_cycles(250000);
    }
}

int main(void)
{
    mcu_init();
    led_init();
    test_io_blink_led();

    return 0;
}
