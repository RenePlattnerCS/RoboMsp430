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
    bool uses_continuous_mode; //TA1 need it for IR capture
	
};

static struct pwm_channel_cfg pwm_cfgs[] = {
    [PWM_LEFT] = { .enabled = false, .cctl = &TA0CCTL1, .ccr = &TA0CCR1, .ctl = &TA0CTL, .uses_continuous_mode = false },
    [PWM_RIGHT] = { .enabled = false, .cctl = &TA1CCTL1, .ccr = &TA1CCR1, .ctl = &TA1CTL, .uses_continuous_mode = true },
};

static uint16_t ta1_ccr0_base = 0;
//----------------------------
//structs to help set the period
struct pwm_period
{
    uint8_t a0_period;
    uint8_t a1_period;
};



static struct pwm_period pwm_periods[] = {
	[PWM_MAX_SPEED] = { .a0_period = 1u, .a1_period = 50u},
	[PWM_HALF_SPEED] = { .a0_period = 2u, .a1_period = 25u},
	[PWM_QUARTER_SPEED] = { .a0_period = 4u, .a1_period = 12u},
	[PWM_STOP_SPEED] = { .a0_period = 0u, .a1_period = 0u}
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
    volatile unsigned int *ctl_reg = pwm_cfgs[pwm].ctl;
    
    if (pwm_cfgs[pwm].enabled != enable) {
        
        if (pwm_cfgs[pwm].uses_continuous_mode) {
            // TA1 (Right motor) - Uses continuous mode for IR compatibility
            if (enable) {
                // Set Reset/Set mode (OUTMOD_7)
                //*pwm_cfgs[pwm].cctl = OUTMOD_7;
		TA1CCTL1 = OUTMOD_2;
                
                // Enable CCR0 interrupt for period management
                TA1CCTL0 = CCIE;
                
                // Timer should already be in MC_2 (continuous) from IR init
                // Just ensure it's running
                *ctl_reg = (*ctl_reg & ~TIMER_MC_MASK) | TACLR | MC_2;
            } else {
                // Disable output
                *pwm_cfgs[pwm].cctl = OUTMOD_0;
                
                // Disable CCR0 interrupt
                TA1CCTL0 &= ~CCIE;
                
                // Keep timer running in continuous mode for IR capture
                // Don't stop the timer!
            }
        } else {
            // TA0 (Left motor) - Standard up mode PWM
            *pwm_cfgs[pwm].cctl = enable ? OUTMOD_7 : OUTMOD_0;
            
            if (enable) {
                *ctl_reg = (*ctl_reg & ~TIMER_MC_MASK) | TACLR | MC_1;
            } else {
                if (ctl_reg == &TA0CTL) {
                    if (!timer_has_active_channels(ctl_reg)) {
                        *ctl_reg = (*ctl_reg & ~TIMER_MC_MASK) | MC_0;
                    }
                }
            }
        }
        
        pwm_cfgs[pwm].enabled = enable;
    }
}

//-----------------------------

/*
static inline uint8_t pwm_scale_duty_cycle(uint8_t duty_cycle_percent)
{
    * Battery is at ~8 V when fully charged and motors are 6 V max,
     * so scale down the duty cycle by 25% to be within specs. This
     * should never return 0. *
    return duty_cycle_percent == 1 ? duty_cycle_percent : duty_cycle_percent * 3 / 4;
}
*/


void pwm_both_timers_set_duty_cycle(pwm_e pwm, pwm_speed_e speed)
{
    const bool enable = (speed != PWM_STOP_SPEED);
    if (enable) {
        *pwm_cfgs[pwm].ccr = pwm_periods[speed].a1_period;
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
     // MC_2:contious mode
    TA1CTL = TASSEL_2 + ID_3 + TACLR + MC_2; 
    // Set period
    TA1CCR0 = PWM_CCR0;
    ta1_ccr0_base = 0;  // Track the base value
}


void pwm_both_timers_init(void)
{
    timer_a0_init();
    timer_a1_init();
}


//----------------------------
// ISR for TA1 CCR0 - Manages PWM period in continuous mode
//----------------------------
INTERRUPT_FUNCTION(TIMER1_A0_VECTOR) isr_reset_continous_timer(void)
{
    if (pwm_cfgs[PWM_RIGHT].enabled) {
        // 1. RESET output at start of period
        TA1CCTL1 |= OUT;
    }

    // 2. Advance software period base
    ta1_ccr0_base += PWM_CCR0 + 1;
    TA1CCR0 = ta1_ccr0_base;

    // 3. Schedule SET event strictly in the future
    if (pwm_cfgs[PWM_RIGHT].enabled) {
	uint16_t duty = *pwm_cfgs[PWM_RIGHT].ccr;

        // --- SAFETY CLAMP (THIS IS THE PART YOU ASKED ABOUT) ---
        if (duty == 0) {
            duty = 1;
        }
        if (duty >= PWM_CCR0) {
            duty = PWM_CCR0 - 1;
        }
        TA1CCR1 = ta1_ccr0_base + duty;
    }
}

