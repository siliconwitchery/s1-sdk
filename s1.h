/**
 * @file  s1.h
 * @brief S1 Module Core API
 *        
 *        Use these functions and macros to configure the S1 Module. All APIs 
 *        are documented here, and organized by category.
 * 
 * @attention Copyright 2021 Silicon Witchery AB
 *
 * Permission to use, copy, modify, and/or distribute this software for any 
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, 
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _S1_H_
#define _S1_H_

#include "SEGGER_RTT.h"
#include "sdk_config.h"

/**
 * @brief Release version of this SDK
 */
#define __S1_SDK_VERSION__ "0.3"

/**
 * @brief Pinout definitions for the nRF52811 chip on the S1 Module. This isn't
 *        the pinout of the module itself, but rather the internal connections. 
 *        Use them if you want to access the ADC, SPI or Flash 
 */
#define ADC1_PIN NRF_SAADC_INPUT_AIN2
#define ADC1_PIN_AS_GPIO NRF_GPIO_PIN_MAP(0, 4)
#define ADC2_PIN NRF_SAADC_INPUT_AIN3
#define ADC2_PIN_AS_GPIO NRF_GPIO_PIN_MAP(0, 5)
#define SPI_SI_PIN NRF_GPIO_PIN_MAP(0, 8)
#define SPI_SO_PIN NRF_GPIO_PIN_MAP(0, 11)
#define SPI_CS_PIN NRF_GPIO_PIN_MAP(0, 12)
#define SPI_CLK_PIN NRF_GPIO_PIN_MAP(0, 15)
#define FPGA_RESET_PIN NRF_GPIO_PIN_MAP(0, 20)
#define FPGA_DONE_PIN NRF_GPIO_PIN_MAP(0, 16)

/**
 * @brief Possible error conditions for the various configuration functions.
 */
typedef enum
{
    S1_SUCCESS = 0,
    S1_PMIC_ERROR,
    S1_FLASH_ERROR,
    S1_INVALID_SETTING,
} s1_error_t;

/**
 * @brief S1 first initialisation. Sets up communication between the internal
 *        ICs and configures the GPIO required for configuring the FPGA. Always
 *        run this at the start of your application. It doesn't change any PMIC
 *        configuration previously called, so this can be called from a deep
 *        sleep state in order to reconfigure the IO without changing power
 *        settings.
 * 
 * @return S1_SUCCESS if okay. Error otherwise.
 */
s1_error_t s1_init(void);

/*******************************************************
 * Power related APIs
 *******************************************************/

/**
 * @brief Set Vaux voltage. Can be set from 0.8V to 5.5V in 50mV steps. Will 
 *        return false if Vio is set to Load Switch mode, and Vaux is set to a
 *        value greater than 3.46V. This is to protect the FPGA.
 * 
 * @param voltage: Voltage level. Automatically rounded to the nearest 50mV. A 
 *                 value of 0V shuts down the rail. 
 * 
 * @returns S1_SUCCESS if okay. Error otherwise.
 */
s1_error_t s1_pmic_set_vaux(float voltage);

/**
 * @brief Set Vio voltage. Can be set from 0.8V to 3.46V in 25mV steps. 
 * 
 * @param voltage: Voltage level. Automatically rounded to the nearest 25mV. A 
 *                 value of 0V shuts down the rail.
 * 
 * @return S1_SUCCESS if okay. Error otherwise.
 */
s1_error_t s1_pmic_set_vio(float voltage);

/**
 * @brief Switches Vio between Low Dropout Regulator mode and Load Switch mode.
 *        In Load switch mode, Vio outputs the same voltage as Vaux. Vio cannot 
 *        be set to load switch mode if Vaux greater than 3.46V. This is to 
 *        protect the FPGA.
 * 
 * @param enable: True enables load switch mode, false switches back to 
 *                regulated mode.
 * 
 * @return S1_SUCCESS if okay. Error otherwise.
 */
s1_error_t s1_pmic_set_vio_lsw(bool enable);

/**
 * @brief Set Vio voltage. Can be set from 1.7V to 2.0V in 50mV steps.
 * 
 * @param voltage: Voltage level. Automatically rounded to the nearest 50mV.
 * 
 * @returns S1_SUCCESS if okay. Error otherwise.
 */
s1_error_t s1_pmic_set_vadc(float voltage);

/**
 * @brief Enable FPGA core and FPGA PLL voltages to 1.2V Disabling also shuts 
 *        down Vio.
 *
 * @param enable: Enable the core voltage.
 */
void s1_pimc_fpga_vcore(bool enable);

/*******************************************************
 * Flash related APIs
 *******************************************************/

/**
 * @brief Wakes up the flash if it's asleep.
 * 
 * @return S1_SUCCESS if okay, or S1_FLASH_ERROR if the chip isn't responding 
 *         correctly.
 */
s1_error_t s1_flash_wakeup(void);

/**
 * @brief Fully erases the flash chip.
 */
void s1_flash_erase_all(void);

/**
 * @brief Check if the flash is busy.
 * 
 * @return true if busy.
 */
bool s1_flash_is_busy(void);

/**
 * @brief Flashes a page at offset address to the flash.
 * 
 * @param offset: Page offset to flash.
 * 
 * @param image: Pointer to the start of the binary you wish to flash.
 * 
 * @return true if busy.
 */
s1_error_t s1_flash_page_from_image(uint32_t offset,
                                    unsigned char *image);

/*******************************************************
 * FPGA related APIs
 *******************************************************/

/**
 * @brief Puts the FPGA into reset. Reccomended to wait 200uS before a 
 *        subsequent flash/fpga operation.
 */
void s1_fpga_hold_reset(void);

/**
 * @brief Passes SPI control to the flash, and releases the FPGA reset to allow 
 *        it to boot. This function must be called before communication can be
 *        made between the nRF and the FPGA directly.
 */
void s1_fpga_boot(void);

/**
 * @brief Checks if the CDONE pin on the FPGA has gone high which tells us the
 *        device has correctly configured. Note that this pin may not activate
 *        if the CDONE pin is controlled by the FPGA application to be used for
 *        something else.
 */
bool s1_fpga_is_booted(void);

/*******************************************************
 * RTT based logging Macros
 *******************************************************/

/**
 * @brief Clears the terminal screen of any previous logs.
 */
#define LOG_CLEAR() SEGGER_RTT_printf(0, RTT_CTRL_CLEAR "\r");

/**
 * @brief A println style logging macro. You can use all the standard printf 
 *        flags. It uses LOG_RAW()
 */

#define LOG(format, ...) LOG_RAW("\r\n" format, ##__VA_ARGS__)

/**
 * @brief Logging macro which outputs printf style logs over a JLink debugger. 
 *        To view the terminal, run JLinkRTTViewer.exe. If a debug session is
 *        already active, JLinkRTTClient.exe should be used. Read more about RTT
 *        here: https://wiki.segger.com/RTT
 *         
 * @param format: printf style format string.
 * 
 * @param ...: Variadic argument list for printf data.
 */
#define LOG_RAW(format, ...)                                                                                  \
    do                                                                                                        \
    {                                                                                                         \
        char _debug_log_buffer[SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - 1] = "";                                    \
        snprintf(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - 1, format, ##__VA_ARGS__);             \
        SEGGER_RTT_Write(0, _debug_log_buffer, strnlen(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP)); \
    } while (0)

#endif