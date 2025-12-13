#include "ir_remote_ta1.h"
#include "common/ring_buffer.h"
#include "common/trace.h"
#include "common/defines.h"
#include <msp430.h>
#include <stdint.h>
#include "drivers/io.h"


#define PWM_TIMER_FREQ_HZ2 (SMCLK / TIMER_INPUT_DIVIDER_3)
#define PWM_PERIOD_FREQ_HZ2 (20000)
#define PWM_PERIOD_TICKS2 (PWM_TIMER_FREQ_HZ2 / PWM_PERIOD_FREQ_HZ2)

// Timer counts from 0, so should decrement by 1
#define PWM_CCR02 (PWM_PERIOD_TICKS2 - 1)




#define TIMER_CLK_FREQ_HZ (SMCLK / TIMER_INPUT_DIVIDER_3) // 2 MHz
#define TICK_PER_US (TIMER_CLK_FREQ_HZ / 1000000u)        // 2 Ticks/uS

// NEC Timing Definitions (in Ticks)
#define NEC_LEADER_MARK_TICKS_NOMINAL (9000 * TICK_PER_US) // 18000
#define NEC_BIT_1_SPACE_TICKS_NOMINAL (1680 * TICK_PER_US) // 3360
#define NEC_BIT_0_SPACE_TICKS_NOMINAL (560 * TICK_PER_US)  // 1120

// With falling edge capture, we measure MARK + SPACE combined
#define NEC_LEADER_TOTAL_TICKS ((9000 + 4500) * TICK_PER_US) // 27000 ticks = 13.5ms
#define NEC_BIT_1_TOTAL_TICKS ((560 + 1680) * TICK_PER_US)   // 4480 ticks = 2.24ms
#define NEC_BIT_0_TOTAL_TICKS ((560 + 560) * TICK_PER_US)    // 2240 ticks = 1.12ms


// Tolerances (e.g., +/- 25%)
#define NEC_TOLERANCE_TICKS(nominal) ((nominal) * 25 / 100)

static uint16_t last_capture = 0; // The timestamp of the previous edge
static uint8_t bit_count = 0;    // Tracks the 32 bits of the NEC protocol
static uint8_t current_byte = 0;
static uint8_t byte_bit_count = 0;
#define IR_CMD_BUFFER_ELEM_CNT (10u)
STATIC_RING_BUFFER(ir_cmd_buffer, IR_CMD_BUFFER_ELEM_CNT, uint8_t);

#define DEBUG_BUFFER_SIZE 40
static volatile struct {
    uint16_t duration[DEBUG_BUFFER_SIZE];
    uint8_t index;
    bool overflow;
} debug_log = {0};


static union {
    struct
    {
        // cppcheck-suppress unusedStructMember
        uint8_t cmd_inverted;
        uint8_t cmd;
        // cppcheck-suppress unusedStructMember
        uint8_t addr_inverted;
        // cppcheck-suppress unusedStructMember
        uint8_t addr;
    } decoded;
    uint32_t raw;
} ir_message;



static void ir_parse(uint16_t dur)
{
    if (dur > NEC_LEADER_MARK_TICKS_NOMINAL - NEC_TOLERANCE_TICKS(NEC_LEADER_MARK_TICKS_NOMINAL)) {
        // Detected Leader Mark (start of new message)
        ir_message.raw = 0;
        bit_count = 0;
        current_byte = 0;
        byte_bit_count = 0;
    }
    else if (bit_count < 32) {
        // NEC sends LSB first within each byte
        current_byte >>= 1;
        
        // If duration is a '1' space (long pause)
        if (dur > NEC_BIT_1_SPACE_TICKS_NOMINAL - NEC_TOLERANCE_TICKS(NEC_BIT_1_SPACE_TICKS_NOMINAL)) {
            current_byte |= 0x80;  // Set the MSB
            bit_count++;
            byte_bit_count++;
        }
        // If duration is a '0' space (short pause)
        else if (dur > NEC_BIT_0_SPACE_TICKS_NOMINAL - NEC_TOLERANCE_TICKS(NEC_BIT_0_SPACE_TICKS_NOMINAL)) {
            bit_count++;
            byte_bit_count++;
        }
        
        // Every 8 bits, we've completed a byte
        if (byte_bit_count == 8) {
            switch (bit_count) {
                case 8:
                    ir_message.decoded.addr = current_byte;
                    break;
                case 16:
                    ir_message.decoded.addr_inverted = current_byte;
                    break;
                case 24:
                    ir_message.decoded.cmd = current_byte;
                    break;
                case 32:
                    ir_message.decoded.cmd_inverted = current_byte;
                    P1OUT ^= BIT0;
                    
                    // Validate NEC checksum
                    if ((ir_message.decoded.cmd ^ ir_message.decoded.cmd_inverted) == 0xFF) {
                        ring_buffer_put(&ir_cmd_buffer, &ir_message.decoded.cmd);
                    }
                    
                    bit_count = 0;
                    break;
            }
            
            current_byte = 0;
            byte_bit_count = 0;
        }
    }
}


