#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

/**
 * @file hal_config.h
 * @brief HAL peripheral configuration and initialization
 * 
 * This file provides HAL peripheral initialization functions for STM32L0.
 * These functions configure I2C, DAC, and Timer peripherals using STM32 HAL.
 */

#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim2;

/**
 * @brief Initialize I2C2 peripheral for pressure sensor
 * 
 * Configures I2C2 with appropriate speed and settings for MS583730BA01 sensor.
 * 
 * @return true if initialization successful, false otherwise
 */
bool hal_i2c2_init(void);

/**
 * @brief Initialize I2C1 peripheral for I2C slave
 * 
 * Configures I2C1 as a slave device with the configured address.
 * 
 * @return true if initialization successful, false otherwise
 */
bool hal_i2c1_init(void);

/**
 * @brief Initialize TIM2 for 2ms interrupt-based sampling
 * 
 * Configures TIM2 to generate interrupts at approximately 2ms intervals (500 Hz).
 * 
 * @return true if initialization successful, false otherwise
 */
bool hal_tim2_init(void);

/**
 * @brief Initialize DAC1 peripheral
 * 
 * Configures DAC1 for analog voltage output on both channels.
 * 
 * @return true if initialization successful, false otherwise
 */
bool hal_dac1_init(void);

/**
 * @brief Start TIM2 to begin generating interrupts
 * 
 * @return true if start successful, false otherwise
 */
bool hal_tim2_start(void);

/**
 * @brief Stop TIM2 interrupts
 * 
 * @return true if stop successful, false otherwise
 */
bool hal_tim2_stop(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_CONFIG_H
