/**
 * @file hal_config.c
 * @brief HAL peripheral configuration implementation
 * 
 * This file implements HAL peripheral initialization for STM32L0.
 * I2C2 initialization for pressure sensor (100kHz)
 * I2C1 initialization for I2C slave
 * TIM2 initialization for 2ms interrupt (500Hz)
 * DAC1 initialization
 * HAL MSP callbacks for GPIO configuration
 * Uses board_config.h macros throughout
 * 
 * @return true if initialization successful, false otherwise
 */
bool hal_i2c2_init(void)
{
    hi2c2.Instance = BOARD_I2C2_PERIPH;
    hi2c2.Init.Timing = 0x00303D5B;  /* 100kHz I2C speed for STM32L0 @ 16MHz */
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
 */

#include "hal_config.h"
#include "board_config.h"
#include "board_init.h"

/* HAL peripheral handles - defined in main.c */
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim2;

/* ============================================================================
 * I2C2 Configuration (Pressure Sensor)
 * ============================================================================ */

bool hal_i2c2_init(void)
{
    hi2c2.Instance = BOARD_I2C2_PERIPH;
    hi2c2.Init.Timing = 0x00303D5B;  /* 100kHz I2C speed for STM32L0 @ 16MHz */
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        return false;
    }
    
    /* Configure I2C analog filter */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        return false;
    }
    
    return true;
}

/* ============================================================================
 * I2C1 Configuration (I2C Slave)
 * ============================================================================ */

bool hal_i2c1_init(void)
{
    hi2c1.Instance = BOARD_I2C1_PERIPH;
    hi2c1.Init.Timing = 0x00303D5B;  /* 100kHz I2C speed for STM32L0 @ 16MHz */
    hi2c1.Init.OwnAddress1 = (BOARD_I2C1_SLAVE_ADDR << 1);  /* 7-bit address shifted */
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        return false;
    }
    
    /* Configure I2C analog filter */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        return false;
    }
    
    return true;
}

/* ============================================================================
 * TIM2 Configuration (2ms Sampling Timer)
 * ============================================================================ */
/*
    System clock: 16 MHz HSI
    TIM2 prescaler: 1600 (timer clock = 10 kHz)
    TIM2 period: 20 counts (2ms period = 500 Hz)
    Interrupt triggers every 2ms to advance the sensor reading state machine
*/
bool hal_tim2_init(void)
{
    uint32_t apb1_freq = board_get_apb1_freq();
    /* For STM32L0: If APB prescaler = 1, timer clock = APB clock (no multiplier)
     *              If APB prescaler > 1, timer clock = APB clock * 2
     * Since we use prescaler = 1, timer clock = APB1 clock = 16MHz */
    uint32_t timer_freq = apb1_freq;  /* 16 MHz */
    
    /* Target: 2ms period = 0.002s, so frequency = 500 Hz
     * Timer period in counts = timer_freq / target_freq
     * For 16MHz and 500Hz: period = 16,000,000 / 500 = 32,000 counts
     * Use prescaler to reduce timer clock for easier calculation */
    uint32_t prescaler = 1600 - 1;  /* Divide 16MHz by 1600 = 10kHz timer clock */
    uint32_t period = 20 - 1;        /* 20 counts at 10kHz = 2ms period */
    
    htim2.Instance = BOARD_TIM2_PERIPH;
    htim2.Init.Prescaler = prescaler;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = period;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        return false;
    }
    
    /* Configure TIM2 interrupt */
    HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    
    return true;
}

bool hal_tim2_start(void)
{
    return (HAL_TIM_Base_Start_IT(&htim2) == HAL_OK);
}

bool hal_tim2_stop(void)
{
    return (HAL_TIM_Base_Stop_IT(&htim2) == HAL_OK);
}

/* ============================================================================
 * DAC1 Configuration
 * ============================================================================ */

bool hal_dac1_init(void)
{
    DAC_ChannelConfTypeDef sConfig = {0};
    
    hdac1.Instance = BOARD_DAC_PERIPH;
    if (HAL_DAC_Init(&hdac1) != HAL_OK) {
        return false;
    }
    
    /* Configure DAC channel 1 */
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, BOARD_DAC1_OUT1_CHANNEL) != HAL_OK) {
        return false;
    }
    
    /* Configure DAC channel 2 */
    if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, BOARD_DAC1_OUT2_CHANNEL) != HAL_OK) {
        return false;
    }
    
    return true;
}

/* ============================================================================
 * HAL Callbacks (Required by HAL)
 * ============================================================================ */

/**
 * @brief I2C MSP Initialization callback
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (hi2c->Instance == BOARD_I2C1_PERIPH) {
        /* I2C1 clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        /* I2C1 GPIO Configuration: PA9 -> SCL, PA10 -> SDA */
        GPIO_InitStruct.Pin = (1UL << BOARD_I2C1_SCL_PIN) | (1UL << BOARD_I2C1_SDA_PIN);
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = BOARD_I2C1_SCL_AF;
        HAL_GPIO_Init(BOARD_I2C1_SCL_PORT, &GPIO_InitStruct);
    }
    else if (hi2c->Instance == BOARD_I2C2_PERIPH) {
        /* I2C2 clock enable */
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        /* I2C2 GPIO Configuration: PB10 -> SCL, PB11 -> SDA */
        GPIO_InitStruct.Pin = (1UL << BOARD_I2C2_SCL_PIN) | (1UL << BOARD_I2C2_SDA_PIN);
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = BOARD_I2C2_SCL_AF;
        HAL_GPIO_Init(BOARD_I2C2_SCL_PORT, &GPIO_InitStruct);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    if (hi2c->Instance == BOARD_I2C1_PERIPH) {
        __HAL_RCC_I2C1_CLK_DISABLE();
        HAL_GPIO_DeInit(BOARD_I2C1_SCL_PORT, (1UL << BOARD_I2C1_SCL_PIN) | (1UL << BOARD_I2C1_SDA_PIN));
    }
    else if (hi2c->Instance == BOARD_I2C2_PERIPH) {
        __HAL_RCC_I2C2_CLK_DISABLE();
        HAL_GPIO_DeInit(BOARD_I2C2_SCL_PORT, (1UL << BOARD_I2C2_SCL_PIN) | (1UL << BOARD_I2C2_SDA_PIN));
    }
}

/**
 * @brief TIM MSP Initialization callback
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if (htim_base->Instance == BOARD_TIM2_PERIPH) {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
    if (htim_base->Instance == BOARD_TIM2_PERIPH) {
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
}

/**
 * @brief DAC MSP Initialization callback
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (hdac->Instance == BOARD_DAC_PERIPH) {
        __HAL_RCC_DAC1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        /* DAC GPIO Configuration: PA4 -> OUT1, PA5 -> OUT2 */
        GPIO_InitStruct.Pin = (1UL << BOARD_DAC1_OUT1_PIN) | (1UL << BOARD_DAC1_OUT2_PIN);
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(BOARD_DAC1_OUT1_PORT, &GPIO_InitStruct);
    }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
    if (hdac->Instance == BOARD_DAC_PERIPH) {
        __HAL_RCC_DAC1_CLK_DISABLE();
        HAL_GPIO_DeInit(BOARD_DAC1_OUT1_PORT, (1UL << BOARD_DAC1_OUT1_PIN) | (1UL << BOARD_DAC1_OUT2_PIN));
    }
}

