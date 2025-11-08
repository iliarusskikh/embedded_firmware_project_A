# Interrupt Flow Documentation

## Overview
This document explains how the 2ms interrupt-based pressure sensor sampling works.

## Interrupt Flow Chain

```
Hardware Timer (TIM2)
    ↓ (every 2ms)
TIM2_IRQHandler() [main.c]
    ↓
HAL_TIM_IRQHandler(&htim2)
    ↓
HAL_TIM_PeriodElapsedCallback() [main.c]
    ↓
sensor_sampling_timer_isr() [sensor_sampling.c]
    ↓
State Machine (advances one step per interrupt)
```

## Where the Interrupt is Called

### 1. Hardware Level
- **TIM2** is configured to generate an interrupt every **2ms** (500 Hz)
- Configuration: `hal/hal_config.c::hal_tim2_init()`
  - Prescaler: 1600 (timer clock = 10 kHz)
  - Period: 20 counts (2ms period)
  - Interrupt enabled via `HAL_NVIC_EnableIRQ(TIM2_IRQn)`

### 2. Interrupt Handler
- **Location**: `src/main.c::TIM2_IRQHandler()`
- **Function**: Entry point called by the interrupt vector table
- **Action**: Calls `HAL_TIM_IRQHandler(&htim2)` to process the interrupt

### 3. HAL Callback
- **Location**: `src/main.c::HAL_TIM_PeriodElapsedCallback()`
- **Function**: Called by HAL when timer period elapses
- **Action**: Verifies it's TIM2, then calls `sensor_sampling_timer_isr()`

### 4. Sensor Sampling State Machine
- **Location**: `app/sensor_sampling.c::sensor_sampling_timer_isr()` (line 143)
- **Function**: Advances the sensor reading state machine one step per interrupt
- **Action**: Performs one step of the multi-step sensor reading process

## Where You Read Every 2ms

The timer interrupt **fires every 2ms**, but the actual sensor reading takes **multiple interrupts** because:

1. Sensor conversion requires time (~0.6ms minimum, we wait 2ms to be safe)
2. The reading process has multiple steps:
   - Start pressure conversion
   - Wait for conversion (1 interrupt = 2ms)
   - Read pressure ADC
   - Start temperature conversion
   - Wait for conversion (1 interrupt = 2ms)
   - Read temperature ADC
   - Calculate pressure and temperature

### Complete Reading Cycle Timeline

| Interrupt # | Time (ms) | State | Action |
|------------|-----------|-------|--------|
| 1 | 0 | START_PRESSURE_CONV | Start pressure conversion command |
| 2 | 2 | WAIT_PRESSURE_CONV | Wait (counter decrements) |
| 3 | 4 | READ_PRESSURE_ADC | Read pressure ADC value |
| 4 | 6 | START_TEMP_CONV | Start temperature conversion command |
| 5 | 8 | WAIT_TEMP_CONV | Wait (counter decrements) |
| 6 | 10 | READ_TEMP_ADC | Read temperature ADC value |
| 7 | 12 | CALCULATE | Calculate P & T, mark data valid, start next cycle |

**Result**: A new pressure/temperature reading is available approximately every **12ms** (6 interrupts), even though the timer fires every 2ms.

### Accessing the Data

To read the latest sensor data from your application:

```c
sensor_data_t data;
if (sensor_sampling_get_data(&data)) {
    // data.pressure contains pressure in 0.01 mbar
    // data.temperature contains temperature in 0.01°C
    // data.valid is true
}
```

**Location**: `app/sensor_sampling.c::sensor_sampling_get_data()`

This function is **thread-safe** and can be called from the main loop or any non-interrupt context.

## Initialization Sequence

1. `main()` calls `hal_tim2_init()` - Configures TIM2 and enables interrupt
2. `main()` calls `sensor_sampling_init()` - Initializes sensor and loads calibration
3. `main()` calls `sensor_sampling_start()` - Starts the state machine
4. `main()` calls `hal_tim2_start()` - Starts TIM2 timer, interrupts begin

## Vector Table

The interrupt vector table is typically in the startup file (`startup/startup_stm32l072xx.s`). The linker automatically connects `TIM2_IRQHandler` to the TIM2 interrupt vector. No manual configuration needed if the function name matches the expected symbol.

## Summary

- **Timer fires**: Every 2ms (500 Hz)
- **Interrupt handler**: `TIM2_IRQHandler()` in `main.c`
- **State machine**: `sensor_sampling_timer_isr()` in `sensor_sampling.c`
- **New reading available**: Approximately every 12ms (6 interrupts)
- **Read data**: Call `sensor_sampling_get_data()` from main loop

