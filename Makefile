# Directories
TOOLS_DIR = ${TOOLS_PATH}
MSPGCC_ROOT_DIR = $(TOOLS_DIR)/msp430-gcc
MSPGCC_BIN_DIR = $(MSPGCC_ROOT_DIR)/bin
MSPGCC_SUPPORT_DIR = $(TOOLS_DIR)/msp430-gcc/msp430-gcc-support-files
STANDART_C_LIB = $(TOOLS_DIR)/msp430-gcc/lib/gcc/msp430-elf/9.3.1/include
INCLUDE_DIRS = \
    $(MSPGCC_SUPPORT_DIR)/include \
    $(STANDART_C_LIB) \
    src \
    src/drivers \
    src/common \ 
LIB_DIRS = $(MSPGCC_SUPPORT_DIR)/include

TI_CCS_DIR = $(TOOLS_DIR)/ti/codeComposer/ccs2040/ccs
DEBUG_BIN_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/bin
DEBUG_DRIVERS_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/drivers
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Toolchain
CC = $(MSPGCC_BIN_DIR)/msp430-elf-gcc
RM = rm
FORMAT = clang-format
DEBUG = LD_LIBRARY_PATH=$(DEBUG_DRIVERS_DIR) $(DEBUG_BIN_DIR)/mspdebug
CPPCHECK = cppcheck

# Files
TARGET = $(BIN_DIR)/robo_sumo
SRC_DIR = src
SOURCES =$(SRC_DIR)/main.c $(SRC_DIR)/drivers/io.c

OBJECT_NAMES = $(SOURCES:.c=.o)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

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
.PHONY: all clean flash cppcheck format

all: $(TARGET)

clean:
	$(RM) -rf $(BUILD_DIR)

flash: $(TARGET)
	$(DEBUG) tilib "prog $(TARGET)"

cppcheck:
	@$(CPPCHECK) --quiet --enable=all --error-exitcode=1 \
	--inline-suppr \
	--suppress=toomanyconfigs \
	--suppress=checkersReport \
        $(addprefix -I, $(INCLUDE_DIRS)) \
	$(SOURCES) \
  	-i external/printf
format: 
	@find . -name "*.c" -o -name "*.h" | xargs $(FORMAT) -i	
