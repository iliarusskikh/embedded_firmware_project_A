#include "ms58.h"
#include "ms58_regs.h"
#include <stdint.h>
#include <limits.h>  /* For INT32_MAX, INT32_MIN */
#include <stddef.h>  /* For NULL */


// Reset the sensor
ms583730ba01_err_t ms5837_reset(const ms583730ba01_h *h) {
    ms583730ba01_err_t result = h->write_cmd(MS5837_RESET);
    if (result != E_MS58370BA01_SUCCESS) {
        return result;  // Return if write failed
    }
      h->delay(3);// Wait for sensor to reset (minimum 2.8ms per datasheet)
    return E_MS58370BA01_SUCCESS;
}

//CRC-4 calculation function (based on the datasheet)



// Read calibration data from PROM with CRC check
ms583730ba01_err_t ms5837_read_prom(const ms583730ba01_h *h, uint16_t *calibration_data) {
    uint8_t cmd;
    uint8_t data[2];
    ms583730ba01_err_t result;

    for (int i = 0; i < 7; i++) {
        cmd = MS5837_PROM_READ_BASE + (i * 2);
        //printk("Sending command 0x%X for coefficient %d\n", cmd, i);

        result = h->write_cmd(cmd);
        if (result != E_MS58370BA01_SUCCESS) {
            //printk("Failed to send PROM read command for coefficient %d, error: %d\n", i, result);
            return result;
        }

        result = h->read_data(data, 2);
        if (result != E_MS58370BA01_SUCCESS) {
            //printk("Failed to read PROM data for coefficient %d, error: %d\n", i, result);
            return result;
        }

        calibration_data[i] = (data[0] << 8) | data[1];
        //printk("Coefficient C%d: %u\n", i, calibration_data[i]);
    }
    return E_MS58370BA01_SUCCESS;
}





// ADC read function
ms583730ba01_err_t ms5837_read_adc(const ms583730ba01_h *h, uint32_t *data) {
    uint8_t adc_data[3];
    ms583730ba01_err_t result;

    // Send ADC read command
    result = h->write_cmd(MS5837_ADC_READ);
    if (result != E_MS58370BA01_SUCCESS) {
        return result;  // Return if write failed
    }

    // Read 3 bytes of ADC data
    result = h->read_data(adc_data, 3);
    if (result != E_MS58370BA01_SUCCESS) {
        return result;  // Return if read failed
    }

    *data = ((uint32_t)adc_data[0] << 16) | ((uint32_t)adc_data[1] << 8) | adc_data[2];
    return E_MS58370BA01_SUCCESS;
}

// Start conversion (pressure or temperature)
ms583730ba01_err_t ms5837_start_conversion(const ms583730ba01_h *h, uint8_t cmd) {
    return h->write_cmd(cmd);  // Send conversion command
}

