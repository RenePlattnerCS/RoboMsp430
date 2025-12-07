#include <msp430.h>
#include "io.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    P1DIR |= BIT0; // Set P1.0 (LED) as output

    while (1) {
        P1OUT ^= BIT0; // Toggle LED

        // Simple delay
        volatile unsigned int i;
        for (i = 0; i < 20000; i++)
            ;
    }

    return 0;
}