// ir_remote.c - Revised ir_remote_init
void ir_remote_init_ta1(void)
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
    TA1CCTL2 = CM_2 | SCS | CAP | CCIE | CCIS_1;

    // Start Timer
    TA1CTL |= MC_2;
}


void ir_debug_print_log(void)
{
    __disable_interrupt();
    uint8_t count = debug_log.index;
    bool overflow = debug_log.overflow;
    __enable_interrupt();
    
    if (count == 0 && !overflow) {
        return;
    }
    
    TRACE("=== IR Debug: %d edges captured ===", count);
    
    for (uint8_t i = 0; i < count; i++) {
        uint16_t dur_ticks = debug_log.duration[i];
        uint16_t dur_us = dur_ticks / TICK_PER_US;
        
        // Classify the duration
        const char* type = "UNKNOWN";
        if (dur_ticks > NEC_LEADER_MARK_TICKS_NOMINAL - NEC_TOLERANCE_TICKS(NEC_LEADER_MARK_TICKS_NOMINAL)) {
            type = "LEADER";
        } else if (dur_ticks > NEC_BIT_1_SPACE_TICKS_NOMINAL - NEC_TOLERANCE_TICKS(NEC_BIT_1_SPACE_TICKS_NOMINAL)) {
            type = "BIT_1";
        } else if (dur_ticks > NEC_BIT_0_SPACE_TICKS_NOMINAL - NEC_TOLERANCE_TICKS(NEC_BIT_0_SPACE_TICKS_NOMINAL)) {
            type = "BIT_0";
        }
        
        TRACE("%2d: %5u ticks (%4u us) - %s", i, dur_ticks, dur_us, type);
    }
    
    if (overflow) {
        TRACE("WARNING: Debug buffer overflow!");
    }
    
    // Reset debug log
    __disable_interrupt();
    debug_log.index = 0;
    debug_log.overflow = false;
    __enable_interrupt();
}


ir_cmd_e ir_remote_get_cmd_ta1(void)
{
    __disable_interrupt();
    ir_cmd_e cmd = IR_CMD_NONE;
    if (!ring_buffer_empty(&ir_cmd_buffer)) {
        ring_buffer_get(&ir_cmd_buffer, &cmd);
    }
    __enable_interrupt();
    return cmd;
}



//void __attribute__((interrupt(8))) isr_timer1_a1 (void)
//{

//void __attribute__((interrupt(TIMER1_A1_VECTOR)))
// Use the TA1 A1 Vector (Timer A1 CCR1/CCR2/Overflow)
void __attribute__((interrupt(TIMER1_A1_VECTOR))) isr_timer1_a1 (void){

       // The TA1IV register automatically selects the highest priority pending interrupt
    // and must be read to clear the flag.
    switch (__even_in_range(TA1IV, 10)) {
        case TA1IV_NONE:   break;    // No interrupt
        case TA1IV_TACCR1:          // TA1.1 (Right Motor PWM Duty Cycle match)
            // No action usually required for simple PWM
            break;
            
        case TA1IV_TACCR2:
        {
            // Read capture value (16-bit)
            uint16_t current_capture = TA1CCR2;
            
            // Calculate duration (handles wraparound automatically)
            uint16_t duration = current_capture - last_capture;
            
            // Store for debugging
            if (debug_log.index < DEBUG_BUFFER_SIZE) {
                debug_log.duration[debug_log.index++] = duration;
            } else {
                debug_log.overflow = true;
            }
            
            // Update last capture
            last_capture = current_capture;
            
            // Parse the IR signal
            ir_parse(duration);
        }
        break;

        case TA1IV_TAIFG:  // TA1 Timer Overflow
            // You may handle overflow here if needed, but the overflow logic above handles it implicitly.
            break;
    }
}


