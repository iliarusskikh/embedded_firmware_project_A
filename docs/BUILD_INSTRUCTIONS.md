# Build Instructions

### Prerequisites

1. **Install ARM GCC Toolchain**:
   ```bash
   # macOS (using Homebrew)
   brew install arm-none-eabi-gcc
   
   # Or download from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain
   ```

2. **Install STM32CubeL0 HAL**:
   - Download from STMicroelectronics website
   - Extract to `hal/stm32cube/` directory
   - Should have structure:
     ```
     hal/stm32cube/
       ├── Drivers/
       │   ├── CMSIS/
       │   └── STM32L0xx_HAL_Driver/
       └── ...
     ```

### Building

1. **Use the Makefile** (not direct gcc):
   ```bash
   cd /embedded_firmware_project_A
   make
   ```

2. **What the Makefile does**:
   - Uses `arm-none-eabi-gcc` (correct toolchain)
   - Sets up all include paths automatically
   - Compiles all source files
   - Links with proper linker script
   - Generates `.elf`, `.bin`, and `.hex` files

### Build Output

After running `make`, you should see:
```
build/
  ├── firmware.elf
  ├── firmware.bin
  ├── firmware.hex
  └── firmware.map
```

### If STM32Cube HAL is Missing

If you get errors about missing STM32 HAL files:

1. **Option 1**: Download STM32CubeL0 from STMicroelectronics
2. **Option 2**: Create minimal stubs (for compilation only):
   ```bash
   # Create minimal HAL stubs if needed
   mkdir -p hal/stm32cube/Drivers/STM32L0xx_HAL_Driver/Inc
   mkdir -p hal/stm32cube/Drivers/CMSIS/Device/ST/STM32L0xx/Include
   mkdir -p hal/stm32cube/Drivers/CMSIS/Include
   ```

### Common Issues

1. **"arm-none-eabi-gcc: command not found"**
   - Install ARM GCC toolchain (see Prerequisites)

2. **"stm32l0xx_hal.h: No such file"**
   - STM32CubeL0 HAL not installed
   - Place in `hal/stm32cube/` directory

3. **"linker.ld: No such file"**
   - The linker script should be in project root
   - Makefile expects it at `linker.ld`

### Visual Studio Code

If using VS Code, you can:
1. Use the integrated terminal: `make`
2. Or create a `.vscode/tasks.json` for build tasks

### Alternative: Manual Compilation (Not Recommended)

If you must compile manually (not recommended):
```bash
arm-none-eabi-gcc \
  -mcpu=cortex-m0plus -mthumb \
  -I./src -I./board -I./hal -I./drivers/pressure_sensor \
  -I./drivers/i2c_slave -I./drivers/dac -I./app \
  -DSTM32L072xx -DUSE_HAL_DRIVER \
  main.c board_init.c hal_config.c ... \
  -T linker.ld -o firmware.elf
```

**But use the Makefile instead!** It handles all this automatically.

