/**
 * @file app.c
 * @brief Application layer implementation
 * 
 * This module coordinates sensor sampling, I2C slave communication, and DAC control.
 */

#include "app.h"
#include "sensor_sampling.h"
#include "hal_config.h"
#include "i2c_slave.h"
#include <stdio.h>  /* For printf */

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================ */

static bool app_initialized = false;
static sensor_data_t latest_sensor_data = {0};
static uint32_t reading_count = 0;

/* ============================================================================
 * CONSTANTS - Sensor Value Ranges (from MS5837-30BA datasheet)
 * ============================================================================ */
/* MS5837-30BA typical ranges:
 * Pressure: 0-30 bar = 0-3000 mbar = 0-300,000 (in 0.01 mbar units)
 * Temperature: -20°C to +85°C = -2000 to +8500 (in 0.01°C units)
 * Adding safety margins for overflow protection */
#define PRESSURE_MIN_RAW     (-500000L)   /* -5000 mbar (safety margin) */
#define PRESSURE_MAX_RAW     (500000L)    /* 5000 mbar (safety margin, sensor max ~3000) */
#define TEMPERATURE_MIN_RAW  (-50000L)    /* -500°C (safety margin) */
#define TEMPERATURE_MAX_RAW  (100000L)    /* 1000°C (safety margin) */
#define READING_COUNT_MAX   (UINT32_MAX - 1)  /* Prevent overflow */

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief I2C slave receive callback
 * 
 * Called automatically when a 32-bit value is received from I2C master.
 * This runs in interrupt context, so keep it short!
 * 
 * @param received_value The 32-bit value received from master
 */
