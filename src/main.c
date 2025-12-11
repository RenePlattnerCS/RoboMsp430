#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "drivers/led.h"
#include "uart.h"
#include "printf.h"

int main(void)
{
    mcu_init();
    uart_init();

    

    while (1) {
	    printf("Halo");
	    __delay_cycles(16000000);
    }

    return 0;
}
