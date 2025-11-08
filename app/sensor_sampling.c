/**
 * @file sensor_sampling.c
 * @brief Interrupt-driven pressure sensor sampling implementation
 * 
 * This module implements timer-based sampling of the MS5837 pressure sensor.
 * Uses a state machine to handle the multi-step sensor reading process
 * across multiple timer interrupts.

 * State machine for multi-step sensor reading:
 * Start pressure conversion → Wait → Read pressure ADC
 * Start temperature conversion → Wait → Read temperature ADC
 * Calculate pressure and temperature
 * Timer interrupt handler (sensor_sampling_timer_isr())
 * Thread-safe data access via sensor_sampling_get_data()
 * Uses OSR=256 for minimum conversion time (~0.6ms each)
 
 * Interrupt	Time	State	What Happens
 * 1	0ms	START_PRESSURE_CONV	Send pressure conversion command
 * 2	2ms	WAIT_PRESSURE_CONV	Wait for conversion (1 interrupt = 2ms)
 * 3	4ms	READ_PRESSURE_ADC	Read pressure ADC value
 * 4	6ms	START_TEMP_CONV	Send temperature conversion command
 * 5	8ms	WAIT_TEMP_CONV	Wait for conversion (1 interrupt = 2ms)
 * 6	10ms	READ_TEMP_ADC	Read temperature ADC value
 * 7	12ms	CALCULATE	Calculate P & T, mark data valid
 * Result: new readings available approximately every 12ms (6 interrupts), even though the timer fires every 2ms.
 
 */

#include "sensor_sampling.h"
#include "ms58.h"
#include "ms58_regs.h"
#include "ms58_hal_wrapper.h"
#include "board_config.h"
#include "board_init.h"
#include "stm32l0xx_hal.h"

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================ */

/* Sensor reading state machine */
typedef enum {
    SENSOR_STATE_IDLE,
    SENSOR_STATE_START_PRESSURE_CONV,
    SENSOR_STATE_WAIT_PRESSURE_CONV,
    SENSOR_STATE_READ_PRESSURE_ADC,
    SENSOR_STATE_START_TEMP_CONV,
    SENSOR_STATE_WAIT_TEMP_CONV,
    SENSOR_STATE_READ_TEMP_ADC,
    SENSOR_STATE_CALCULATE,
    SENSOR_STATE_ERROR
} sensor_state_t;

/* OSR (Oversampling Ratio) settings - using minimum for 2ms sampling */
#define SENSOR_OSR_D1    MS5837_CONVERT_D1_256  /* Pressure conversion OSR=256 */
#define SENSOR_OSR_D2    MS5837_CONVERT_D2_256  /* Temperature conversion OSR=256 */
/* Delay in timer interrupts (each interrupt = 2ms)
 * OSR=256 requires ~0.6ms, but we wait 1 interrupt (2ms) to be safe */
#define SENSOR_DELAY_D1_INTERRUPTS  1  /* 1 interrupt = 2ms for pressure conversion */
#define SENSOR_DELAY_D2_INTERRUPTS  1  /* 1 interrupt = 2ms for temperature conversion */

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================ */

static ms583730ba01_h sensor_handle;
static uint16_t calibration_data[7];
static bool calibration_loaded = false;
static sensor_state_t sensor_state = SENSOR_STATE_IDLE;
static sensor_data_t latest_data = {0};
static uint32_t pressure_adc = 0;
static uint32_t temperature_adc = 0;
static uint32_t wait_counter = 0;

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief Load calibration data from sensor PROM
 */
