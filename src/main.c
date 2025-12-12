#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "drivers/led.h"
#include "trace.h"
#include "printf.h"
#include "drivers/ir_remote.h"
#include "common/defines.h"

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

int main(void)
{
    test_ir();
    return 0;
}
