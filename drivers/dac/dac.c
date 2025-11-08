/**
 * @file dac.c
 * @brief DAC driver implementation
 * 
 * This module implements the DAC driver with voltage-to-code conversion
 * and automatic clipping to valid ranges.
 */

 /*
    Related files:
        drivers/dac/dac.h — API with function declarations
        drivers/dac/dac.c — Implementation with voltage-to-code conversion
        src/main.c — Integrated DAC initialization
        app/app.c — Added DAC include and example usage
        docs/DAC_IMPLEMENTATION.md — Documentation

    Features
        Accepts volts: functions take voltage in engineering units
        Automatic conversion: volts → 12-bit DAC codes
        Automatic clipping: clamps to 0.0V–3.3V range
        Dual channel: supports both DAC channels (PA4 and PA5)
        Helper functions: voltage ↔ code conversion utilities
 
 */
#include "dac.h"
#include "board_config.h"
#include "hal_config.h"
#include "stm32l0xx_hal.h"

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================ */

static bool dac_initialized = false;

/* External HAL handle - defined in main.c */
extern DAC_HandleTypeDef hdac1;

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief Clip voltage to valid range
 * 
 * Clips voltage to the valid DAC range (0.0V to VREF).
 * 
 * @param voltage_volts Input voltage
 * @return Clipped voltage
 */
static float dac_clip_voltage(float voltage_volts)
{
    if (voltage_volts < 0.0f) {
        return 0.0f;
    } else if (voltage_volts > BOARD_DAC_VREF_VOLTS) {
        return BOARD_DAC_VREF_VOLTS;
    }
    return voltage_volts;
}

/**
 * @brief Get HAL channel from DAC channel enum
 * 
 * @param channel DAC channel enum
 * @return HAL DAC channel constant
 */
static uint32_t dac_get_hal_channel(dac_channel_t channel)
{
    if (channel == DAC_CHANNEL_OUT1) {
        return BOARD_DAC1_OUT1_CHANNEL;
    } else {
        return BOARD_DAC1_OUT2_CHANNEL;
    }
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

bool dac_init(void)
{
    /* Verify HAL DAC is initialized */
    if (hdac1.Instance != BOARD_DAC_PERIPH) {
        return false;
    }
    
    /* Enable both DAC channels */
    if (HAL_DAC_Start(&hdac1, BOARD_DAC1_OUT1_CHANNEL) != HAL_OK) {
        return false;
    }
    
    if (HAL_DAC_Start(&hdac1, BOARD_DAC1_OUT2_CHANNEL) != HAL_OK) {
        return false;
    }
    
    /* Initialize both channels to 0V */
    dac_set_voltage_ch1(0.0f);
    dac_set_voltage_ch2(0.0f);
    
    dac_initialized = true;
    return true;
}

bool dac_set_voltage_ch1(float voltage_volts)
{
    return dac_set_voltage(DAC_CHANNEL_OUT1, voltage_volts);
}

bool dac_set_voltage_ch2(float voltage_volts)
{
    return dac_set_voltage(DAC_CHANNEL_OUT2, voltage_volts);
}

bool dac_set_voltage(dac_channel_t channel, float voltage_volts)
{
    if (!dac_initialized) {
        return false;
    }
    
    /* Clip voltage to valid range */
    float clipped_voltage = dac_clip_voltage(voltage_volts);
    
    /* Convert voltage to DAC code */
    uint16_t dac_code = dac_voltage_to_code(clipped_voltage);
    
    /* Get HAL channel */
    uint32_t hal_channel = dac_get_hal_channel(channel);
    
    /* Set DAC value */
    if (HAL_DAC_SetValue(&hdac1, hal_channel, DAC_ALIGN_12B_R, dac_code) != HAL_OK) {
        return false;
    }
    
    return true;
}

uint16_t dac_voltage_to_code(float voltage_volts)
{
    /* Clip voltage to valid range */
    float clipped = dac_clip_voltage(voltage_volts);
    
    /* Convert to DAC code: code = (voltage / VREF) * MAX_CODE */
    float normalized = clipped / BOARD_DAC_VREF_VOLTS;
    float code_float = normalized * (float)BOARD_DAC_MAX_CODE;
    
    /* Round to nearest integer */
    uint16_t code = (uint16_t)(code_float + 0.5f);
    
    /* Ensure code is within valid range (should be, but double-check) */
    if (code > BOARD_DAC_MAX_CODE) {
        code = BOARD_DAC_MAX_CODE;
    }
    
    return code;
}

float dac_code_to_voltage(uint16_t dac_code)
{
    /* Clamp code to valid range */
    if (dac_code > BOARD_DAC_MAX_CODE) {
        dac_code = BOARD_DAC_MAX_CODE;
    }
    
    /* Convert to voltage: voltage = (code / MAX_CODE) * VREF */
    float normalized = (float)dac_code / (float)BOARD_DAC_MAX_CODE;
    float voltage = normalized * BOARD_DAC_VREF_VOLTS;
    
    return voltage;
}

float dac_get_vref(void)
{
    return BOARD_DAC_VREF_VOLTS;
}

uint8_t dac_get_resolution_bits(void)
{
    return BOARD_DAC_RESOLUTION_BITS;
}

