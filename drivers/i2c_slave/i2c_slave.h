#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

/**
 * @file i2c_slave.h
 * @brief I2C slave driver for receiving and transmitting 32-bit values
 * 
 * This module implements I2C slave functionality to:
 * - Receive a 32-bit value from an external I2C master (write operation)
 * - Reply with a 32-bit value when requested by the master (read operation)
 * 
 * The implementation uses interrupt-based handling for efficient operation.
 */

#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"  /* For I2C_HandleTypeDef */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief I2C slave callback function type
 * 
 * Called when a 32-bit value is received from the master.
 * 
 * @param received_value The 32-bit value received from master
 */
typedef void (*i2c_slave_rx_callback_t)(uint32_t received_value);

/**
 * @brief I2C slave callback function type for providing data to send
 * 
 * Called when master requests data. Should return the 32-bit value to send.
 * 
 * @return 32-bit value to send to master
 */
typedef uint32_t (*i2c_slave_tx_callback_t)(void);

/* ============================================================================
 * FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize I2C slave
 * 
 * Initializes the I2C peripheral as a slave device and enables interrupts.
 * The I2C peripheral handle must be initialized before calling this function.
 * 
 * @param hi2c Pointer to I2C handle (must be initialized via hal_i2c1_init)
 * @param slave_addr 7-bit I2C slave address
 * @return true if initialization successful, false otherwise
 */
bool i2c_slave_init(I2C_HandleTypeDef *hi2c, uint8_t slave_addr);

/**
 * @brief Start I2C slave listening
 * 
 * Starts the I2C slave to listen for master requests.
 * Must be called after i2c_slave_init().
 * 
 * @return true if start successful, false otherwise
 */
bool i2c_slave_start(void);

/**
 * @brief Stop I2C slave
 * 
 * Stops the I2C slave from listening for master requests.
 * 
 * @return true if stop successful, false otherwise
 */
bool i2c_slave_stop(void);

/**
 * @brief Register callback for received data
 * 
 * Registers a callback function that will be called when a 32-bit value
 * is received from the master.
 * 
 * @param callback Function to call when data is received (NULL to disable)
 */
void i2c_slave_register_rx_callback(i2c_slave_rx_callback_t callback);

/**
 * @brief Register callback for data to transmit
 * 
 * Registers a callback function that will be called when the master requests
 * data. The callback should return the 32-bit value to send.
 * 
 * @param callback Function to call when data is requested (NULL to disable)
 */
void i2c_slave_register_tx_callback(i2c_slave_tx_callback_t callback);

/**
 * @brief Get last received value
 * 
 * Returns the most recently received 32-bit value from the master.
 * This is a non-blocking function.
 * 
 * @param value Pointer to store the received value
 * @return true if a new value was received, false otherwise
 */
bool i2c_slave_get_received_value(uint32_t *value);

/**
 * @brief Set value to transmit
 * 
 * Sets the 32-bit value that will be sent when the master requests data.
 * This is an alternative to using the TX callback.
 * 
 * @param value 32-bit value to send on next read request
 */
void i2c_slave_set_tx_value(uint32_t value);

/**
 * @brief I2C slave interrupt handler
 * 
 * This function should be called from the I2C1 interrupt handler.
 * It processes I2C slave events (address match, receive, transmit).
 * 
 * NOTE: This function must be called from interrupt context.
 */
void i2c_slave_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif // I2C_SLAVE_H
