#include "drivers/pwm_both_timers.h"
#include "drivers/io.h"
#include "common/defines.h"
#include <msp430.h>
#include <stdbool.h>

/* MSP430G2553 has no dedicated PWM, so use timer A0 to emulate
 * hardware PWM. Each timer has three capture/compare channels
 * (CC) and first channel must be sacrificed for setting the
 * base period (TA0CCR). The two other channels are used for
 * one PWM output each and duty cycle is set by setting the
 * timer value (TA0CCR1 and TA0CCR2). The CC outputs are muxed
 * to corresponding IO pins (see io.h).
 *
 * Example (one period):
 * -----------------_____________ // CC output
 * <----TA0CCRx---->              // Duty cycle
 * <----------TA0CCR0-----------> // Base period
 *
 * Set the base frequency to 20000 Hz because with SMCLK of 16 MHz it
 * gives a base period of 100 ticks, which means the duty cycle
 * percent corresponds to the TA0CCRx directly without any conversion.
 * 20 kHz also gives stable motor behaviour. */
#define PWM_TIMER_FREQ_HZ (SMCLK / TIMER_INPUT_DIVIDER_3)
#define PWM_PERIOD_FREQ_HZ (20000)
#define PWM_PERIOD_TICKS (PWM_TIMER_FREQ_HZ / PWM_PERIOD_FREQ_HZ)

// Timer counts from 0, so should decrement by 1
#define PWM_CCR0 (PWM_PERIOD_TICKS - 1)

#define PWM_PERIOD_TICKS2   (PWM_PERIOD_TICKS * 8u)

static pwm_speed_e current_speed_right;
bool is_chanel_enabled[2] = { false, false}; 

//static struct pwm_channel_cfg pwm_cfgs[] = {
//    [PWM_LEFT] = { .enabled = false, .cctl = &TA0CCTL1, .ccr = &TA0CCR1, .ctl = &TA0CTL, .uses_continuous_mode = false },

    
//	[PWM_RIGHT] = { .enabled = false, .cctl = &TA1CCTL1, .ccr = &TA1CCR1, .ctl = &TA1CTL, .uses_continuous_mode = true },
//};

static uint16_t ta1_ccr0_base = 0;
//----------------------------
//structs to help set the period
struct pwm_period
{
    uint16_t a1_period[2];
    uint8_t a0_period;
};



static struct pwm_period pwm_periods[] = {
	[PWM_MAX_SPEED] = { .a1_period = {PWM_PERIOD_TICKS2 * 2u, PWM_PERIOD_TICKS2 *1u}, .a0_period = 67u},
	[PWM_HALF_PLUS_SPEED] = { .a1_period = {PWM_PERIOD_TICKS2 + (PWM_PERIOD_TICKS2 /2), PWM_PERIOD_TICKS2 *1u}, .a0_period = 60u},
	[PWM_HALF_SPEED] = { .a1_period = {PWM_PERIOD_TICKS2 * 1u, PWM_PERIOD_TICKS2 * 1u}, .a0_period = 50u},
	[PWM_QUARTER_PLUS_SPEED] = { .a1_period = {(PWM_PERIOD_TICKS2 / 2) + (PWM_PERIOD_TICKS2 / 4), PWM_PERIOD_TICKS2 *1u}, .a0_period = 43u},
	[PWM_QUARTER_SPEED] = { .a1_period = {PWM_PERIOD_TICKS2 / 2, PWM_PERIOD_TICKS2}, .a0_period = 33u},
	[PWM_EIGHTH_PLUS_SPEED] = { .a1_period = {(PWM_PERIOD_TICKS2 / 4) + (PWM_PERIOD_TICKS2 / 8), PWM_PERIOD_TICKS2 *1u}, .a0_period = 27u},
	[PWM_EIGHTH_SPEED] = { .a1_period ={PWM_PERIOD_TICKS2 / 4, PWM_PERIOD_TICKS2}, .a0_period = 20u},
	[PWM_STOP_SPEED] = { .a1_period ={1u, PWM_PERIOD_TICKS2}, .a0_period = 0u}

};

//----------------------------

static void pwm_channel_enable(pwm_e pwm, bool enable)
{
    
    if (is_chanel_enabled[pwm] != enable) {
        
        if (pwm == PWM_RIGHT) {
            // TA1 (Right motor) - Uses continuous mode for IR compatibility
            if (enable) {
                
                // Enable CCR0 interrupt for period management
                TA1CCTL0 = CCIE;
                
            } else {
                
                // Disable CCR0 interrupt
                TA1CCTL0 &= ~CCIE;
                
            }
        } else {
            // TA0 (Left motor) - Standard up mode PWM
            TA0CCTL1 = enable ? OUTMOD_7 : OUTMOD_0;
            
            if (enable) {
                TA0CTL = (TA0CTL & ~TIMER_MC_MASK) | TACLR | MC_1;
            } else {
                TA0CTL = (TA0CTL & ~TIMER_MC_MASK) | MC_0;
                }
        }
        
        is_chanel_enabled[pwm] = enable;
    }
}

//-----------------------------

void pwm_both_timers_set_duty_cycle(pwm_e pwm, pwm_speed_e speed)
{
    const bool enable = (speed != PWM_STOP_SPEED);
    if (enable) {
	if(pwm == PWM_LEFT)
	{
		TA0CCR1 = pwm_periods[speed].a0_period;
	}
	else
	{
		__disable_interrupt();
   		 current_speed_right = speed;
	    	__enable_interrupt();

	}
	pwm_channel_enable(pwm, enable);
}
}


static void timer_a0_init(void)
{
    /* TASSEL_2: Clock source SMCLK
    * ID_3: Input divider /8
    * MC_0: Stopped */
    TA0CTL = TASSEL_2 + ID_3 + MC_0;
    // Set period
    TA0CCR0 = PWM_CCR0;

}

static void timer_a1_init(void)
{
     // MC_2:contious mode
    TA1CTL = TASSEL_2 + ID_3 + TACLR + MC_2; 
    // Set period
    ta1_ccr0_base = TA1R + PWM_PERIOD_TICKS;  // Track the base value
    TA1CCR0 = ta1_ccr0_base;
	
    //TA1CCTL0 = CCIE;
}


void pwm_both_timers_init(void)
{
    timer_a0_init();
    timer_a1_init();

    current_speed_right = PWM_STOP_SPEED;
}


//----------------------------
// ISR for TA1 CCR0 - Manages PWM period in continuous mode
//----------------------------
static bool toggle_bit = false;

INTERRUPT_FUNCTION(TIMER1_A0_VECTOR) isr_reset_continous_timer(void)
{
    if (is_chanel_enabled[PWM_RIGHT]) {
	if(!toggle_bit)
	{
		io_set_out(IO_PWM_MOTORS_RIGHT, IO_OUT_HIGH);
		ta1_ccr0_base += pwm_periods[current_speed_right].a1_period[0];

	}
	else
	{
		io_set_out(IO_PWM_MOTORS_RIGHT, IO_OUT_LOW);
		ta1_ccr0_base += pwm_periods[current_speed_right].a1_period[1];

	}
    	toggle_bit = !toggle_bit;
    
    }

    // 2. Advance software period base
    TA1CCR0 = ta1_ccr0_base;
    
    }


