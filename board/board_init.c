/**
 * @file board_init.c
 * @brief Board initialization implementation for STM32L072
 * 
 * This file contains platform-specific initialization code using STM32 HAL/LL.
 * When retargeting to another MCU, this file should be replaced with
 * the appropriate HAL/LL calls for that platform.
 */

#include "board_init.h"
#include "board_config.h"

/* STM32 HAL includes - adjust paths based on your STM32Cube structure */
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_rcc.h"
#include "stm32l0xx_hal_gpio.h"

/* External HAL handle - should be defined in hal_config.h or main */
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim2;

/* System clock frequency (updated by clock init) */
static uint32_t sysclk_freq = BOARD_SYSCLK_FREQ_HZ;

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief Configure GPIO pin for I2C alternate function
 */
static void board_config_i2c_pin(GPIO_TypeDef *port, uint32_t pin, uint32_t af)
{
    GPIO_InitTypeDef gpio_init = {0};
    
    gpio_init.Pin = pin;
    gpio_init.Mode = GPIO_MODE_AF_OD;
    gpio_init.Speed = GPIO_SPEED_FREQ_MEDIUM;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Alternate = af;
    
    HAL_GPIO_Init(port, &gpio_init);
}

/**
 * @brief Configure GPIO pin for DAC analog output
 */
static void board_config_dac_pin(GPIO_TypeDef *port, uint32_t pin)
{
    GPIO_InitTypeDef gpio_init = {0};
    
    gpio_init.Pin = pin;
    gpio_init.Mode = GPIO_MODE_ANALOG;
    gpio_init.Pull = GPIO_NOPULL;
    
    HAL_GPIO_Init(port, &gpio_init);
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

bool board_init_clock(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Enable HSI oscillator */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return false;
    }
    
    /* Configure system clock to use HSI */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | 
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        return false;
    }
    
    /* Update system clock frequency */
    sysclk_freq = BOARD_HSI_FREQ_HZ;
    
    /* Update SystemCoreClock variable */
    SystemCoreClockUpdate();
    
    return true;
}

bool board_init_gpio(void)
{
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /* Configure I2C1 pins (PA9, PA10) for I2C slave */
    board_config_i2c_pin(BOARD_I2C1_SCL_PORT, 
                         (1UL << BOARD_I2C1_SCL_PIN), 
                         BOARD_I2C1_SCL_AF);
    board_config_i2c_pin(BOARD_I2C1_SDA_PORT, 
                         (1UL << BOARD_I2C1_SDA_PIN), 
                         BOARD_I2C1_SDA_AF);
    
    /* Configure I2C2 pins (PB10, PB11) for pressure sensor */
    board_config_i2c_pin(BOARD_I2C2_SCL_PORT, 
                         (1UL << BOARD_I2C2_SCL_PIN), 
                         BOARD_I2C2_SCL_AF);
    board_config_i2c_pin(BOARD_I2C2_SDA_PORT, 
                         (1UL << BOARD_I2C2_SDA_PIN), 
                         BOARD_I2C2_SDA_AF);
    
    /* Configure DAC pins (PA4, PA5) for analog output */
    board_config_dac_pin(BOARD_DAC1_OUT1_PORT, (1UL << BOARD_DAC1_OUT1_PIN));
    board_config_dac_pin(BOARD_DAC1_OUT2_PORT, (1UL << BOARD_DAC1_OUT2_PIN));
    
    return true;
}

bool board_init(void)
{
    /* Initialize system clock */
    if (!board_init_clock()) {
        return false;
    }
    
    /* Initialize GPIO pins */
    if (!board_init_gpio()) {
        return false;
    }
    
    /* Enable peripheral clocks */
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    return true;
}

uint32_t board_get_sysclk_freq(void)
{
    return sysclk_freq;
}

uint32_t board_get_apb1_freq(void)
{
    return sysclk_freq;  /* APB1 = SYSCLK when prescaler is 1:1 */
}

uint32_t board_get_apb2_freq(void)
{
    return sysclk_freq;  /* APB2 = SYSCLK when prescaler is 1:1 */
}

void board_delay_ms(uint32_t ms)
{
    /* Simple blocking delay - should be replaced with HAL_Delay or 
     * a more accurate implementation based on system clock */
    volatile uint32_t count = ms * (sysclk_freq / 1000);
    while (count--) {
        __NOP();
    }
}

void board_delay_us(uint32_t us)
{
    /* Simple blocking delay - should be replaced with a more accurate 
     * implementation based on system clock */
    volatile uint32_t count = us * (sysclk_freq / 1000000);
    while (count--) {
        __NOP();
    }
}

