# Embedded_firmware_project_A
      This short assignment demonstrates core embedded skills: professional project setup, portable software structure, interrupt-driven sensor sampling, I2C communication, and DAC control. 


## Overview
      This project implements a bare-metal firmware for the STM32L0 microcontroller, focusing on interrupt-driven pressure sensor sampling, I2C slave communication, and internal DAC control. The system is designed to be portable, with separation between platform-specific HAL/board code, reusable drivers, and application logic. This allows easy retargeting to another MCU with minimal changes.

## Firmware:
    * Samples a pressure sensor (MS583730BA01-50) at approximately 2 ms intervals using a hardware timer interrupt.
    * Implements an I2C slave on a separate bus to receive a 32-bit value from an external master and reply with a 32-bit value on request.
    * Controls the STM32L0's internal DAC(s) to output two voltages, with APIs that accept values in volts (engineering units) and handle conversion/clipping to DAC codes.

      This repository uses STM32Cube HAL/LL drivers for peripheral access. The project is built using GCC (arm-none-eabi toolchain) and Make for compilation.


## Hardware
    * MCU: STM32L072CBT6 (LQFP48 package, low-power series with internal DAC and multiple I2C peripherals).
    * Pressure Sensor (MS583730BA01-50): Connected via I2C2 (SCL on PB10, SDA on PB11). Powered from 3V3 rail, address 0x76. Pull-up resistors (2.2kΩ) on SCL and SDA. TVS protection (NUP2105LT1G) present.
    * I2C Slave: Implemented on I2C1 (SCL on PA9, SDA on PA10) to ensure separation from the sensor's bus. TVS protection (NUP2105LT1G) present. Slave address assumed as 0x10 (configurable in code). No pull-up resistors shown on this bus—assuming provided by the external master; if not, add 4.7kΩ externally.
    * DAC Outputs: Using internal DAC channels (DAC1_OUT1 on PA4, DAC1_OUT2 on PA5). Outputs clipped to 0-3.3V range (VREF+ = VDDA = 3V3, filtered via ferrite bead FB1).
    * Timer: TIM2 used for 2 ms interrupt triggering (configurable prescaler and period for ~500 Hz). No external pins required for internal timing.
    * Power/Voltage Rails: MCU and sensor on 3V3 rail (derived from 12V input via buck converter TPS561208 in the schematic, but firmware assumes stable 3V3). GND common. VDDA connected to 3V3 via filter (FB1 120Ω ferrite bead). No separate analog reference shown.
    * Clock: No external oscillator connected (OSC_IN and OSC_OUT floating); using internal HSI (16 MHz).

    * Assumptions: External DACs (MCP4725) and associated op-amps (OPA192) removed as per assignment—internal DAC outputs used directly without buffering. If buffering is needed, outputs would connect to former VOUT points of external DACs. I2C slave does not use INTR_MCU signal (not required for assignment; in schematic, it's routed to connector but not tied to an MCU GPIO in the parsed netlist—assumed optional). Connector J1 for sensor uses pins: 1 CLK (SCL), 2 SDA, 3 VDD (3V3), 4 GND (decoupling cap C1 100nF). If schematic implies different configurations, update board/board_config.h accordingly.


## Prerequisites
    * ARM GCC Toolchain (arm-none-eabi-gcc, version 10+ recommended).
    * STM32CubeL0 HAL/LL libraries (download from STMicroelectronics and place in hal/ or use provided stubs).
    * Make for building.
    * Git for version control.
    * Optional: OpenOCD or ST-Link for flashing/debugging.



## Build and Run Instructions
    1) Clone the repository:
        """"
        git clone <repo-url>
        cd <repo-name>
        """"
        
    2) Ensure STM32CubeL0 is in hal/stm32cube/ (or symlink it).
    
    3) Build:
        make
    This compiles to build/firmware.elf (and .bin/.hex).
    
    4) Flash to MCU (using ST-Link):
        st-flash write build/firmware.bin 0x8000000 
    Or use OpenOCD/ST-Link Utility.
    
    5) Monitor via UART (if enabled in code) or debug with ST-Link.


