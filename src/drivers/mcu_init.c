#include <msp430.h>
#include "drivers/mcu_init.h"
#include "drivers/io.h"
#include "led.h"

// 16 MHz / 32768 = ~2000 Hz
#define WDT_MDLY_0_5_16MHZ (WDTPW + WDTTMSEL + WDTCNTCL + WDTIS0)

static void init_clocks()
{
    led_init();
    led_state_e l_state = LED_STATE_OFF;
    const struct io_config led_config = { .dir = IO_DIR_OUTPUT,
                                          .select = IO_SELECT_GPIO,
                                          .resistor = IO_RESISTOR_DISABLED,
                                          .out = IO_OUT_LOW };
    io_configure(IO_TEST_LED, &led_config);
    
    if (CALBC1_16MHZ == 0xFF || CALDCO_16MHZ == 0xFF) {
        l_state = LED_STATE_ON;
        led_set(LED_TEST, l_state);
        while(1); // halt if calibration erased
    }
    
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
    
    // **CRITICAL: Explicitly configure SMCLK**
    BCSCTL2 = 0;  // SMCLK = DCO/1, MCLK = DCO/1
    
    BCSCTL3 = LFXT1S_2;
}


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
    init_clocks();
    io_init();


    //  Enables globally
    _enable_interrupts();
}
