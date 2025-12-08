#include <msp430.h>
#include "io.h"

static void test_io_blink_led(void)
{

    const struct io_config led_config = { .dir = IO_DIR_OUTPUT,
                                          .select = IO_SELECT_GPIO,
                                          .resistor = IO_RESISTOR_DISABLED,
                                          .out = IO_OUT_LOW };
    io_configure(IO_TEST_LED, &led_config);
    io_out_e out = IO_OUT_LOW;
    return;
    while (1) {
        out = (out == IO_OUT_LOW) ? IO_OUT_HIGH : IO_OUT_LOW;
        io_set_out(IO_TEST_LED, out);
        __delay_cycles(250000);
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    P1DIR |= BIT0; // Set P1.0 as output
    P1OUT |= BIT0;
    test_io_blink_led();

    return 0;
}
