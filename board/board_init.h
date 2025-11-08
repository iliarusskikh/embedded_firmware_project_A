#ifndef BOARD_INIT_H
#define BOARD_INIT_H

/**
 * @file board_init.h
 * @brief Board initialization functions
 * 
 * This header provides platform-specific board initialization functions.
 * These functions handle clock setup, GPIO configuration, and peripheral
 * initialization that is specific to the hardware platform.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize board hardware (clocks, GPIO, peripherals)
 * 
 * This function performs all platform-specific initialization:
 * - System clock configuration (HSI 16 MHz)
 * - GPIO pin configuration for I2C, DAC, etc.
 * - Peripheral clock enables
 * 
 * @return true if initialization successful, false otherwise
 */
bool board_init(void);

/**
 * @brief Initialize system clock
 * 
 * Configures the system clock to use HSI (16 MHz internal oscillator).
 * This is platform-specific and should be updated when retargeting.
 * 
 * @return true if clock initialization successful, false otherwise
 */
bool board_init_clock(void);

/**
 * @brief Initialize GPIO pins
 * 
 * Configures all GPIO pins used by the application:
 * - I2C1 pins (PA9, PA10) for I2C slave
 * - I2C2 pins (PB10, PB11) for pressure sensor
 * - DAC pins (PA4, PA5) for analog outputs
 * 
 * @return true if GPIO initialization successful, false otherwise
 */
bool board_init_gpio(void);

/**
 * @brief Get system clock frequency in Hz
 * 
 * @return System clock frequency in Hz
 */
uint32_t board_get_sysclk_freq(void);

/**
 * @brief Get APB1 peripheral clock frequency in Hz
 * 
 * @return APB1 clock frequency in Hz
 */
uint32_t board_get_apb1_freq(void);

/**
 * @brief Get APB2 peripheral clock frequency in Hz
 * 
 * @return APB2 clock frequency in Hz
 */
uint32_t board_get_apb2_freq(void);

/**
 * @brief Delay function (milliseconds)
 * 
 * Platform-specific delay implementation. Uses a blocking delay
 * based on the system clock frequency.
 * 
 * @param ms Delay time in milliseconds
 */
void board_delay_ms(uint32_t ms);

/**
 * @brief Delay function (microseconds)
 * 
 * Platform-specific delay implementation. Uses a blocking delay
 * based on the system clock frequency.
 * 
 * @param us Delay time in microseconds
 */
void board_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_INIT_H */

