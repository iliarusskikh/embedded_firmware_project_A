#ifndef SENSOR_SAMPLING_H
#define SENSOR_SAMPLING_H

/**
 * @file sensor_sampling.h
 * @brief Interrupt-driven pressure sensor sampling
 * 
 * This module handles timer-based sampling of the MS5837 pressure sensor.
 * Sampling is triggered by hardware timer interrupts at approximately 2ms intervals.
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
 * @brief Sensor data structure
 */
typedef struct {
    int32_t pressure;      /* Pressure in 0.01 mbar (from sensor calculation) */
    int32_t temperature; /* Temperature in 0.01°C (from sensor calculation) */
    bool valid;          /* True if data is valid and ready */
} sensor_data_t;

/* ============================================================================
 * FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize sensor sampling module
 * 
 * Initializes the pressure sensor and prepares for interrupt-based sampling.
 * This should be called after I2C2 and TIM2 are initialized.
 * 
 * @return true if initialization successful, false otherwise
 */
bool sensor_sampling_init(void);

/**
 * @brief Start sensor sampling
 * 
 * Starts the timer interrupt to begin periodic sensor sampling.
 * 
 * @return true if start successful, false otherwise
 */
bool sensor_sampling_start(void);

/**
 * @brief Stop sensor sampling
 * 
 * Stops the timer interrupt and halts sensor sampling.
 * 
 * @return true if stop successful, false otherwise
 */
bool sensor_sampling_stop(void);

/**
 * @brief Get latest sensor data
 * 
 * Retrieves the most recently sampled pressure and temperature values.
 * This function is safe to call from the main application loop.
 * 
 * @param data Pointer to sensor_data_t structure to fill
 * @return true if data is valid, false if no valid data available
 */
bool sensor_sampling_get_data(sensor_data_t *data);

/**
 * @brief Timer interrupt handler
 * 
 * This function should be called from the TIM2 interrupt handler.
 * It manages the state machine for sensor reading.
 * 
 * NOTE: This function must be called from interrupt context.
 */
void sensor_sampling_timer_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_SAMPLING_H */