## Folder Structure
   The project structure is designed for portability:
    * Platform-specific code (HAL, board configs) is isolated in hal/ and board/.
    * Drivers are reusable and MCU-agnostic where possible.
    * Application logic is in app/, independent of hardware details.
    
      (Assumption - most of the structures will be preserved after regenerating the code with STM32CubeMX. 
      .
      ├── README.md                # This file: project overview, assumptions, instructions.
      ├── Makefile                 # Build script using GCC.
      ├── linker.ld                # Linker script for STM32L0 (memory layout).
      ├── startup/                 # Startup code and vector table.
      │   └── startup_stm32l072xx.s
      ├── inc/                     # Global includes (redirects to subfolders).
      ├── src/                     # Main source files.
      │   └── main.c               # Entry point: initializes HAL, drivers, and app.
      ├── hal/                     # Platform-specific HAL/LL (STM32CubeL0 subset).
      │   ├── stm32cube/           # STM32CubeL0 files (CMSIS, HAL/LL drivers for I2C, TIM, DAC).
      │   └── hal_config.h         # HAL initialization configs.
      ├── board/                   # Board-specific configurations (pins, clocks).
      │   ├── board_config.h       # Pin mappings (e.g., I2C pins, DAC channels).
      │   └── board_init.c         # Board-specific init (clock setup, GPIO).
      ├── drivers/                 # Reusable peripheral drivers.
      │   ├── pressure_sensor/     # Adapted MS583730BA01 driver.
      │   │   ├── ms58.c           # Driver implementation (from provided code, adapted to HAL/LL).
      │   │   ├── ms58.h           # Header.
      │   │   └── ms58_regs.h      # Registers.
      │   ├── i2c_slave/           # I2C slave driver.
      │   │   ├── i2c_slave.c      # Handles receive/reply of 32-bit values.
      │   │   └── i2c_slave.h
      │   └── dac/                 # DAC driver.
      │       ├── dac.c            # API for voltage setting (volts to codes).
      │       └── dac.h
      ├── app/                     # Portable application logic.
      │   ├── app.c                # Main loop: processes samples, I2C, DAC.
      │   ├── app.h
      │   ├── sensor_sampling.c    # Interrupt handler for timer-based sampling.
      │   └── sensor_sampling.h
      ├── build/                   # Build artifacts (generated).
      └── docs/                    # Additional docs and provided files. There are many variations depends on complexity of the project.
          ├── datasheets/
          │   ├── MS583730BA01-50.pdf  # Sensor datasheet.
          │   └── stm32l072v8.pdf      # MCU datasheet.
          ├── schematics/
          │   └── Schematics.pdf       # Hardware schematic.
          ├── project_management/  # Core project management documents.
          │   ├── software_requirements_specification.doc
          │   ├── software_peripheral_design.doc
          │   ├── software_construction_design.doc    
          │   ├── project_schedule.xlsx # Project time-management sheet.
          │   └── issue_tracker.xlsx    # Issue tracker.
          └── Assignment.pdf
          
## MAKE

    """
        make                       
        CC  src/main.c
        CC  src/memset.c
        CC  src/libc_init.c
        CC  board/board_init.c
        CC  hal/hal_config.c
        CC  drivers/pressure_sensor/ms58.c
        CC  drivers/pressure_sensor/ms58_hal_wrapper.c
        CC  drivers/i2c_slave/i2c_slave.c
        CC  drivers/dac/dac.c
        CC  app/app.c
        CC  app/sensor_sampling.c
        CC  hal/stm32cube/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim_ex.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dac.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dac_ex.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c
        CC  hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c
        AS  startup/startup_stm32l072xx.s
        LD  build/firmware.elf
        Memory region         Used Size  Region Size  %age Used
                FLASH:       16904 B       192 KB      8.60%
                    RAM:        1912 B        20 KB      9.34%
        Size:
        text    data     bss     dec     hex filename
        16888      16    1896   18800    4970 build/firmware.elf
        OBJCOPY build/firmware.bin
        OBJCOPY build/firmware.hex

    """