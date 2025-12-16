#include <msp430.h>
#include "io.h"
#include "drivers/mcu_init.h"
#include "trace.h"
#include "printf.h"
#include "drivers/ir_remote_ta1.h"
#include "common/defines.h"
#include "drivers/pwm_both_timers.h"
#include "drivers/tb6612fng.h"
#include "app/drive.h"
#include "drivers/i2c.h"
#include "drivers/vl53l0x.h"

#define IO_TEST_BUTTON IO_MOTORS_RIGHT_CC_2 // Alias P1.3 for clarity
#define IO_TEST_LED    IO_MOTORS_RIGHT_CC_2


#define PWM_TIMER_FREQ_HZ2 (SMCLK / TIMER_INPUT_DIVIDER_3)
#define PWM_PERIOD_FREQ_HZ2 (20000)
#define PWM_PERIOD_TICKS2 (PWM_TIMER_FREQ_HZ2 / PWM_PERIOD_FREQ_HZ2)

// Timer counts from 0, so should decrement by 1
#define PWM_CCR02 (PWM_PERIOD_TICKS2 - 1)


SUPPRESS_UNUSED
void isr_test_toggle_led(void)
{
	io_set_out(IO_MOTORS_RIGHT_CC_2, IO_OUT_HIGH);
	
}

// Define the bit mask for P2.5
#define BIT5_MASK 0x20

void verify_p2_muxing(void)
{
    uint8_t sel = P2SEL;
    uint8_t sel2 = P2SEL2;

	uint8_t dir = P2DIR;
TRACE("P2DIR (Hex): 0x%X", dir);

if (dir & BIT5_MASK) {
    TRACE("P2.5 ERROR: Configured as OUTPUT (driving), not INPUT (listening).");
}

    // Print the raw register values (e.g., P2SEL=0x20, P2SEL2=0x00)
    TRACE("P2SEL (Hex): 0x%X, P2SEL2 (Hex): 0x%X", sel, sel2);
    
    // Check if P2.5 is configured correctly for TA1.2 (ALT1 function)
    if ((sel & BIT5_MASK) && !(sel2 & BIT5_MASK))
    {
        TRACE("P2.5 MUX: SUCCESS! TA1.2 function is active.");
    }
    else
    {
        TRACE("P2.5 MUX: FAILURE! Check P2SEL/P2SEL2 configuration.");
    }
}

SUPPRESS_UNUSED
static void setup_p1_interrupt_test(void)
{
    // 1. Ensure the LED pin (P1.0) is configured as a GPIO output (should be done in io_init)
    // No action needed here, just verify its initial state (LOW).
    
    // 2. Configure P1.3 (the "button" pin) for interrupt:
    //    - Trigger on RISING edge
    //    - Register the isr_test_toggle_led function
    io_configure_interrupt(IO_IR_REMOTE, IO_TRIGGER_RISING, isr_test_toggle_led);
    
    // 3. Configure P1.3 as an input with a pull-down resistor (assuming external button pulls HIGH)
    //    If you use a wire/jumper to touch VCC to P1.3:
    //    io_configure(IO_TEST_BUTTON, IO_SELECT_GPIO, IO_RESISTOR_PULLDOWN, IO_DIR_INPUT, IO_OUT_LOW);
    //    *Ensure your io_configure function can handle this, or set the registers manually.*
    
    // 4. Enable the interrupt for P1.3
    io_enable_interrupt(IO_IR_REMOTE);
    
}

