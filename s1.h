/**
 * @file  s1.h
 *
 * @brief S1 Module Core API
 *
 *        Use these functions and macros to configure the S1 Module. All APIs
 *        are documented here, and organized by category.
 *
 * @attention Copyright 2022 Silicon Witchery AB
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

#include <string.h>
#include "SEGGER_RTT.h"
#include "sdk_config.h"

/**
 * @brief Release version of this SDK
 */
#define __S1_SDK_VERSION__ "1.0"

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
    S1_INIT_ERROR,
    S1_PMIC_COMMUNICATION_ERROR,
    S1_PMIC_INVALID_VALUE,
    S1_PMIC_VAUX_NOT_ENABLED,
    S1_PMIC_VAUX_TOO_LOW,
    S1_PMIC_VAUX_TOO_HIGH,
    S1_PMIC_VFPGA_NOT_ENABLED,
    S1_FLASH_FPGA_COMMUNICATION_ERROR,
    S1_FLASH_ERROR,
} s1_error_t;

/**
 * @brief S1 first initialisation. Sets up communication between the internal
 *        ICs and configures the GPIO required for configuring the FPGA. Always
 *        run this at the start of your application. It doesn't change any PMIC
 *        configuration previously called, so this can be called from a deep
 *        sleep state in order to reconfigure the IO without changing power
 *        settings.
 *
 * @return S1_SUCCESS if okay,
 *         S1_INIT_ERROR if GPIOTE or I2C resources are already used,
 *         S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_init(void);

/*******************************************************
 * Power related functions
 *******************************************************/

/**
 * @brief Gets the current settings of the Li charger.
 *
 * @param voltage: A pointer to where the currently configured set voltage of
 *                 the charger should be stored.
 *
 * @param current: A pointer to where the currently configured constant current
 *                 setting of the charger should be stored.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pmic_get_chg(float *voltage, float *current);

/**
 * @brief Sets the charger target voltage and constant current values. Note that
 *        setting incorrect values may damage a connected battery and cause fire
 *        or burns. It's important to check that this configuration matches the
 *        real measured values once a battery is connected.
 *
 * @param voltage: The battery max voltage. For Li-Po, this is typically 4.2V.
 *                 Can be set between 3.6V and 4.6V in 0.025mV steps.
 *
 * @param current: The charging current limit. This should typically be set
 *                 to no more than 1 x the mAh capacity of the battery. Can be
 *                 set between 7.5mA and 300mA in 7.5mA steps.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pmic_set_chg(float voltage, float current);

/**
 * @brief Gets the current Vaux set voltage.
 *
 * @param voltage: A pointer to where the ready voltage should be stored.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pmic_get_vaux(float *voltage);

/**
 * @brief Set Vaux voltage. Can be set from 0.8V to 5.5V in 50mV steps. Will
 *        return false if Vio is set to Load Switch mode, and Vaux is set to a
 *        value greater than 3.46V. This is to protect the FPGA.
 *
 * @param voltage: Voltage level. Automatically rounded to the nearest 50mV. A
 *                 value of 0V shuts down the rail.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_INVALID_VALUE if voltage is not a valid range,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pmic_set_vaux(float voltage);

/**
 * @brief Gets the current Vio set voltage, or the mode of the load switch.
 *
 * @param voltage: A pointer to where the read voltage should be stored. If
 *                 lsw_mode is true, a voltage of 0 signifies the load switch is
 *                 off, and a value of 1 signifies the load switch is on.
 *
 * @param lsw_mode: A pointer to where the load swich mode should be stored.
 *                  True if load switch mode is active, false if LDO mode is
 *                  active.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_VAUX_NOT_ENABLED if Vio is not powered from Vaux,
 *          S1_PMIC_VAUX_TOO_LOW if Vaux is too low for the LDO to regulate,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pmic_get_vio(float *voltage, bool *lsw_mode);

/**
 * @brief Set Vio voltage. Can be set from 0.8V to 3.46V in 25mV steps.
 *
 * @param voltage: Voltage level. Automatically rounded to the nearest 25mV. A
 *                 value of 0V shuts down the rail. If in load switch mode, a
 *                 value of 0 shuts off the load switch, a value greater than 0
 *                 turns on the load switch.
 *
 * @param lsw_mode: If the mode of Vio should be load switch, or LDO. If true,
 *                  the load switch passes through the Vaux voltage, otherwise
 *                  Vio is regulated by an LDO.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_VAUX_NOT_ENABLED if Vio is not powered from Vaux,
 *          S1_PMIC_VFPGA_NOT_ENABLED if Vfpga needs to be activated first,
 *          S1_PMIC_VAUX_TOO_HIGH if Vaux is too high for the load switch mode,
 *          S1_PMIC_INVALID_VALUE if voltage is not a valid range,
 *          S1_PMIC_VAUX_TOO_LOW if Vaux is too low for Vio to properly regulate,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pmic_set_vio(float voltage, bool lsw_mode);

/**
 * @brief Gets the enable state of the FPGA core voltage.
 *
 * @param enable: A pointer to where the enable value should be stored.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pimc_get_vfpga(bool *enable);

/**
 * @brief Sets the enable state of the FPGA core voltage.
 *
 * @param enable: If true, the FPGA is powered, if false the FPGA is shut down.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_PMIC_COMMUNICATION_ERROR if the PMIC did not respond.
 */