static void app_i2c_slave_rx_callback(uint32_t received_value)
{
    /* Print received value - commented out but retained for debugging */
    /* Uncomment to enable printf output (requires UART/USB setup) */
    /*
    printf("[I2C Slave] Received 32-bit value: 0x%08lX (%lu)\r\n", 
           (unsigned long)received_value, 
           (unsigned long)received_value);
    printf("  Hex: 0x%08lX\r\n", (unsigned long)received_value);
    printf("  Decimal: %lu\r\n", (unsigned long)received_value);
    printf("  Bytes: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
           (uint8_t)(received_value & 0xFF),
           (uint8_t)((received_value >> 8) & 0xFF),
           (uint8_t)((received_value >> 16) & 0xFF),
           (uint8_t)((received_value >> 24) & 0xFF));
    printf("\r\n");
    */
    
    /* NOTE: This callback runs in interrupt context!
     * Keep processing minimal here. For heavy processing,
     * store the value and process it in app_main_loop() */
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

bool app_init(void)
{
    /* Initialize sensor sampling */
    if (!sensor_sampling_init()) {
        return false;
    }
    
    /* I2C slave is initialized in main_init_drivers() */
    /* I2C slave is started in main_init_app() */
    
    /* Register I2C slave RX callback to print received values */
    i2c_slave_register_rx_callback(app_i2c_slave_rx_callback);
    
    /* TODO: Initialize DAC */
    
    app_initialized = true;
    return true;
}

void app_main_loop(void)
{
    /* Main application loop - most work is done in interrupt handlers */
    /* This function can be used for non-critical tasks, logging, etc. */
    
    if (!app_initialized) {
        return;
    }
    
    /* ========================================================================
     * READ AND PROCESS SENSOR DATA
     * ======================================================================== */
    
    /* Attempt to read latest sensor data */
    if (sensor_sampling_get_data(&latest_sensor_data)) {
        /* Data is valid and ready */
        
        /* Overflow protection: Prevent reading_count from wrapping */
        if (reading_count < READING_COUNT_MAX) {
            reading_count++;
        }
        /* else: Count has reached maximum, keep at max to prevent overflow */
        
        /* Overflow protection: Clamp sensor values to expected ranges */
        int32_t pressure_clamped = latest_sensor_data.pressure;
        int32_t temperature_clamped = latest_sensor_data.temperature;
        
        if (pressure_clamped < PRESSURE_MIN_RAW) {
            pressure_clamped = PRESSURE_MIN_RAW;
        } else if (pressure_clamped > PRESSURE_MAX_RAW) {
            pressure_clamped = PRESSURE_MAX_RAW;
        }
        
        if (temperature_clamped < TEMPERATURE_MIN_RAW) {
            temperature_clamped = TEMPERATURE_MIN_RAW;
        } else if (temperature_clamped > TEMPERATURE_MAX_RAW) {
            temperature_clamped = TEMPERATURE_MAX_RAW;
        }
        
        /* Convert raw values to engineering units for display */
        /* Pressure: sensor returns in 0.01 mbar, convert to mbar */
        /* Use clamped values to prevent overflow in float conversion */
        float pressure_mbar = (float)pressure_clamped / 100.0f;
        
        /* Temperature: sensor returns in 0.01°C, convert to °C */
        float temperature_c = (float)temperature_clamped / 100.0f;
        
        /* Optional: Convert pressure to other units */
        /* Overflow protection: Check if multiplication would overflow float range */
        float pressure_psi = pressure_mbar * 0.0145038f;  /* 1 mbar ≈ 0.0145 PSI */
        float pressure_pa = pressure_mbar * 100.0f;        /* 1 mbar = 100 Pa */
        
        /* Clamp float values to prevent infinity/NaN */
        if (pressure_pa > 1e6f) pressure_pa = 1e6f;  /* Max 1 MPa */
        if (pressure_pa < -1e6f) pressure_pa = -1e6f;
        if (pressure_psi > 150.0f) pressure_psi = 150.0f;  /* Max ~150 PSI */
        if (pressure_psi < -150.0f) pressure_psi = -150.0f;
        
        /* Debug output - commented out but retained for debugging */
        /* Uncomment these lines to enable printf output (requires UART/USB setup) */
        /*
        printf("[Sensor Reading #%lu]\r\n", (unsigned long)reading_count);
        printf("  Pressure:   %ld (0.01 mbar) = %.2f mbar = %.2f PSI = %.2f Pa\r\n",
               pressure_clamped,  // Use clamped value for safety 
               pressure_mbar,
               pressure_psi,
               pressure_pa);
        printf("  Temperature: %ld (0.01°C) = %.2f°C\r\n",
               temperature_clamped,  // Use clamped value for safety
               temperature_c);
        printf("  Valid:       %s\r\n", latest_sensor_data.valid ? "Yes" : "No");
        if (pressure_clamped != latest_sensor_data.pressure || 
            temperature_clamped != latest_sensor_data.temperature) {
            printf("  WARNING: Values clamped to prevent overflow!\r\n");
        }
        printf("\r\n");
        */
        
        /* Store latest reading for other application modules */
        /* This data can be used by I2C slave, DAC control, etc. */
        
        /* Update I2C slave TX value with latest pressure reading */
        /* When master reads, it will get the latest pressure value */
        i2c_slave_set_tx_value((uint32_t)pressure_clamped);
        
        /* TODO: Process received I2C slave data if needed */
        /* uint32_t received_value;
         * if (i2c_slave_get_received_value(&received_value)) {
         *     // Process received value from master
         * }
         */
        
        /* TODO: Update DAC outputs based on sensor data or I2C commands */
    }
    /* else: No new data available yet, sensor still reading or error occurred */
    
    /* The sensor sampling is handled entirely in interrupt context */
    /* This function just reads and processes the results */
}

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

uint32_t app_get_reading_count(void)
{
    return reading_count;
}

bool app_get_latest_sensor_data(sensor_data_t *data)
{
    if (data == NULL) {
        return false;
    }
    
    /* Get data directly from sensor sampling module */
    return sensor_sampling_get_data(data);
}

