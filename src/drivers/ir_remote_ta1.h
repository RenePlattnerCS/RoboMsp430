#ifndef IR_REMOTE_TA1_H
#define IR_REMOTE_TA1_H

// A driver that decodes the commands sent to the IR receiver (NEC protocol)

typedef enum {
    IR_CMD_0 = 0x19,
    IR_CMD_1 = 0x45,
    IR_CMD_2 = 0x46,
    IR_CMD_3 = 0x47,
    IR_CMD_4 = 0x44,
    IR_CMD_5 = 0x40,
    IR_CMD_6 = 0x43,
    IR_CMD_7 = 0x07,
    IR_CMD_8 = 0x15,
    IR_CMD_9 = 0x09,
    IR_CMD_STAR = 0x16,
    IR_CMD_HASH = 0x0D,
    IR_CMD_UP = 0x18,
    IR_CMD_DOWN = 0x52,
    IR_CMD_LEFT = 0x08,
    IR_CMD_RIGHT = 0x5A,
    IR_CMD_OK = 0x1C,
    IR_CMD_NONE = 0xFF
} ir_cmd_e;

void ir_remote_init_ta1(void);
void ir_debug_print_log(void);
ir_cmd_e ir_remote_get_cmd_ta1(void);
#endif // IR_REMOTE_TA1_H