s1_error_t s1_pimc_set_vfpga(bool enable);

/*******************************************************
 * Flash related functions
 *******************************************************/

/**
 * @brief Wakes up the flash if it's asleep.
 *
 * @return S1_SUCCESS if okay,
 *         S1_FLASH_ERROR if the flash IC did not respond as expected.
 */
s1_error_t s1_flash_wakeup(void);

/**
 * @brief Fully erases the flash chip.
 */
void s1_flash_erase_all(void);

/**
 * @brief Checks if the flash is currently busy with an erase or write operation.
 *
 * @return True if busy,
 *         False if idle.
 */
bool s1_flash_is_busy(void);

/**
 * @brief Flashes a page to the flash at a given offset.
 *
 * @param offset: Page offset to flash.
 *
 * @param image: Pointer to the start of the binary you wish to flash.
 */
void s1_flash_page_from_image(uint32_t offset,
                              unsigned char *image);

/**
 * @brief Performs a transfer on the SPI bus to the flash IC.
 *
 * @param tx_buffer: A pointer to where the transmit data is stored.
 *
 * @param tx_len: Length of the transmit data buffer in bytes. i.e how many
 *                bytes to write.
 *
 * @param rx_buffer: A pointer to to where the receive data will be stored.
 *
 * @param rx_len: Length of the receive buffer in bytes. i.e how many bytes to
 *                read.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_FLASH_FPGA_COMMUNICATION_ERROR if spi bus is busy, or the buffers
 *          are not within the ram region. i.e not writable.
 */
s1_error_t flash_tx_rx(uint8_t *tx_buffer, size_t tx_len,
                       uint8_t *rx_buffer, size_t rx_len);

/*******************************************************
 * FPGA related functions
 *******************************************************/

/**
 * @brief Puts the FPGA into reset. It's recommended to wait 200uS before a
 *        subsequent flash or fpga operation.
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
 *
 * @returns True if booted,
 *          False if reset, or the image could not boot.
 */
bool s1_fpga_is_booted(void);

/**
 * @brief Performs a transfer on the SPI bus to the FPGA.
 *
 * @param tx_buffer: A pointer to where the transmit data is stored.
 *
 * @param tx_len: Length of the transmit data buffer in bytes. i.e how many
 *                bytes to write.
 *
 * @param rx_buffer: A pointer to to where the receive data will be stored.
 *
 * @param rx_len: Length of the receive buffer in bytes. i.e how many bytes to
 *                read.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_FLASH_FPGA_COMMUNICATION_ERROR if spi bus is busy, or the buffers
 *          are not within the ram region. i.e not writable.
 */
s1_error_t fpga_tx_rx(uint8_t *tx_buffer, size_t tx_len,
                      uint8_t *rx_buffer, size_t rx_len);

/*******************************************************
 * RTT based logging macros
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