#ifndef DAC_H
#define DAC_H

/**
 * @file dac.h
 * @brief DAC driver for STM32L0 internal DAC
 * 
 * This module provides a high-level API for controlling the STM32L0 internal DAC.
 * Functions accept voltage values in engineering units (volts) and automatically
 * convert them to DAC codes with proper clipping to the valid range.
 * 
 * Features:
 * - Accepts voltage in volts (engineering units)
 * - Automatic conversion to 12-bit DAC codes
 * - Automatic clipping to valid range (0.0V to VREF)
 * - Support for both DAC channels (OUT1 and OUT2)
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief DAC channel enumeration
 */
typedef enum {
    DAC_CHANNEL_OUT1 = 0,  /* DAC1 Channel 1 (PA4) */
    DAC_CHANNEL_OUT2 = 1   /* DAC1 Channel 2 (PA5) */
} dac_channel_t;

/* ============================================================================
 * FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize DAC driver
 * 
 * Initializes the DAC driver. The HAL DAC peripheral must be initialized
 * before calling this function (via hal_dac1_init()).
 * 
 * @return true if initialization successful, false otherwise
 */
bool dac_init(void);

/**
 * @brief Set DAC output voltage for channel 1
 * 
 * Sets the DAC output voltage for channel 1 (PA4).
 * The voltage is automatically converted to DAC codes and clipped to
 * the valid range (0.0V to VREF).
 * 
 * @param voltage_volts Desired output voltage in volts (0.0 to VREF)
 * @return true if successful, false otherwise
 */
bool dac_set_voltage_ch1(float voltage_volts);

/**
 * @brief Set DAC output voltage for channel 2
 * 
 * Sets the DAC output voltage for channel 2 (PA5).
 * The voltage is automatically converted to DAC codes and clipped to
 * the valid range (0.0V to VREF).
 * 
 * @param voltage_volts Desired output voltage in volts (0.0 to VREF)
 * @return true if successful, false otherwise
 */
bool dac_set_voltage_ch2(float voltage_volts);

/**
 * @brief Set DAC output voltage for specified channel
 * 
 * Sets the DAC output voltage for the specified channel.
 * The voltage is automatically converted to DAC codes and clipped to
 * the valid range (0.0V to VREF).
 * 
 * @param channel DAC channel (DAC_CHANNEL_OUT1 or DAC_CHANNEL_OUT2)
 * @param voltage_volts Desired output voltage in volts (0.0 to VREF)
 * @return true if successful, false otherwise
 */
bool dac_set_voltage(dac_channel_t channel, float voltage_volts);

/**
 * @brief Convert voltage to DAC code
 * 
 * Converts a voltage value (in volts) to the corresponding 12-bit DAC code.
 * The voltage is clipped to the valid range before conversion.
 * 
 * @param voltage_volts Voltage in volts
 * @return 12-bit DAC code (0 to 4095)
 */
uint16_t dac_voltage_to_code(float voltage_volts);

/**
 * @brief Convert DAC code to voltage
 * 
 * Converts a 12-bit DAC code to the corresponding voltage value.
 * 
 * @param dac_code 12-bit DAC code (0 to 4095)
 * @return Voltage in volts
 */
float dac_code_to_voltage(uint16_t dac_code);

/**
 * @brief Get DAC reference voltage
 * 
 * Returns the DAC reference voltage (VREF) in volts.
 * 
 * @return Reference voltage in volts
 */
float dac_get_vref(void);

/**
 * @brief Get DAC resolution in bits
 * 
 * Returns the DAC resolution in bits.
 * 
 * @return DAC resolution in bits (12 for STM32L0)
 */
uint8_t dac_get_resolution_bits(void);

#ifdef __cplusplus
}
#endif

#endif // DAC_H
