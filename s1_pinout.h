#ifndef __S1_PINOUT_H
#define __S1_PINOUT_H

/**
 * @file  s1_pinout.h
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

#endif