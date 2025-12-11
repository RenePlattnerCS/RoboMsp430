#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "drivers/led.h"
#include "trace.h"
#include "printf.h"

int main(void)
{
    mcu_init();
    trace_init();

    

    while (1) {
	    TRACE("Hallo %d", 2121);
	    __delay_cycles(16000000);
    }

    return 0;
}
