/**
 * @file i2c_slave.c
 * @brief I2C slave driver implementation
 * 
 * This module implements interrupt-based I2C slave functionality for
 * receiving and transmitting 32-bit values.
 */

/*
    Related files:
        drivers/i2c_slave/i2c_slave.h — API with function declarations
        drivers/i2c_slave/i2c_slave.c — Interrupt-based I2C slave driver
        src/main.c — Added I2C1 interrupt handlers
        hal/hal_config.c — Enabled I2C1 interrupts
        app/app.c — Integrated I2C slave with application
    
    Features:
        Receive 32-bit value: Master can write 4 bytes (32-bit value) to slave
        Reply with 32-bit value: Master can read 4 bytes (32-bit value) from slave
        Interrupt-based: Uses HAL I2C interrupt callbacks
        Callback support: Optional callbacks for RX/TX events
        Automatic integration: TX value updated with latest pressure reading
    
    How it works:
    Master Write (Master → Slave):
        Master addresses slave (0x10) with write
        Master sends 4 bytes (32-bit value, little-endian)
        Slave receives and converts to uint32_t
        Value available via i2c_slave_get_received_value() or RX callback
    Master Read (Slave → Master):
        Master addresses slave (0x10) with read
        Slave sends latest pressure value (updated in app_main_loop())
        Master receives 4 bytes (32-bit value, little-endian)
    
    Current integration:
        TX value: Automatically set to latest pressure reading in app_main_loop()
        Initialization: Done in main_init_drivers() and main_init_app()
        Interrupts: Properly configured and enabled

*/

#include "i2c_slave.h"
#include "board_config.h"
#include "stm32l0xx_hal.h"

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================ */

static I2C_HandleTypeDef *i2c_slave_handle = NULL;
static bool i2c_slave_initialized = false;
static bool i2c_slave_started = false;

/* Receive buffer for 32-bit value (4 bytes) */
static uint8_t rx_buffer[4] = {0};
static uint32_t rx_value = 0;
static bool rx_value_ready = false;

/* Transmit buffer for 32-bit value (4 bytes) */
static uint8_t tx_buffer[4] = {0};
static uint32_t tx_value = 0;
static bool tx_value_set = false;

/* Callbacks */
static i2c_slave_rx_callback_t rx_callback = NULL;
static i2c_slave_tx_callback_t tx_callback = NULL;

/* State tracking */
static enum {
    I2C_SLAVE_STATE_IDLE,
    I2C_SLAVE_STATE_RX,
    I2C_SLAVE_STATE_TX
} i2c_slave_state = I2C_SLAVE_STATE_IDLE;

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief Convert 32-bit value to 4-byte array (little-endian)
 */
static void uint32_to_bytes(uint32_t value, uint8_t *bytes)
{
    bytes[0] = (uint8_t)(value & 0xFF);
    bytes[1] = (uint8_t)((value >> 8) & 0xFF);
    bytes[2] = (uint8_t)((value >> 16) & 0xFF);
    bytes[3] = (uint8_t)((value >> 24) & 0xFF);
}

/**
 * @brief Convert 4-byte array to 32-bit value (little-endian)
 */
static uint32_t bytes_to_uint32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0]) |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

bool i2c_slave_init(I2C_HandleTypeDef *hi2c, uint8_t slave_addr)
{
    if (hi2c == NULL) {
        return false;
    }
    
    i2c_slave_handle = hi2c;
    
    /* Verify the handle is configured for slave mode */
    if (i2c_slave_handle->Init.OwnAddress1 != (uint32_t)(slave_addr << 1)) {
        return false;  /* Address mismatch */
    }
    
    /* Initialize state */
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
    rx_value_ready = false;
    tx_value_set = false;
    rx_callback = NULL;
    tx_callback = NULL;
    
    i2c_slave_initialized = true;
    return true;
}

bool i2c_slave_start(void)
{
    if (!i2c_slave_initialized || i2c_slave_handle == NULL) {
        return false;
    }
    
    if (i2c_slave_started) {
        return true;  /* Already started */
    }
    
    /* Prepare receive buffer */
    rx_value_ready = false;
    
    /* Start listening for address match (receive mode) */
    HAL_StatusTypeDef status = HAL_I2C_EnableListen_IT(i2c_slave_handle);
    
    if (status == HAL_OK) {
        i2c_slave_started = true;
        i2c_slave_state = I2C_SLAVE_STATE_IDLE;
        return true;
    }
    
    return false;
}

