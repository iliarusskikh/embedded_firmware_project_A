#ifndef MS58_HAL_WRAPPER_H
#define MS58_HAL_WRAPPER_H

/**
 * @file ms58_hal_wrapper.h
 * @brief STM32 HAL wrapper for MS5837 pressure sensor driver
 */

#include "ms58.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get initialized MS5837 driver handle for STM32 HAL
 * 
 * Returns a handle structure with function pointers configured for
 * STM32 HAL I2C communication. Use this handle with all ms58.c driver functions.
 * 
 * @return ms583730ba01_h Initialized driver handle
 */
ms583730ba01_h ms58_get_hal_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* MS58_HAL_WRAPPER_H */

