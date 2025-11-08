/**
 * @file ms58_hal_wrapper.c
 * @brief STM32 HAL wrapper for MS5837 pressure sensor driver
 * 
 * This file provides platform-specific I2C communication functions that
 * bridge STM32 HAL to the portable MS5837 driver. This allows the ms58.c
 * driver to remain platform-independent.
 */

#include "ms58.h"
#include "ms58_regs.h"
#include "board_config.h"
#include "board_init.h"
#include "stm32l0xx_hal.h"

/* External I2C handle for pressure sensor */
extern I2C_HandleTypeDef hi2c2;

/* ============================================================================
 * I2C Communication Functions (Platform-Specific)
 * ============================================================================ */

/**
 * @brief Write command to MS5837 sensor via I2C
 * 
 * Platform-specific implementation using STM32 HAL.
 * 
 * @param cmd Command byte to send
 * @return ms583730ba01_err_t Error code
 */
static ms583730ba01_err_t ms58_hal_write_cmd(uint8_t cmd)
{
    HAL_StatusTypeDef status;
    
    /* MS5837 uses write-only commands (no data) */
    status = HAL_I2C_Master_Transmit(&hi2c2, 
                                     (BOARD_I2C2_SENSOR_ADDR << 1),
                                     &cmd, 
                                     1, 
                                     HAL_MAX_DELAY);
    
    if (status == HAL_OK) {
        return E_MS58370BA01_SUCCESS;
    } else {
        return E_MS58370BA01_COM_ERR;
    }
}

/**
 * @brief Read data from MS5837 sensor via I2C
 * 
 * Platform-specific implementation using STM32 HAL.
 * 
 * @param buf Buffer to store read data
 * @param n Number of bytes to read
 * @return ms583730ba01_err_t Error code
 */
static ms583730ba01_err_t ms58_hal_read_data(uint8_t *buf, uint32_t n)
{
    HAL_StatusTypeDef status;
    
    if (buf == NULL) {
        return E_MS58370BA01_NULLPTR_ERR;
    }
    
    status = HAL_I2C_Master_Receive(&hi2c2,
                                    (BOARD_I2C2_SENSOR_ADDR << 1),
                                    buf,
                                    n,
                                    HAL_MAX_DELAY);
    
    if (status == HAL_OK) {
        return E_MS58370BA01_SUCCESS;
    } else {
        return E_MS58370BA01_COM_ERR;
    }
}

/**
 * @brief Delay function using board delay
 * 
 * @param ms Delay time in milliseconds
 */
static void ms58_hal_delay(uint16_t ms)
{
    board_delay_ms(ms);
}

/* ============================================================================
 * Driver Handle Initialization
 * ============================================================================ */

/**
 * @brief Get initialized MS5837 driver handle
 * 
 * Returns a handle structure with function pointers for I2C communication.
 * This handle can be used with all ms58.c driver functions.
 * 
 * @return ms583730ba01_h Initialized driver handle
 */
ms583730ba01_h ms58_get_hal_handle(void)
{
    ms583730ba01_h handle = {
        .write_cmd = ms58_hal_write_cmd,
        .read_data = ms58_hal_read_data,
        .delay = ms58_hal_delay
    };
    
    return handle;
}

