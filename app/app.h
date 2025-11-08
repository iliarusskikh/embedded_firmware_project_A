#ifndef APP_H
#define APP_H

/**
 * @file app.h
 * @brief Application layer - coordinates all system components
 * 
 * This module provides the main application logic that coordinates
 * sensor sampling, I2C slave communication, and DAC control.
 */

#include <stdint.h>
#include <stdbool.h>
#include "sensor_sampling.h"  /* For sensor_data_t type */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize application layer
 * 
 * Initializes all application components including sensor sampling,
 * I2C slave, and DAC. This should be called after all drivers are initialized.
 * 
 * @return true if initialization successful, false otherwise
 */
bool app_init(void);

/**
 * @brief Main application loop
 * 
 * This function is called from the main loop to process application logic.
 * Most work is done in interrupt handlers, but this can handle non-critical
 * tasks and coordination.
 */
void app_main_loop(void);

/**
 * @brief Get latest sensor reading count
 * 
 * Returns the number of sensor readings that have been processed.
 * Useful for statistics and debugging.
 * 
 * @return Number of readings processed since initialization
 */
uint32_t app_get_reading_count(void);

/**
 * @brief Get latest sensor data
 * 
 * Returns a copy of the most recently read sensor data.
 * This is a convenience function that wraps sensor_sampling_get_data().
 * 
 * @param data Pointer to sensor_data_t structure to fill
 * @return true if data is valid, false if no valid data available
 */
bool app_get_latest_sensor_data(sensor_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */

