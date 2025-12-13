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

struct pwm_channel_cfg
{
    bool enabled;
    volatile unsigned int *const cctl;
    volatile unsigned int *const ccr;
    volatile unsigned int *const ctl;

	
};

static struct pwm_channel_cfg pwm_cfgs[] = {
    [PWM_LEFT] = { .enabled = false, .cctl = &TA0CCTL1, .ccr = &TA0CCR1, .ctl = &TA0CTL },
    [PWM_RIGHT] = { .enabled = false, .cctl = &TA1CCTL1, .ccr = &TA1CCR1, .ctl = &TA1CTL },
};


//----------------------------
// Helper function to check if any other channel using the given Control Register is active
static bool timer_has_active_channels(volatile unsigned int *timer_ctl_addr)
{
    for (uint8_t ch = 0; ch < ARRAY_SIZE(pwm_cfgs); ch++) {
        // Check if the channel is enabled AND if its control register pointer matches the one we are checking
        if (pwm_cfgs[ch].enabled && pwm_cfgs[ch].ctl == timer_ctl_addr) {
            return true;
        }
    }
    return false;
}


static void pwm_channel_enable(pwm_e pwm, bool enable)
{
    volatile unsigned int *ctl_reg = pwm_cfgs[pwm].ctl; // Get TA0CTL or TA1CTL pointer
    
    if (pwm_cfgs[pwm].enabled != enable) {
        
        // 1. Set the channel output mode (OUTMOD_7 or OUTMOD_0)
        *pwm_cfgs[pwm].cctl = enable ? OUTMOD_7 : OUTMOD_0;
        pwm_cfgs[pwm].enabled = enable;

        // 2. Control the specific parent Timer (TA0 or TA1)
        if (enable) {
            // If enabling, ensure the timer starts (MC_1)
            // *ctl_reg writes to TA0CTL or TA1CTL
            // The TASSEL and ID bits should have been set in the _init function.
            *ctl_reg = (*ctl_reg & ~TIMER_MC_MASK) | TACLR | MC_1;
            
        } else {
            // If disabling, check if all other channels on this timer are also disabled
            // If they are all disabled, stop the timer (MC_0)
            if (!timer_has_active_channels(ctl_reg)) {
                *ctl_reg = (*ctl_reg & ~TIMER_MC_MASK) | MC_0;
            }
        }
    }
}
//-----------------------------


static inline uint8_t pwm_scale_duty_cycle(uint8_t duty_cycle_percent)
{
    /* Battery is at ~8 V when fully charged and motors are 6 V max,
     * so scale down the duty cycle by 25% to be within specs. This
     * should never return 0. */
    return duty_cycle_percent == 1 ? duty_cycle_percent : duty_cycle_percent * 3 / 4;
}



void pwm_both_timers_set_duty_cycle(pwm_e pwm, uint8_t duty_cycle_percent)
{
    if(duty_cycle_percent > 100)
	    return;

    const bool enable = duty_cycle_percent > 0;
    if (enable) {
        *pwm_cfgs[pwm].ccr = pwm_scale_duty_cycle(duty_cycle_percent);
    }
    pwm_channel_enable(pwm, enable);
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
    /* TASSEL_2: Clock source SMCLK
    * ID_3: Input divider /8
    * MC_0: Stopped */
    TA1CTL = TASSEL_2 + ID_3 + MC_0;
    // Set period
    TA1CCR0 = PWM_CCR0;

}


void pwm_both_timers_init(void)
{
    timer_a0_init();
    timer_a1_init();
}
