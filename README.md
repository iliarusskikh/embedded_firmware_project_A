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
    * MCU: STM32L072V8.
    * Pressure Sensor (MS583730BA01-50): Connected via I2C1. Powered from a 3.3V rail.
    * I2C Slave: Implemented on I2C2 to ensure separation from the sensor's bus. Slave address assumed as 0x10 (configurable in code; document if changed).
    * DAC Outputs: Using internal DAC channels. Outputs clipped to 0-3.3V range (assuming VREF+ = 3.3V).
    * Timer: TIM2 used for 2 ms interrupt triggering (configurable prescaler and period for ~500 Hz).
    * Power/Voltage Rails: All components on 3.3V rail; no external DACs as per updated assignment.


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
          
