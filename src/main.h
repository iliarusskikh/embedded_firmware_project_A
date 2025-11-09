#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
/**
 * @file main.h
 * @brief Main application entry point and initialization
 * 
 * This file provides the main application entry point and initialization
 * sequence. The main function orchestrates the initialization of HAL,
 * board hardware, drivers, and application logic.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Main application entry point
 * 
 * Initialization sequence:
 * 1. HAL initialization
 * 2. Board hardware initialization (clocks, GPIO)
 * 3. Driver initialization (I2C, DAC, Timer, Pressure Sensor)
 * 4. Application initialization
 * 5. Main application loop
 * 
 * @return Exit code (should never return in embedded system)
 */
int main(void);

/**
 * @brief Error handler for fatal errors
 * 
 * Called when a critical error occurs. This function should handle
 * the error appropriately (e.g., enter error state, blink LED, etc.)
 * 
 * @param error_code Error code indicating the type of error
 */
void main_error_handler(uint32_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */

