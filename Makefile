###############################################################################
# Makefile for STM32L072 Embedded Firmware Project
###############################################################################

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

# Project name
PROJECT = firmware

# Build directory
BUILD_DIR = build

# Source directories
SRC_DIR = src
BOARD_DIR = board
HAL_DIR = hal
DRIVERS_DIR = drivers
APP_DIR = app
STARTUP_DIR = startup

# Include directories (inc/ first so our stdint.h is found before system one)
INC_DIRS = -Iinc \
           -I$(SRC_DIR) \
           -I$(BOARD_DIR) \
           -I$(HAL_DIR) \
           -I$(DRIVERS_DIR)/pressure_sensor \
           -I$(DRIVERS_DIR)/i2c_slave \
           -I$(DRIVERS_DIR)/dac \
           -I$(APP_DIR) \
           -Ihal/stm32cube/Drivers/CMSIS/Device/ST/STM32L0xx/Include \
           -Ihal/stm32cube/Drivers/CMSIS/Core/Include \
           -Ihal/stm32cube/Drivers/CMSIS/Include \
           -Ihal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Inc \
           -Ihal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Inc/Legacy

# CMSIS system file
CMSIS_SRCS = $(HAL_DIR)/stm32cube/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c

# LL (Low Layer) source files - not needed, using HAL only
# LL_SRCS = $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_gpio.c \
#           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_rcc.c
LL_SRCS =

# HAL source files (required HAL modules)
HAL_SRCS = $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim_ex.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dac.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dac_ex.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c \
           $(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c

# Application source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/memset.c \
       $(SRC_DIR)/libc_init.c \
       $(BOARD_DIR)/board_init.c \
       $(HAL_DIR)/hal_config.c \
       $(DRIVERS_DIR)/pressure_sensor/ms58.c \
       $(DRIVERS_DIR)/pressure_sensor/ms58_hal_wrapper.c \
       $(DRIVERS_DIR)/i2c_slave/i2c_slave.c \
       $(DRIVERS_DIR)/dac/dac.c \
       $(APP_DIR)/app.c \
       $(APP_DIR)/sensor_sampling.c \
       $(CMSIS_SRCS) \
       $(LL_SRCS) \
       $(HAL_SRCS)

# Startup file
STARTUP_SRC = $(STARTUP_DIR)/startup_stm32l072xx.s

# Object files
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
STARTUP_OBJ = $(if $(wildcard $(STARTUP_SRC)),$(BUILD_DIR)/$(STARTUP_SRC:.s=.o),)

# Linker script
LINKER_SCRIPT = linker.ld

# Compiler flags
CFLAGS = -mcpu=cortex-m0plus \
         -mthumb \
         -mfloat-abi=soft \
         -Wall \
         -Wextra \
         -Wno-unused-parameter \
         -g \
         -O2 \
         -ffunction-sections \
         -fdata-sections \
         -DSTM32L072xx \
         -DUSE_HAL_DRIVER \
         $(INC_DIRS)

# Assembler flags
ASFLAGS = -mcpu=cortex-m0plus \
          -mthumb \
          -g

# Linker flags
# Note: nano.specs and nosys.specs may not be available in all toolchain installations
# If they're missing, the linker will use default specs
LDFLAGS = -mcpu=cortex-m0plus \
          -mthumb \
          -mfloat-abi=soft \
          -T$(LINKER_SCRIPT) \
          -Wl,--gc-sections \
          -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
          -Wl,--print-memory-usage \
          -nostdlib \
          -lgcc

# HAL library path (adjust if your STM32Cube location differs)
HAL_LIB_DIR = hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src

# Default target
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT).hex

# Create build directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)
	@mkdir -p $(BUILD_DIR)/$(BOARD_DIR)
	@mkdir -p $(BUILD_DIR)/$(HAL_DIR)
	@mkdir -p $(BUILD_DIR)/$(HAL_DIR)/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src
	@mkdir -p $(BUILD_DIR)/$(DRIVERS_DIR)/pressure_sensor
	@mkdir -p $(BUILD_DIR)/$(DRIVERS_DIR)/i2c_slave
	@mkdir -p $(BUILD_DIR)/$(DRIVERS_DIR)/dac
	@mkdir -p $(BUILD_DIR)/$(APP_DIR)
	@mkdir -p $(BUILD_DIR)/$(STARTUP_DIR)

# Compile C source files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Assemble startup file
$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@echo "AS  $<"
	@$(AS) $(ASFLAGS) $< -o $@

# Link
# Build startup object if source exists
ifneq ($(wildcard $(STARTUP_SRC)),)
$(BUILD_DIR)/$(PROJECT).elf: $(OBJS) $(STARTUP_OBJ) $(LINKER_SCRIPT) | $(BUILD_DIR)
else
$(BUILD_DIR)/$(PROJECT).elf: $(OBJS) $(LINKER_SCRIPT) | $(BUILD_DIR)
endif
	@echo "LD  $@"
	@if [ -n "$(STARTUP_OBJ)" ] && [ -f "$(STARTUP_OBJ)" ]; then \
		$(CC) $(OBJS) $(STARTUP_OBJ) $(LDFLAGS) -o $@; \
	else \
		echo "WARNING: Startup file not found or empty, linking without it"; \
		echo "NOTE: You'll need a startup file for a complete build"; \
		$(CC) $(OBJS) $(LDFLAGS) -o $@; \
	fi
	@echo "Size:"
	@$(SIZE) $@

# Generate binary
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@

# Generate hex
$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O ihex $< $@

# Clean
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)

# Flash using st-flash (requires stlink tools)
flash: $(BUILD_DIR)/$(PROJECT).bin
	@echo "Flashing $(BUILD_DIR)/$(PROJECT).bin to MCU..."
	@st-flash write $(BUILD_DIR)/$(PROJECT).bin 0x8000000

# Display help
help:
	@echo "Available targets:"
	@echo "  all     - Build firmware (default)"
	@echo "  clean   - Remove build files"
	@echo "  flash   - Flash firmware to MCU (requires st-flash)"
	@echo "  help    - Show this help message"

.PHONY: all clean flash help

