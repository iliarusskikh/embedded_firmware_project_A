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

/* STM32 HAL/LL includes - adjust paths based on your STM32Cube structure */
#include "stm32l0xx_hal.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_bus.h"

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
    LL_GPIO_InitTypeDef gpio_init = {0};
    
    gpio_init.Pin = pin;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_init.Pull = LL_GPIO_PULL_UP;
    gpio_init.Alternate = af;
    
    LL_GPIO_Init(port, &gpio_init);
}

/**
 * @brief Configure GPIO pin for DAC analog output
 */
static void board_config_dac_pin(GPIO_TypeDef *port, uint32_t pin)
{
    LL_GPIO_InitTypeDef gpio_init = {0};
    
    gpio_init.Pin = pin;
    gpio_init.Mode = LL_GPIO_MODE_ANALOG;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    
    LL_GPIO_Init(port, &gpio_init);
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

bool board_init_clock(void)
{
    /* Enable HSI oscillator */
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() == 0) {
        /* Wait for HSI to be ready */
    }
    
    /* Configure system clock to use HSI */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {
        /* Wait for clock switch */
    }
    
    /* Configure AHB, APB1, APB2 prescalers (1:1 for simplicity) */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    
    /* Update system clock frequency */
    sysclk_freq = BOARD_HSI_FREQ_HZ;
    
    /* Update SystemCoreClock variable (if using HAL) */
    SystemCoreClockUpdate();
    
    return true;
}

bool board_init_gpio(void)
{
    /* Enable GPIO clocks */
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    
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
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    
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

