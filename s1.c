/**
 * @file  s1.c
 * @brief S1 Module Core Functions
 *        
 *        Various functions to setup and configure the
 *        S1 Module. To access these functions, use the
 *        s1.h header file.
 * 
 * @attention (c) 2021 Silicon Witchery 
 *            (info@siliconwitchery.com)
 *
 *        Licensed under a Creative Commons Attribution 
 *        4.0 International License. This code is provided
 *        as-is and no warranty is given.
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "app_error.h"

#include "nrf_gpio.h"
#include "nrfx_saadc.h"
#include "nrfx_spim.h"
#include "nrfx_twim.h"
#include "nrf52811.h"
#include "s1.h"

/**
 * @brief Pinout definition for the nRF52811 chip
 *        on the S1 Module.
 * 
 *        This isn't the pinout of the module itself,
 *        but rather the internal connections between
 *        the nRF, PMIC, flash IC and FPGA.
 */
#define ADC1_PIN            NRF_SAADC_INPUT_AIN2
#define ADC2_PIN            NRF_SAADC_INPUT_AIN3
#define PMIC_AMUX_PIN       NRF_SAADC_INPUT_AIN1

#define SPI_SI_PIN          NRF_GPIO_PIN_MAP(0, 8)
#define SPI_SO_PIN          NRF_GPIO_PIN_MAP(0, 11)
#define SPI_CS_PIN          NRF_GPIO_PIN_MAP(0, 12)
#define SPI_CLK_PIN         NRF_GPIO_PIN_MAP(0, 15)
#define FPGA_RESET_PIN      NRF_GPIO_PIN_MAP(0, 20)
#define FPGA_DONE_PIN       NRF_GPIO_PIN_MAP(0, 16)

#define PMIC_SDA_PIN        NRF_GPIO_PIN_MAP(0, 14)
#define PMIC_SCL_PIN        NRF_GPIO_PIN_MAP(0, 17)
#define PMIC_ADDRESS        0x48