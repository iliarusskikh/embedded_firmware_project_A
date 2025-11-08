#ifndef MS5837_H
#define MS5837_H

#include <stdint.h>
#include <stdbool.h>

#include "ms58_regs.h"



// Error codes
typedef enum {
    E_MS58370BA01_SUCCESS = 0,            //!< Success
    E_MS58370BA01_NULLPTR_ERR = (1 << 0), //!< Nullpointer error
    E_MS58370BA01_COM_ERR = (1 << 1),     //!< Communication error
    E_MS58370BA01_CONFIG_ERR = (1 << 2),  //!< Configuration error
    E_MS58370BA01_ERR = (1 << 3),         //!< Other error
} ms583730ba01_err_t;

#define MS5837_ADDR               0x76  // I2C address of the MS5837 sensor

// Function pointer structure for I2C communication
typedef struct {
    ms583730ba01_err_t (*write_cmd)(uint8_t cmd);
    ms583730ba01_err_t (*read_data)(uint8_t *buf, uint32_t n);
    void (*delay)(uint16_t ms);
} ms583730ba01_h;

// Function prototypes
 //delay is platform specific and must be implemented same as i2c read/write
ms583730ba01_err_t ms5837_reset(const ms583730ba01_h *h);
ms583730ba01_err_t ms5837_read_prom(const ms583730ba01_h *h, uint16_t *calibration_data);
ms583730ba01_err_t ms5837_start_conversion(const ms583730ba01_h *h, uint8_t cmd);
ms583730ba01_err_t ms5837_read_adc(const ms583730ba01_h *h, uint32_t *data);
ms583730ba01_err_t ms5837_read_temperature_and_pressure(
    const ms583730ba01_h *h, uint16_t *calibration_data, int32_t *pressure, int32_t *temperature,
    int osr_d1, int osr_d2, uint16_t delay_d1, uint16_t delay_d2
);

/**
 * @brief Calculate pressure and temperature from ADC values
 * 
 * This function performs the calculation only, using pre-read ADC values.
 * Use this when you've already read D1 and D2 separately.
 * 
 * @param calibration_data Calibration coefficients from PROM (7 values)
 * @param d1_pressure Raw pressure ADC value (D1)
 * @param d2_temperature Raw temperature ADC value (D2)
 * @param pressure Calculated pressure output (0.01 mbar resolution)
 * @param temperature Calculated temperature output (0.01°C resolution)
 * @return ms583730ba01_err_t Error code
 */
ms583730ba01_err_t ms5837_calculate_pressure_temperature(
    const uint16_t *calibration_data,
    uint32_t d1_pressure,
    uint32_t d2_temperature,
    int32_t *pressure,
    int32_t *temperature
);

#endif // MS5837_H