static bool sensor_load_calibration(void)
{
    ms583730ba01_err_t result;
    
    if (calibration_loaded) {
        return true;  /* Already loaded */
    }
    
    /* Reset sensor */
    result = ms5837_reset(&sensor_handle);
    if (result != E_MS58370BA01_SUCCESS) {
        return false;
    }
    
    /* Read calibration data from PROM */
    result = ms5837_read_prom(&sensor_handle, calibration_data);
    if (result != E_MS58370BA01_SUCCESS) {
        return false;
    }
    
    calibration_loaded = true;
    return true;
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

bool sensor_sampling_init(void)
{
    /* Get HAL handle for sensor */
    sensor_handle = ms58_get_hal_handle();
    
    /* Load calibration data */
    if (!sensor_load_calibration()) {
        return false;
    }
    
    /* Initialize state */
    sensor_state = SENSOR_STATE_IDLE;
    latest_data.valid = false;
    
    return true;
}

bool sensor_sampling_start(void)
{
    /* Reset state machine to start sampling */
    sensor_state = SENSOR_STATE_START_PRESSURE_CONV;
    wait_counter = 0;
    
    return true;
}

bool sensor_sampling_stop(void)
{
    sensor_state = SENSOR_STATE_IDLE;
    return true;
}

bool sensor_sampling_get_data(sensor_data_t *data)
{
    if (data == NULL) {
        return false;
    }
    
    if (latest_data.valid) {
        *data = latest_data;
        return true;
    }
    
    return false;
}

void sensor_sampling_timer_isr(void)
{
    ms583730ba01_err_t result;
    
    /* State machine for sensor reading */
    // switch method could be further replaced and optimised for performace improvements
    switch (sensor_state) {
        case SENSOR_STATE_IDLE:
            /* Do nothing, waiting for start */
            break;
            
        case SENSOR_STATE_START_PRESSURE_CONV:
            /* Start pressure conversion */
            result = ms5837_start_conversion(&sensor_handle, SENSOR_OSR_D1);
            if (result == E_MS58370BA01_SUCCESS) {
                sensor_state = SENSOR_STATE_WAIT_PRESSURE_CONV;
                wait_counter = SENSOR_DELAY_D1_INTERRUPTS;  /* Wait 1 interrupt (2ms) */
            } else {
                sensor_state = SENSOR_STATE_ERROR;
            }
            break;
            
        case SENSOR_STATE_WAIT_PRESSURE_CONV:
            /* Wait for pressure conversion to complete */
            if (wait_counter > 0) {
                wait_counter--;
            }
            if (wait_counter == 0) {
                sensor_state = SENSOR_STATE_READ_PRESSURE_ADC;
            }
            break;
            
        case SENSOR_STATE_READ_PRESSURE_ADC:
            /* Read pressure ADC value */
            result = ms5837_read_adc(&sensor_handle, &pressure_adc);
            if (result == E_MS58370BA01_SUCCESS) {
                sensor_state = SENSOR_STATE_START_TEMP_CONV;
            } else {
                sensor_state = SENSOR_STATE_ERROR;
            }
            break;
            
        case SENSOR_STATE_START_TEMP_CONV:
            /* Start temperature conversion */
            result = ms5837_start_conversion(&sensor_handle, SENSOR_OSR_D2);
            if (result == E_MS58370BA01_SUCCESS) {
                sensor_state = SENSOR_STATE_WAIT_TEMP_CONV;
                wait_counter = SENSOR_DELAY_D2_INTERRUPTS;  /* Wait 1 interrupt (2ms) */
            } else {
                sensor_state = SENSOR_STATE_ERROR;
            }
            break;
            
        case SENSOR_STATE_WAIT_TEMP_CONV:
            /* Wait for temperature conversion to complete */
            if (wait_counter > 0) {
                wait_counter--;
            }
            if (wait_counter == 0) {
                sensor_state = SENSOR_STATE_READ_TEMP_ADC;
            }
            break;
            
        case SENSOR_STATE_READ_TEMP_ADC:
            /* Read temperature ADC value */
            result = ms5837_read_adc(&sensor_handle, &temperature_adc);
            if (result == E_MS58370BA01_SUCCESS) {
                sensor_state = SENSOR_STATE_CALCULATE;
            } else {
                sensor_state = SENSOR_STATE_ERROR;
            }
            break;
            
        case SENSOR_STATE_CALCULATE:
            /* Calculate pressure and temperature from ADC values */
            result = ms5837_calculate_pressure_temperature(
                calibration_data,
                pressure_adc,
                temperature_adc,
                &latest_data.pressure,
                &latest_data.temperature
            );
            
            if (result == E_MS58370BA01_SUCCESS) {
                latest_data.valid = true;
                /* Start next sampling cycle */
                sensor_state = SENSOR_STATE_START_PRESSURE_CONV;
            } else {
                sensor_state = SENSOR_STATE_ERROR;
            }
            break;
            
        case SENSOR_STATE_ERROR:
            /* Error state - try to recover by resetting */
            latest_data.valid = false;
            /* Wait a few cycles before retrying */
            /* Overflow protection: Prevent wait_counter from wrapping */
            if (wait_counter < UINT32_MAX) {
                wait_counter++;
            }
            if (wait_counter > 10) {
                wait_counter = 0;
                sensor_state = SENSOR_STATE_START_PRESSURE_CONV;
            }
            break;
            
        default:
            sensor_state = SENSOR_STATE_IDLE;
            break;
    }
}

