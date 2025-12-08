#include <msp430.h>
#include "drivers/mcu_init.h"
#include "drivers/io.h"

static inline void watchdog_setup(void)
{
    // Stop watchdog
    WDTCTL = WDTPW + WDTHOLD;

    // Re-purpose watchdog to count milliseconds instead (see millis.c)
    // WDTCTL = WDT_MDLY_0_5_16MHZ;
    // IE1 |= WDTIE;
}

void mcu_init(void)
{
    // Must stop and configure watchdog before anything else
    watchdog_setup();
    // init_clocks();
    // io_init();
    //  Enables globally
    //_enable_interrupts();
}