bool i2c_slave_stop(void)
{
    if (!i2c_slave_started) {
        return true;  /* Already stopped */
    }
    
    HAL_I2C_DisableListen_IT(i2c_slave_handle);
    i2c_slave_started = false;
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
    
    return true;
}

void i2c_slave_register_rx_callback(i2c_slave_rx_callback_t callback)
{
    rx_callback = callback;
}

void i2c_slave_register_tx_callback(i2c_slave_tx_callback_t callback)
{
    tx_callback = callback;
}

bool i2c_slave_get_received_value(uint32_t *value)
{
    if (value == NULL) {
        return false;
    }
    
    if (rx_value_ready) {
        *value = rx_value;
        rx_value_ready = false;  /* Clear flag after reading */
        return true;
    }
    
    return false;
}

void i2c_slave_set_tx_value(uint32_t value)
{
    tx_value = value;
    tx_value_set = true;
    uint32_to_bytes(tx_value, tx_buffer);
}

void i2c_slave_irq_handler(void)
{
    if (i2c_slave_handle == NULL) {
        return;
    }
    
    /* Let HAL process the interrupt */
    HAL_I2C_EV_IRQHandler(i2c_slave_handle);
    HAL_I2C_ER_IRQHandler(i2c_slave_handle);
}

/* ============================================================================
 * HAL CALLBACKS (Called by HAL from interrupt context)
 * ============================================================================ */

/**
 * @brief I2C slave address match callback
 * 
 * Called when master addresses this slave device.
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    (void)AddrMatchCode;  /* Unused */
    
    if (hi2c != i2c_slave_handle) {
        return;  /* Not our I2C peripheral */
    }
    
    if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
        /* Master wants to write (send data to us) */
        i2c_slave_state = I2C_SLAVE_STATE_RX;
        
        /* Prepare to receive 4 bytes */
        rx_value_ready = false;
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_buffer, 4, I2C_FIRST_AND_LAST_FRAME);
    }
    else if (TransferDirection == I2C_DIRECTION_RECEIVE) {
        /* Master wants to read (get data from us) */
        i2c_slave_state = I2C_SLAVE_STATE_TX;
        
        /* Get value to transmit */
        if (tx_callback != NULL) {
            tx_value = tx_callback();
            tx_value_set = true;
        }
        
        if (tx_value_set) {
            uint32_to_bytes(tx_value, tx_buffer);
        } else {
            /* Default: send zeros if no value set */
            tx_buffer[0] = 0;
            tx_buffer[1] = 0;
            tx_buffer[2] = 0;
            tx_buffer[3] = 0;
        }
        
        /* Prepare to transmit 4 bytes */
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, tx_buffer, 4, I2C_FIRST_AND_LAST_FRAME);
    }
}

/**
 * @brief I2C slave receive complete callback
 * 
 * Called when data reception is complete.
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != i2c_slave_handle) {
        return;
    }
    
    /* Convert received bytes to 32-bit value */
    rx_value = bytes_to_uint32(rx_buffer);
    rx_value_ready = true;
    
    /* Call user callback if registered */
    if (rx_callback != NULL) {
        rx_callback(rx_value);
    }
    
    /* Re-enable listening for next transaction */
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
    HAL_I2C_EnableListen_IT(hi2c);
}

/**
 * @brief I2C slave transmit complete callback
 * 
 * Called when data transmission is complete.
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != i2c_slave_handle) {
        return;
    }
    
    /* Re-enable listening for next transaction */
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
    HAL_I2C_EnableListen_IT(hi2c);
}

/**
 * @brief I2C error callback
 * 
 * Called when an I2C error occurs.
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != i2c_slave_handle) {
        return;
    }
    
    /* Clear error flags and re-enable listening */
    uint32_t error = HAL_I2C_GetError(hi2c);
    (void)error;  /* Can be used for error handling if needed */
    
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
    
    /* Re-enable listening */
    if (i2c_slave_started) {
        HAL_I2C_EnableListen_IT(hi2c);
    }
}

/**
 * @brief I2C listen complete callback
 * 
 * Called when listen mode completes (master stop condition).
 */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != i2c_slave_handle) {
        return;
    }
    
    /* Re-enable listening for next transaction */
    if (i2c_slave_started) {
        HAL_I2C_EnableListen_IT(hi2c);
    }
}

