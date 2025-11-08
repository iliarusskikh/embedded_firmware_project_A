# I2C Slave Implementation Summary

## Overview
I2C slave functionality has been implemented to receive 32-bit values from an external master and reply with 32-bit values on request.

## Implementation Details

### 1. Driver Module (`drivers/i2c_slave/`)

#### `i2c_slave.h`
- API for I2C slave operations
- Callback function types for RX and TX
- Functions for initialization, start/stop, and data access

#### `i2c_slave.c`
- **Receive functionality**: Receives 4 bytes (32-bit value) from master
- **Transmit functionality**: Sends 4 bytes (32-bit value) to master
- **Interrupt-based**: Uses HAL I2C interrupt callbacks
- **Byte order**: Little-endian (LSB first)
- **State machine**: Tracks RX/TX/IDLE states

### 2. HAL Configuration (`hal/hal_config.c`)

- I2C1 configured as slave with address `BOARD_I2C1_SLAVE_ADDR` (0x10)
- I2C interrupts enabled:
  - `I2C1_IRQn` (event interrupt)
  - `I2C1_ER_IRQn` (error interrupt)
- Interrupt priority set to 1 (lower than timer interrupt)

### 3. Interrupt Handlers (`src/main.c`)

- `I2C1_EV_IRQHandler()`: Handles I2C1 event interrupts
- `I2C1_ER_IRQHandler()`: Handles I2C1 error interrupts
- Both call `i2c_slave_irq_handler()` which processes the interrupt

### 4. HAL Callbacks (`drivers/i2c_slave/i2c_slave.c`)

- `HAL_I2C_AddrCallback()`: Called when master addresses slave
  - Detects write (master → slave) or read (slave → master) direction
  - Prepares RX or TX buffers accordingly
- `HAL_I2C_SlaveRxCpltCallback()`: Called when receive completes
  - Converts 4 bytes to 32-bit value
  - Calls user RX callback if registered
  - Re-enables listening
- `HAL_I2C_SlaveTxCpltCallback()`: Called when transmit completes
  - Re-enables listening for next transaction
- `HAL_I2C_ErrorCallback()`: Called on I2C errors
  - Clears errors and re-enables listening
- `HAL_I2C_ListenCpltCallback()`: Called when listen mode completes
  - Re-enables listening

### 5. Application Integration (`app/app.c`)

- I2C slave TX value updated with latest pressure reading
- Latest sensor data automatically sent when master reads
- Received values can be processed via `i2c_slave_get_received_value()`

## Data Flow

### Master Write (Master → Slave)
1. Master addresses slave (0x10) with write bit
2. `HAL_I2C_AddrCallback()` called with `I2C_DIRECTION_TRANSMIT`
3. Slave prepares to receive 4 bytes
4. Master sends 4 bytes (32-bit value, little-endian)
5. `HAL_I2C_SlaveRxCpltCallback()` called
6. Bytes converted to `uint32_t`
7. User RX callback called (if registered)
8. Value available via `i2c_slave_get_received_value()`

### Master Read (Slave → Master)
1. Master addresses slave (0x10) with read bit
2. `HAL_I2C_AddrCallback()` called with `I2C_DIRECTION_RECEIVE`
3. Slave gets value to send:
   - Calls TX callback if registered, OR
   - Uses value set via `i2c_slave_set_tx_value()`
4. Value converted to 4 bytes (little-endian)
5. Slave transmits 4 bytes to master
6. `HAL_I2C_SlaveTxCpltCallback()` called
7. Re-enables listening

## Initialization Sequence

1. `hal_i2c1_init()` - Configures I2C1 as slave, enables interrupts
2. `i2c_slave_init(&hi2c1, BOARD_I2C1_SLAVE_ADDR)` - Initializes driver
3. `i2c_slave_start()` - Starts listening for master requests

## Usage Example

```c
// Initialize (done in main_init_drivers())
i2c_slave_init(&hi2c1, 0x10);

// Start listening (done in main_init_app())
i2c_slave_start();

// Set value to send when master reads
i2c_slave_set_tx_value(0x12345678);

// Register callback for received data
void my_rx_callback(uint32_t value) {
    // Process received value
}
i2c_slave_register_rx_callback(my_rx_callback);

// Or poll for received data
uint32_t received;
if (i2c_slave_get_received_value(&received)) {
    // Process received value
}
```

## Current Integration

- **TX value**: Automatically updated with latest pressure reading in `app_main_loop()`
- **RX value**: Can be read via `i2c_slave_get_received_value()` if needed
- **Callbacks**: Can be registered for event-driven processing


## Notes

- **Byte order**: Little-endian (LSB first)
- **Data size**: Fixed 32-bit (4 bytes)
- **Address**: Configurable via `BOARD_I2C1_SLAVE_ADDR` (default: 0x10)
- **Interrupt priority**: 1 (lower than timer interrupt priority 2)
- **Error handling**: Automatic recovery via error callback