ms583730ba01_err_t ms5837_read_temperature_and_pressure(
    const ms583730ba01_h *h, uint16_t *calibration_data, int32_t *pressure, int32_t *temperature,
    int osr_d1, int osr_d2, uint16_t delay_d1, uint16_t delay_d2
) {
    uint32_t D1 = 0, D2 = 0;  // Raw ADC values
    int32_t dT;               // Temperature difference
    int64_t OFF, SENS, P;     // Intermediate calculations
    ms583730ba01_err_t result;

    // 1) Start pressure conversion (D1) with the specified oversampling ratio
    result = ms5837_start_conversion(h, osr_d1);
    h->delay(delay_d1); 
    if (result != E_MS58370BA01_SUCCESS) {
        return result;
    }

    // 2) Read ADC result for D1
    result = ms5837_read_adc(h, &D1);
    if (result != E_MS58370BA01_SUCCESS) {
        return result;
    }

    // 3) Start temperature conversion (D2) with the specified oversampling ratio
    result = ms5837_start_conversion(h, osr_d2);
    h->delay(delay_d2);
    if (result != E_MS58370BA01_SUCCESS) {
        return result;
    }

    // 4) Read ADC result for D2
    result = ms5837_read_adc(h, &D2);
    if (result != E_MS58370BA01_SUCCESS) {
        return result;
    }

    // 5) Calculate dT (difference between actual and reference temperature)
    dT = (int32_t)D2 - ((int32_t)calibration_data[5] * 256);  // C5 * 2^8

    // 6) Calculate temperature (TEMP)
    *temperature = 2000 + ((int64_t)dT * calibration_data[6]) / 8388608;  // C6 / 2^23

    // 7) Calculate OFF (Offset at actual temperature)
    OFF = ((int64_t)calibration_data[2] * 131072)  // C2 * 2^17
        + (((int64_t)calibration_data[4] * dT) / 64);  // C4 * dT / 2^6

    // 8) Calculate SENS (Sensitivity at actual temperature)
    SENS = ((int64_t)calibration_data[1] * 65536)  // C1 * 2^16
         + (((int64_t)calibration_data[3] * dT) / 128);  // C3 * dT / 2^7

    // 9) Calculate pressure (P)
    P = ((((int64_t)D1 * SENS) / 2097152) - OFF) / 32768;  // (D1 * SENS / 2^21 - OFF) / 2^15
    
    // Overflow protection: Clamp pressure to int32_t range before casting
    if (P > INT32_MAX) {
        *pressure = INT32_MAX;
    } else if (P < INT32_MIN) {
        *pressure = INT32_MIN;
    } else {
        *pressure = (int32_t)P;  // Convert to mbar (pressure in 0.01 mbar resolution)
    }
    
    // Overflow protection: Clamp temperature to int32_t range (shouldn't overflow, but safe)
    // Note: *temperature is int32_t, so it can't be < INT32_MIN, only check upper bound
    if (*temperature > INT32_MAX) {
        *temperature = INT32_MAX;
    }

    return E_MS58370BA01_SUCCESS;
}

// Calculate pressure and temperature from ADC values (calculation only)
ms583730ba01_err_t ms5837_calculate_pressure_temperature(
    const uint16_t *calibration_data,
    uint32_t d1_pressure,
    uint32_t d2_temperature,
    int32_t *pressure,
    int32_t *temperature
) {
    if (calibration_data == NULL || pressure == NULL || temperature == NULL) {
        return E_MS58370BA01_NULLPTR_ERR;
    }
    
    int32_t dT;               // Temperature difference
    int64_t OFF, SENS, P;     // Intermediate calculations
    
    // Calculate dT (difference between actual and reference temperature)
    dT = (int32_t)d2_temperature - ((int32_t)calibration_data[5] * 256);  // C5 * 2^8
    
    // Calculate temperature (TEMP)
    *temperature = 2000 + ((int64_t)dT * calibration_data[6]) / 8388608;  // C6 / 2^23
    
    // Calculate OFF (Offset at actual temperature)
    OFF = ((int64_t)calibration_data[2] * 131072)  // C2 * 2^17
        + (((int64_t)calibration_data[4] * dT) / 64);  // C4 * dT / 2^6
    
    // Calculate SENS (Sensitivity at actual temperature)
    SENS = ((int64_t)calibration_data[1] * 65536)  // C1 * 2^16
         + (((int64_t)calibration_data[3] * dT) / 128);  // C3 * dT / 2^7
    
    // Calculate pressure (P)
    P = ((((int64_t)d1_pressure * SENS) / 2097152) - OFF) / 32768;  // (D1 * SENS / 2^21 - OFF) / 2^15
    
    // Overflow protection: Clamp pressure to int32_t range before casting
    if (P > INT32_MAX) {
        *pressure = INT32_MAX;
    } else if (P < INT32_MIN) {
        *pressure = INT32_MIN;
    } else {
        *pressure = (int32_t)P;  // Convert to mbar (pressure in 0.01 mbar resolution)
    }
    
    // Overflow protection: Clamp temperature to int32_t range (shouldn't overflow, but safe)
    // Note: *temperature is int32_t, so it can't be < INT32_MIN, only check upper bound
    if (*temperature > INT32_MAX) {
        *temperature = INT32_MAX;
    }
    
    return E_MS58370BA01_SUCCESS;
}