SUPPRESS_UNUSED
void ir_remote_init_ta1_2(void)
{

	P1DIR |= BIT0;
P1OUT &= ~BIT0;	
    // --- Phase 1: Force P2.5 to TA1.2 ---
    // 1. Ensure Pin 2.5 is configured as Input
    
    // 2. Configure Pin 2.5 for TA1.2 (ALT1: P2SEL=1, P2SEL2=0)
    P2SEL |= BIT5;
    P2SEL2 &= ~BIT5; 
	P2DIR &= ~BIT5;

    // 3. Optional: Enable Pull-up/down resistor
    P2REN |= BIT5;
    P2OUT |= BIT5; // Pull-up selection (HIGH)

    // --- Phase 2: Timer Configuration ---
    // Stop/Clear Timer
    TA1CTL = TASSEL_2 + ID_3 + TACLR;
     
    //TA1CCR0 = PWM_CCR02;

    // Configure Capture on Both Edges + Interrupt Enable
    TA1CCTL2 = CM_3 | SCS | CAP | CCIE | CCIS_1;

    // Start Timer
    TA1CTL |= MC_2; 
}



SUPPRESS_UNUSED
static void test_ir_ta1(void)
{
    mcu_init();
    trace_init();
    TRACE("--------------------");
    ir_remote_init_ta1();
    TRACE("Starting IR capture");
    
    while (1) {
        BUSY_WAIT_ms(2500);
        
        // Print captured edges
       // ir_debug_print_log();
        
        // Check for commands
        ir_cmd_e cmd = ir_remote_get_cmd_ta1();
        if (cmd != IR_CMD_NONE) {
            TRACE("Command received: %d (0x%02X)", cmd, cmd);
        }
    }
}

SUPPRESS_UNUSED
static void test_motor(void)
{
    mcu_init();
    trace_init();
    ir_remote_init_ta1(); 
    tb6612fng_init();
	tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_FORWARD);
        tb6612fng_set_pwm(TB6612FNG_LEFT , PWM_MAX_SPEED);


        tb6612fng_set_mode(TB6612FNG_RIGHT, TB6612FNG_MODE_FORWARD);
        tb6612fng_set_pwm(TB6612FNG_RIGHT , PWM_MAX_SPEED);

    while (1) {
        TRACE("test motor 2");
	
	ir_cmd_e cmd = ir_remote_get_cmd_ta1();
        if (cmd != IR_CMD_NONE) {
            TRACE("Command received: %d (0x%02X)", cmd, cmd);
        }

	BUSY_WAIT_ms(3000);
    }
}

SUPPRESS_UNUSED
static void test_pwm_timers(void)
{
    mcu_init();
    trace_init();
   ir_remote_init_ta1();
   //pwm_both_timers_init();
  tb6612fng_init();

   pwm_speed_e speeds[7] =  {
    PWM_MAX_SPEED,
    PWM_HALF_PLUS_SPEED,
    PWM_HALF_SPEED,
    PWM_QUARTER_PLUS_SPEED,
    PWM_QUARTER_SPEED,
    PWM_EIGHTH_PLUS_SPEED,
    PWM_EIGHTH_SPEED
}; 

static const char *speed_name[] = {
    "MAX",
    "HALF+",
    "HALF",
    "QUARTER+",
    "QUARTER",
    "EIGTH+",
    "EIGTH"
};
    
   //pwm_both_timers_set_duty_cycle(PWM_LEFT, PWM_MAX_SPEED);
   //pwm_both_timers_set_duty_cycle(PWM_RIGHT, PWM_MAX_SPEED);
uint8_t i = 0;
    while (1) {
	
        TRACE("speed: %s",speed_name[i] );
	//pwm_both_timers_set_duty_cycle(PWM_LEFT, speeds[i]);
       // pwm_both_timers_set_duty_cycle(PWM_RIGHT, speeds[i]);
        tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_FORWARD);
        tb6612fng_set_pwm(TB6612FNG_LEFT , speeds[i]);


        tb6612fng_set_mode(TB6612FNG_RIGHT, TB6612FNG_MODE_FORWARD);
        tb6612fng_set_pwm(TB6612FNG_RIGHT , speeds[i]);
	BUSY_WAIT_ms(5000);
	
	i++;
	if (i >= 7)
	{
		i = 0;
	}
    }

}

