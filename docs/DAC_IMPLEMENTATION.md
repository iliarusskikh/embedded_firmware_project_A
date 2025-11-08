# DAC Implementation Summary

## Overview
DAC driver has been implemented to control the STM32L0 internal DAC with APIs that accept voltage values in engineering units (volts) and automatically convert them to DAC codes with proper clipping.

## Implementation Details

### 1. Driver Module (`drivers/dac/`)

#### `dac.h`
- API for DAC operations
- Functions accept voltage in volts (engineering units)
- Support for both DAC channels (OUT1 and OUT2)
- Helper functions for voltage/code conversion

#### `dac.c`
- **Voltage-to-code conversion**: Automatically converts volts to 12-bit DAC codes
- **Automatic clipping**: Clips voltage to valid range (0.0V to VREF = 3.3V)
- **Channel support**: Both DAC channels (PA4 and PA5)
- **Initialization**: Enables both channels and sets to 0V

### 2. Key Features

#### Voltage-to-Code Conversion
```c
uint16_t dac_voltage_to_code(float voltage_volts)
```
- Formula: `code = (voltage / VREF) * MAX_CODE`
- VREF = 3.3V (from `board_config.h`)
- MAX_CODE = 4095 (12-bit: 2^12 - 1)
- Automatic clipping to 0.0V - 3.3V range

#### Code-to-Voltage Conversion
```c
float dac_code_to_voltage(uint16_t dac_code)
```
- Formula: `voltage = (code / MAX_CODE) * VREF`
- Useful for verification and debugging

#### Set Voltage Functions
```c
bool dac_set_voltage_ch1(float voltage_volts);  // Channel 1 (PA4)
bool dac_set_voltage_ch2(float voltage_volts);    // Channel 2 (PA5)
bool dac_set_voltage(dac_channel_t channel, float voltage_volts);
```
- Accepts voltage in volts
- Automatically clips to valid range
- Converts to DAC code
- Sets DAC output via HAL

### 3. Configuration

From `board_config.h`:
- **VREF**: 3.3V (`BOARD_DAC_VREF_VOLTS`)
- **Resolution**: 12 bits (`BOARD_DAC_RESOLUTION_BITS`)
- **Max Code**: 4095 (`BOARD_DAC_MAX_CODE`)
- **Channel 1**: PA4 (`BOARD_DAC1_OUT1_CHANNEL`)
- **Channel 2**: PA5 (`BOARD_DAC1_OUT2_CHANNEL`)

### 4. Integration

#### Initialization Sequence
1. `hal_dac1_init()` - HAL DAC initialization (in `main_init_drivers()`)
2. `dac_init()` - DAC driver initialization (in `main_init_drivers()`)
   - Enables both channels
   - Sets both outputs to 0V

#### Usage Example
```c
// Set Channel 1 to 1.65V (half of 3.3V)
dac_set_voltage_ch1(1.65f);

// Set Channel 2 to 2.5V
dac_set_voltage_ch2(2.5f);

// Set using channel enum
dac_set_voltage(DAC_CHANNEL_OUT1, 3.0f);

// Values outside range are automatically clipped
dac_set_voltage_ch1(5.0f);  // Clipped to 3.3V
dac_set_voltage_ch1(-1.0f); // Clipped to 0.0V
```

### 5. Application Integration

#### Example Usage (Commented in `app/app.c`)
```c
// Map pressure to 0-3.3V
float dac1_voltage = (pressure_mbar / 3000.0f) * 3.3f;
dac_set_voltage_ch1(dac1_voltage);

// Map temperature to 0-3.3V
float dac2_voltage = ((temperature_c + 20.0f) / 105.0f) * 3.3f;
dac_set_voltage_ch2(dac2_voltage);
```

## Conversion Details

### Voltage to Code
```
Input: voltage_volts (e.g., 1.65V)
  ↓ Clip to 0.0V - 3.3V
  ↓ Normalize: normalized = voltage / 3.3V
  ↓ Scale: code_float = normalized * 4095
  ↓ Round: code = (uint16_t)(code_float + 0.5)
Output: dac_code (e.g., 2048 for 1.65V)
```

### Code to Voltage
```
Input: dac_code (e.g., 2048)
  ↓ Clamp to 0 - 4095
  ↓ Normalize: normalized = code / 4095
  ↓ Scale: voltage = normalized * 3.3V
Output: voltage_volts (e.g., 1.65V)
```

## Notes

- **Clipping**: Values outside 0.0V - 3.3V are automatically clipped
- **Resolution**: 12-bit (4096 levels, 0.805 mV per step)
- **Channels**: Both channels operate independently
- **Initialization**: Both channels start at 0V
- **Portability**: All configuration via `board_config.h` macros

