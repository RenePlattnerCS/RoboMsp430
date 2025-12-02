# Directories
MSPGCC_ROOT_DIR = /home/rene/tools/arm-gcc/msp430-gcc-9.3.1.11_linux64
MSPGCC_BIN_DIR = $(MSPGCC_ROOT_DIR)/bin
MSPGCC_INCLUDE_DIR = /home/rene/tools/ti/codeComposer/ccs2040/ccs/ccs_base/msp430/include_gcc
INCLUDE_DIRS = $(MSPGCC_INCLUDE_DIR)
LIB_DIRS = /home/rene/tools/ti/codeComposer/ccs2040/ccs/ccs_base/msp430/include_gcc/
TI_CCS_DIR = /home/rene/tools/ti/codeComposer/ccs2040/ccs
DEBUG_BIN_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/bin
DEBUG_DRIVERS_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/drivers
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Toolchain
CC = $(MSPGCC_BIN_DIR)/msp430-elf-gcc
RM = rm
DEBUG = LD_LIBRARY_PATH=$(DEBUG_DRIVERS_DIR) $(DEBUG_BIN_DIR)/mspdebug

# Files
TARGET = $(BIN_DIR)/robo_sumo
SRC_DIR = src
SOURCES = main.c
SOURCE_PATHS = $(addprefix $(SRC_DIR)/,$(SOURCES))
OBJECT_NAMES = $(SOURCES:.c=.o)
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(OBJECT_NAMES))

# Flags
MCU = msp430g2553
WFLAGS = -Wall -Wextra -Werror -Wshadow
CFLAGS = -mmcu=$(MCU) $(WFLAGS) $(addprefix -I, $(INCLUDE_DIRS)) -Og -g
LDFLAGS = -mmcu=$(MCU) $(addprefix -L,$(LIB_DIRS))

# Linking
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@

# Compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^


# Phonies
.PHONY: all clean flash

all: $(TARGET)

clean:
	$(RM) -rf $(BUILD_DIR)

flash: $(TARGET)
	$(DEBUG) tilib "prog $(TARGET)"
