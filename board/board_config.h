#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/**
 * @file board_config.h
 * @brief Board-specific pin mappings and peripheral configurations
 * 
 * This file defines all pin assignments and peripheral configurations
 * for the STM32L072CBT6 board. These definitions are platform-specific
 * and should be updated when retargeting to a different MCU/board.
 */

#include <stdint.h>

/* ============================================================================
 * PIN DEFINITIONS
 * ============================================================================ */

/* I2C2 - Pressure Sensor (MS583730BA01-50) */
#define BOARD_I2C2_SCL_PORT        GPIOB
#define BOARD_I2C2_SCL_PIN         10
#define BOARD_I2C2_SCL_AF           4  /* AF4 for I2C2_SCL on PB10 */

#define BOARD_I2C2_SDA_PORT        GPIOB
#define BOARD_I2C2_SDA_PIN         11
#define BOARD_I2C2_SDA_AF           4  /* AF4 for I2C2_SDA on PB11 */

/* I2C1 - I2C Slave Communication */
#define BOARD_I2C1_SCL_PORT        GPIOA
#define BOARD_I2C1_SCL_PIN         9
#define BOARD_I2C1_SCL_AF           4  /* AF4 for I2C1_SCL on PA9 */

#define BOARD_I2C1_SDA_PORT        GPIOA
#define BOARD_I2C1_SDA_PIN         10
#define BOARD_I2C1_SDA_AF           4  /* AF4 for I2C1_SDA on PA10 */

/* DAC Outputs */
#define BOARD_DAC1_OUT1_PORT       GPIOA
#define BOARD_DAC1_OUT1_PIN        4
#define BOARD_DAC1_OUT1_CHANNEL    DAC_CHANNEL_1  /* STM32 HAL define */

#define BOARD_DAC1_OUT2_PORT       GPIOA
#define BOARD_DAC1_OUT2_PIN        5
#define BOARD_DAC1_OUT2_CHANNEL    DAC_CHANNEL_2  /* STM32 HAL define */

/* ============================================================================
 * PERIPHERAL CONFIGURATIONS
 * ============================================================================ */

/* I2C2 - Pressure Sensor Configuration */
#define BOARD_I2C2_PERIPH          I2C2
#define BOARD_I2C2_SENSOR_ADDR     0x76  /* MS583730BA01-50 I2C address */

/* I2C1 - I2C Slave Configuration */
#define BOARD_I2C1_PERIPH           I2C1
#define BOARD_I2C1_SLAVE_ADDR       0x10  /* Configurable slave address */

/* Timer Configuration */
#define BOARD_TIM2_PERIPH           TIM2
#define BOARD_TIM2_FREQ_HZ          500   /* 500 Hz = 2 ms period */
#define BOARD_TIM2_PRESCALER        1600  /* Adjust based on system clock */
#define BOARD_TIM2_PERIOD           1000  /* Adjust based on prescaler */

/* DAC Configuration */
#define BOARD_DAC_PERIPH            DAC1
#define BOARD_DAC_VREF_VOLTS        3.3f  /* Reference voltage in volts */
#define BOARD_DAC_RESOLUTION_BITS   12    /* 12-bit DAC resolution */
#define BOARD_DAC_MAX_CODE          ((1UL << BOARD_DAC_RESOLUTION_BITS) - 1)

/* ============================================================================
 * CLOCK CONFIGURATION
 * ============================================================================ */

/* System Clock Configuration */
#define BOARD_SYSCLK_FREQ_HZ        16000000UL  /* 16 MHz HSI */
#define BOARD_HSI_FREQ_HZ           16000000UL
#define BOARD_APB1_FREQ_HZ          BOARD_SYSCLK_FREQ_HZ
#define BOARD_APB2_FREQ_HZ          BOARD_SYSCLK_FREQ_HZ

/* ============================================================================
 * PLATFORM-SPECIFIC TYPES
 * ============================================================================ */

/* GPIO Port type (platform-specific) */
typedef void* board_gpio_port_t;

/* GPIO Pin type (platform-specific) */
typedef uint16_t board_gpio_pin_t;

/* Peripheral handle type (platform-specific) */
typedef void* board_periph_handle_t;

#endif /* BOARD_CONFIG_H */