SUPPRESS_UNUSED
void test_driver(void)
{
    mcu_init();
    trace_init();
    drive_init();
    ir_remote_init_ta1();

    drive_set(DRIVE_DIR_REVERSE, DRIVE_SPEED_MEDIUM);
    while(1)
    {
	ir_cmd_e cmd = ir_remote_get_cmd_ta1();
        if (cmd != IR_CMD_NONE) {
            TRACE("Command received: %d (0x%02X)", cmd, cmd);

		switch(cmd)
		{
			case IR_CMD_1:
				drive_set(DRIVE_DIR_FORWARD, DRIVE_SPEED_MAX);
				break;
				case IR_CMD_8:
                                drive_set(DRIVE_DIR_ARCTURN_MID_LEFT, DRIVE_SPEED_MEDIUM);
                                break;
				case IR_CMD_2:
                                drive_set(DRIVE_DIR_FORWARD, DRIVE_SPEED_FAST);
                                break;
				case IR_CMD_3:
                                drive_set(DRIVE_DIR_FORWARD, DRIVE_SPEED_MEDIUM);
                                break;
				case IR_CMD_4:
                                drive_set(DRIVE_DIR_FORWARD, DRIVE_SPEED_SLOW);
                                break;
				case IR_CMD_5:
                                drive_set(DRIVE_DIR_REVERSE, DRIVE_SPEED_SLOW);
                                break;
				case IR_CMD_6:
                                drive_set(DRIVE_DIR_ROTATE_LEFT, DRIVE_SPEED_MAX);
                                break;
				case IR_CMD_7:
                                drive_set(DRIVE_DIR_ROTATE_RIGHT, DRIVE_SPEED_MAX);
                                break;
			default:
				break;
		}


        }

    	
    }
}


SUPPRESS_UNUSED
void test_vlx(void)
{
	mcu_init();
        trace_init();

	vl53l0x_result_e result = vl53l0x_init();
	if(result) {
		TRACE("vl init failed");
	}

	while(1)
	{
		uint16_t range = 0;
		result = vl53l0x_read_range_single(VL53L0X_IDX_FRONT, &range);
		if (result)
		{
			TRACE("meassurement failed: %u", result);
		}else{
			if(range != VL53L0X_OUT_OF_RANGE) {
				TRACE("Range %u", range);
			}else{
				TRACE("Range %u mm", range);
			}
		}

		BUSY_WAIT_ms(1000);
	}
}

SUPPRESS_UNUSED
void test_i2c(void)
{
	mcu_init();
	trace_init();
	i2c_init();
	io_set_out(IO_XSHUT_FRONT,IO_OUT_HIGH );
	i2c_set_slave_address(0x29);
	BUSY_WAIT_ms(100); 
	while(1)
	{
		BUSY_WAIT_ms(100);
		uint8_t vl53l0x_id = 0;
		i2c_result_e result = i2c_read_addr8_data8(0xC0, &vl53l0x_id);
		if(result) {
			TRACE("I2C Error %d", result);
		}
		else
		{
			if(vl53l0x_id == 0xEE)
			{
				TRACE("READ expected id");
			}else {
				TRACE("Reas unexpected id 0x%X", vl53l0x_id); 
			}
			
		}
	}
}

void test_multiple_vlx(void)
{
	mcu_init();
        trace_init();
       // i2c_init();

	vl53l0x_result_e result = vl53l0x_init();
	if(result)
	{
		TRACE("vlx init failed: %u" , result);
	}

	while(1) {
		vl53l0x_ranges_t ranges = {0,0,0,0,0};
		bool fresh_values = false;
		result = vl53l0x_read_range_multiple(ranges, &fresh_values);
		if (result) {
			TRACE("Range meassure failed, result: %u", result);
		}
		TRACE("meassurement f %u fl %u fr %u", ranges[VL53L0X_IDX_FRONT], ranges[VL53L0X_IDX_FRONT_LEFT], ranges[VL53L0X_IDX_FRONT_RIGHT]);

		BUSY_WAIT_ms(1000);
	}

}

int main(void)
{
   //test_ir_ta1();
   //test_pwm_timers();
   //test_motor();
   //test_driver();
  // test_vlx();
test_multiple_vlx();
   //test_i2c();
   return 0;
}
