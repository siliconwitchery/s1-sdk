/**
 * @file  s1.c
 *
 * @brief S1 Module Core Functions
 *
 *        Various functions to setup and configure the S1 Module. To access
 *        these functions, use the s1.h header file.
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

#include <string.h>
#include <math.h>

#include "nrf_gpio.h"
#include "nrfx_gpiote.h"
#include "nrfx_saadc.h"
#include "nrfx_spim.h"
#include "nrfx_twim.h"
#include "nrf52811.h"
#include "s1.h"

/**
 * @brief Instance of the SPI driver to the Flash and FPGA
 */
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(0);

/**
 * @brief Instance of the I2C driver to the PMIC
 */
static const nrfx_twim_t i2c = NRFX_TWIM_INSTANCE(0);

/**
 * @brief Interrupt driven pending flag for when the FPGA_DONE_PIN goes high
 */
static bool fpga_done_flag_pending = false;

/**
 * @brief Definition of the ADC input pin for battery monitoring.
 */
#define PMIC_AMUX_PIN NRF_SAADC_INPUT_AIN1

/**
 * @brief Local function for reading a register of the PMIC. Should not be
 *        directly accessed, instead use the relevant s1_pmic_...() functions
 *        to read data.
 *
 * @param reg: Address of the 8bit register that should be read.
 *
 * @param data: Pointer to where the read data will be stored.
 *
 * @returns S1_SUCCESS if okay, or S1_PMIC_COMMUNICATION_ERROR if the PMIC did
 *          not respond.
 */
static s1_error_t pmic_read_reg(uint8_t reg, uint8_t *data)
{
    // Transfer descriptor configured for a 1 byte read, and 1 byte write
    nrfx_twim_xfer_desc_t i2c_xfer =
        NRFX_TWIM_XFER_DESC_TXRX(0x48, &reg, 1, data, 1);

    // Initiate the transfer
    nrfx_err_t err = nrfx_twim_xfer(&i2c, &i2c_xfer, 0);

    // If an error occurs, try again after 100us. This can be needed if the
    // PMIC is under load, and the power fluctuates
    if (err != NRFX_SUCCESS)
    {
        NRFX_DELAY_US(100);
        err = nrfx_twim_xfer(&i2c, &i2c_xfer, 0);

        // If another error occurs, return a communication error
        if (err != NRFX_SUCCESS)
        {
            return S1_PMIC_COMMUNICATION_ERROR;
        }
    }

    // If transfer was okay, return success
    return S1_SUCCESS;
}

/**
 * @brief Local function for writing a register of the PMIC. Should not be
 *        directly accessed, instead use the relevant s1_pmic_...() functions
 *        to write data.
 *
 * @param reg: Address of the 8bit register that should be written to.
 *
 * @param data: Value which should be written in the register.
 *
 * @returns S1_SUCCESS if okay, or S1_PMIC_COMMUNICATION_ERROR if the PMIC did
 *          not respond.
 */
static s1_error_t pmic_write_reg(uint8_t reg, uint8_t data)
{
    // Create a two byte buffer with the register and value
    uint8_t buffer[2] = {reg, data};

    // Transfer descriptor configured for a 2 byte write
    nrfx_twim_xfer_desc_t i2c_xfer =
        NRFX_TWIM_XFER_DESC_TX(0x48, buffer, 2);

    // Initiate the transfer
    nrfx_err_t err = nrfx_twim_xfer(&i2c, &i2c_xfer, 0);

    // If an error occurs, return a communication error
    if (err != NRFX_SUCCESS)
    {
        return S1_PMIC_COMMUNICATION_ERROR;
    }

    // If transfer was okay, return success
    return S1_SUCCESS;
}

/**
 * @brief Performs a transfer on the SPI bus to the flash or FPGA.
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
 * @param sel_fpga: If true, selects the FPGA. If false, selects the flash.
 *
 * @returns S1_SUCCESS if okay,
 *          S1_FLASH_FPGA_COMMUNICATION_ERROR if spi bus is busy, or the buffers
 *          are not within the ram region. i.e not writable.
 */
static s1_error_t spi_tx_rx(uint8_t *tx_buffer, size_t tx_len,
                            uint8_t *rx_buffer, size_t rx_len, bool sel_fpga)
{
    // SPI hardware configuration
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
    spi_config.mosi_pin = SPI_SO_PIN;
    spi_config.miso_pin = SPI_SI_PIN;
    spi_config.sck_pin = SPI_CLK_PIN;
    spi_config.ss_pin = SPI_CS_PIN;

    // If selecting the FPGA, invert the chip select line
    if (sel_fpga)
    {
        spi_config.ss_active_high = true;
    }

    // Initialise the SPI if it was not already
    nrfx_spim_init(&spi, &spi_config, NULL, NULL);

    // Transfer descriptor for how many bytes to read and write
    nrfx_spim_xfer_desc_t spi_xfer = NRFX_SPIM_XFER_TRX(tx_buffer, tx_len,
                                                        rx_buffer, rx_len);

    // Initiate the transfer
    nrfx_err_t err = nrfx_spim_xfer(&spi, &spi_xfer, 0);

    // If an error occurs, return a flash error
    if (err != NRFX_SUCCESS)
    {
        return S1_FLASH_FPGA_COMMUNICATION_ERROR;
    }

    // If transfer was okay, return success
    return S1_SUCCESS;
}

/**
 * @brief Interrupt routine for when the FPGA configuration is complete, and the
 *        CDONE pin goes high.
 *
 * @param pin: Pin that caused the trigger.
 *
 * @param action: The polarity of the interrupt trigger.
 */
static void fpga_done_pin_interrupt(nrfx_gpiote_pin_t pin,
                                    nrf_gpiote_polarity_t action)
{
    if (pin == FPGA_DONE_PIN && action == NRF_GPIOTE_POLARITY_LOTOHI)
    {
        fpga_done_flag_pending = true;
    }
}

s1_error_t s1_init(void)
{
    // Configure FPGA reset pin as an output. A low signal holds FPGA in reset
    nrf_gpio_cfg_output(FPGA_RESET_PIN);

    // Set up done pin as an interrupt. It goes high when the FPGA is loaded
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    config.pull = NRF_GPIO_PIN_PULLUP;

    // Start the GPIOTE driver if not already started
    nrfx_gpiote_init();

    // Add the pin as an input event
    nrfx_err_t err =
        nrfx_gpiote_in_init(FPGA_DONE_PIN, &config, fpga_done_pin_interrupt);

    // If an error occurs, return an initialisation error
    if (err != NRFX_SUCCESS)
    {
        return S1_INIT_ERROR;
    }

    // Enable the event
    nrfx_gpiote_in_event_enable(FPGA_DONE_PIN, true);

    // Configure the I2C
    nrfx_twim_config_t pmic_twi_config = NRFX_TWIM_DEFAULT_CONFIG;
    pmic_twi_config.scl = NRF_GPIO_PIN_MAP(0, 17);
    pmic_twi_config.sda = NRF_GPIO_PIN_MAP(0, 14);

    // Initialise the I2C driver
    err = nrfx_twim_init(&i2c, &pmic_twi_config, NULL, NULL);

    // If an error occurs, return an initialisation error
    if (err != NRFX_SUCCESS)
    {
        return S1_INIT_ERROR;
    }

    // Enable the bus
    nrfx_twim_enable(&i2c);

    // Check PMIC Chip ID
    uint8_t pmic_chip_id;
    s1_error_t s1_err = pmic_read_reg(0x14, &pmic_chip_id);

    // If an error occurs, return a PMIC communication error
    if (s1_err != S1_SUCCESS)
    {
        return S1_PMIC_COMMUNICATION_ERROR;
    }

    // If the chip ID is incorrect, also return a PMIC communication error
    if (pmic_chip_id != 0x7A)
    {
        return S1_PMIC_COMMUNICATION_ERROR;
    }

    // TODO setup analog pins here

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pmic_get_chg(float *voltage, float *current)
{
    uint8_t reg_value;

    // Read the charge voltage register
    s1_error_t err = pmic_read_reg(0x26, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Shift down to get only the top 6 bits
    reg_value = reg_value >> 2;

    // Convert the register value to a voltage
    *voltage = (reg_value * 0.025f) + 3.6f;

    // Read the charge current register
    err = pmic_read_reg(0x24, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Shift down to get only the top 6 bits
    reg_value = reg_value >> 2;

    // Convert the register value to a current (mA)
    *current = (reg_value * 7.5f) + 7.5f;

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pmic_set_chg(float voltage, float current)
{
    // Check if voltage is a valid range
    if (voltage < 3.6f || voltage > 4.6f)
    {
        return S1_PMIC_INVALID_VALUE;
    }

    // Check if the current is a valid range
    if (current < 7.5f || current > 300.0f)
    {
        return S1_PMIC_INVALID_VALUE;
    }

    // Set the charging voltage (shifted to be in the top 6 bits of the register)
    uint8_t voltage_setting = (uint8_t)round((voltage - 3.6f) / 0.025f) << 2;

    // Apply the voltage, and ensure charging is allowed
    s1_error_t err = pmic_write_reg(0x26, voltage_setting | 0b00);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Set the charging current (shifted to be in the top 6 bits of the register)
    uint8_t current_setting = (uint8_t)round((current - 7.5f) / 7.5f) << 2;

    // Apply the current, and ensure a 3hr safety timer is set
    err = pmic_write_reg(0x24, current_setting | 0b01);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pmic_get_vaux(float *voltage)
{
    uint8_t reg_value;

    // Read the SBB2 register
    s1_error_t err = pmic_read_reg(0x2E, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Check if SBB2 is enabled
    bool vaux_en = (reg_value & 0b110) == 0b110;

    // If SBB2 is off, return 0V
    if (vaux_en == false)
    {
        *voltage = 0.0f;
        return S1_SUCCESS;
    }

    // Otherwise read the current set value
    err = pmic_read_reg(0x2D, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // We only need the bottom 7 bits
    reg_value = reg_value & 0x7F;

    // Convert the register value to a voltage
    *voltage = (reg_value * 0.05f) + 0.8f;

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pmic_set_vaux(float voltage)
{
    // If 0V, shutdown SBB2
    if (voltage == 0.0f)
    {
        // Write to the SBB2 en register
        s1_error_t err = pmic_write_reg(0x2E, 0x0C);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // Return success once complete
        return S1_SUCCESS;
    }

    // Disallow voltage settings outside of the normal range
    if (voltage < 0.8f || voltage > 5.5f)
    {
        return S1_PMIC_INVALID_VALUE;
    }

    // If voltage > than 3.45
    if (voltage > 3.45f)
    {
        uint8_t reg_value;

        // Read the mode of SBB2
        s1_error_t err = pmic_read_reg(0x39, &reg_value);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // LDO0 must not be in LSW mode, otherwise it'll blow up the FPGA
        if ((reg_value & 0x10) == 0x10)
        {
            return S1_PMIC_INVALID_VALUE;
        }
    }

    // Set the SBB2 target voltage
    s1_error_t err = pmic_write_reg(0x2D, (uint8_t)round((voltage - 0.8f) / 0.05f));

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Enable SBB2 as buck-boost, with 1A limit, and discharge resistor enabled
    err = pmic_write_reg(0x2E, 0x0E);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pmic_get_vio(float *voltage, bool *lsw_mode)
{
    uint8_t reg_value;

    // Read SBB2
    s1_error_t err = pmic_read_reg(0x2E, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // If SBB2 is disabled, notify the user
    if ((reg_value & 0b110) != 0b110)
    {
        return S1_PMIC_VAUX_NOT_ENABLED;
    }

    // Read the LSW/LDO mode
    err = pmic_read_reg(0x39, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // If in load switch mode
    if ((reg_value & 0x10) == 0x10)
    {
        // Set LSW mode pointer to true
        *lsw_mode = true;

        // Check if load switch mode is enabled
        if ((reg_value & 0b110) == 0b110)
        {
            // Set voltage to true
            *voltage = 1.0f;

            // Return success
            return S1_SUCCESS;
        }

        // Otherwise set the voltage to false
        *voltage = 0.0f;

        // Return success
        return S1_SUCCESS;
    }

    // If in LDO mode
    if ((reg_value & 0b110) == 0b110)
    {
        // Set LSW mode pointer to false
        *lsw_mode = false;

        // Read the LDO set voltage
        err = pmic_read_reg(0x38, &reg_value);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // Convert the register value into a voltage (mask 7 bits)
        *voltage = ((float)(reg_value & 0x7F) * 0.025f) + 0.8f;

        // Read the SBB2 (Vaux) register
        err = pmic_read_reg(0x2D, &reg_value);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // Convert the register value into a voltage
        float sbb2_voltage = ((float)(reg_value & 0x7F) * 0.05f) + 0.8f;

        // If sbb2 voltage is too low (including the 100mV dropout)
        if (sbb2_voltage < *voltage + 0.1f)
        {
            // Notify the user
            return S1_PMIC_VAUX_TOO_LOW;
        }

        // Return success once read
        return S1_SUCCESS;
    }

    // Otherwise LDO0 is 0V
    *voltage = 0.0f;

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pmic_set_vio(float voltage, bool lsw_mode)
{
    uint8_t reg_value;

    // Read SBB2
    s1_error_t err = pmic_read_reg(0x2E, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // If SBB2 is disabled, notify the user
    if ((reg_value & 0b110) != 0b110)
    {
        return S1_PMIC_VAUX_NOT_ENABLED;
    }

    // Read the SBB1 register
    err = pmic_read_reg(0x2C, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Check if fpga is powered off
    if ((reg_value & 0b10) == 0)
    {
        // Prevent configuration if FPGA core rail is off
        return S1_PMIC_VFPGA_NOT_ENABLED;
    }

    // If the lsw flag was provided
    if (lsw_mode)
    {
        // Read SBB2
        err = pmic_read_reg(0x2D, &reg_value);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // Ensure it is below the 3.45V limit of the FPGA IO
        // reg_value = (3.45 - 0.8) / 0.05 = 53
        if ((reg_value & 0x7F) > 53)
        {
            return S1_PMIC_VAUX_TOO_HIGH;
        }

        // If the voltage value is greater than 0V
        if (voltage > 0.0f)
        {
            // Turn on the regulator with LSW mode with discharge enabled
            err = pmic_write_reg(0x39, 0x1E);

            // If an error occurs, return it
            if (err != S1_SUCCESS)
            {
                return err;
            }

            // Return
            return S1_SUCCESS;
        }

        // Otherwise, turn off the regulator with LSW mode with discharge enabled
        err = pmic_write_reg(0x39, 0x1C);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // Return
        return S1_SUCCESS;
    }

    // If user requests 0V
    if (voltage == 0.0f)
    {
        // Turn off the regulator, ensuring LDO mode and discharge resistor is set
        err = pmic_write_reg(0x39, 0x0C);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // Return
        return S1_SUCCESS;
    }

    // Disallow voltage settings outside of the normal range
    if (voltage < 0.8f || voltage > 3.45f)
    {
        return S1_PMIC_INVALID_VALUE;
    }

    // Otherwise, check SBB2 (Vaux) set voltage
    err = pmic_read_reg(0x2D, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Convert the register value into a voltage (mask 7 bits)
    float sbb2_voltage = ((float)(reg_value & 0x7F) * 0.05f) + 0.8f;

    // If sbb2 voltage is too low (including the 100mV dropout)
    if (sbb2_voltage < voltage + 0.1f)
    {
        // Notify the user
        return S1_PMIC_VAUX_TOO_LOW;
    }

    // Set the output voltage
    err = pmic_write_reg(0x38, (uint8_t)round((voltage - 0.8f) / 0.025f));

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Turn on the regulator with LDO mode set, and discharge enabled
    err = pmic_write_reg(0x39, 0x0E);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pimc_get_vfpga(bool *enable)
{
    uint8_t reg_value;

    // Read the SBB1 register
    s1_error_t err = pmic_read_reg(0x2C, &reg_value);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Return the value. Second bit represents on or off
    *enable = reg_value & 0b10;

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_pimc_set_vfpga(bool enable)
{
    // Ensure SBB1 is 1.2V (TODO later we can do undervolting tricks)
    s1_error_t err = pmic_write_reg(0x2B, 0x08);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // If enable
    if (enable)
    {
        // Enable SBB1 as buck mode with 0.333A limit
        err = pmic_write_reg(0x2C, 0x7E);

        // If an error occurs, return it
        if (err != S1_SUCCESS)
        {
            return err;
        }

        // Return success once complete
        return S1_SUCCESS;
    }

    // Otherwise, first disable LDO0 (Vio) to avoid damaging the FPGA
    err = pmic_write_reg(0x39, 0x0C);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Finally, disable SBB1
    err = pmic_write_reg(0x2C, 0x7C);

    // If an error occurs, return it
    if (err != S1_SUCCESS)
    {
        return err;
    }

    // Return success once complete
    return S1_SUCCESS;
}

s1_error_t s1_flash_wakeup(void)
{
    // Wake up the flash
    uint8_t wake_seq[4] = {0xAB, 0, 0, 0};
    uint8_t wake_res[5] = {0};
    spi_tx_rx((uint8_t *)&wake_seq, 4, (uint8_t *)&wake_res, 5, false);
    NRFX_DELAY_US(3); // tRES1 required to come out of sleep

    // Reset sequence has to happen as two transfers
    uint8_t reset_seq[3] = {0x66, 0x99};
    spi_tx_rx((uint8_t *)&reset_seq, 1, NULL, 0, false);
    spi_tx_rx((uint8_t *)&reset_seq + 1, 1, NULL, 0, false);
    NRFX_DELAY_US(30); // tRST to fully reset

    // Check if the capacity ID corresponds to 32M
    uint8_t cap_id_reg[1] = {0x9F};
    uint8_t cap_id_res[4] = {0};
    spi_tx_rx((uint8_t *)&cap_id_reg, 1, (uint8_t *)&cap_id_res, 4, false);

    if (cap_id_res[3] != 0x16)
    {
        return S1_FLASH_ERROR;
    }

    return S1_SUCCESS;
}

void s1_flash_erase_all(void)
{
    // Issue erase sequence
    uint8_t erase_seq[2] = {0x06, 0x60};
    spi_tx_rx((uint8_t *)&erase_seq, 1, NULL, 0, false);
    spi_tx_rx((uint8_t *)&erase_seq + 1, 1, NULL, 0, false);
}

bool s1_flash_is_busy(void)
{
    // Read status register
    uint8_t status_reg[1] = {0x05};
    uint8_t status_res[2] = {0};
    spi_tx_rx((uint8_t *)&status_reg, 1, (uint8_t *)&status_res, 2, false);

    if (!(status_res[1] & 0x01))
    {
        return false;
    }

    return true;
}

void s1_flash_page_from_image(uint32_t offset,
                              unsigned char *image)
{
    uint8_t tx[260];

    // Disable write protection
    tx[0] = 0x06;
    spi_tx_rx((uint8_t *)&tx, 1, NULL, 0, false);

    // Write page command with 24bit address
    // Lowest byte of address is always 0
    tx[0] = 0x02;
    tx[1] = (uint8_t)(offset >> 16);
    tx[2] = (uint8_t)(offset >> 8);
    tx[3] = 0x00; // Lower byte 0 to avoid partial pages

    // Copy page from image and transfer
    memcpy(tx + 4, image + offset, 256);
    spi_tx_rx((uint8_t *)&tx, 260, NULL, 0, false);
}

s1_error_t flash_tx_rx(uint8_t *tx_buffer, size_t tx_len,
                       uint8_t *rx_buffer, size_t rx_len)
{
    return spi_tx_rx(tx_buffer, tx_len, rx_buffer, rx_len, false);
}

void s1_fpga_hold_reset(void)
{
    nrf_gpio_pin_clear(FPGA_RESET_PIN);
}

void s1_fpga_boot(void)
{
    // Release SPI
    nrfx_spim_uninit(&spi);

    // Set the SPI pins as inputs
    // CS needs a pullup
    nrf_gpio_cfg_input(SPI_CS_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(SPI_CLK_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SPI_SI_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SPI_SO_PIN, NRF_GPIO_PIN_NOPULL);

    // Bring FPGA out of reset
    nrf_gpio_pin_set(FPGA_RESET_PIN);
}

bool s1_fpga_is_booted(void)
{
    if (fpga_done_flag_pending)
    {
        fpga_done_flag_pending = false;
        return true;
    }
    return false;
}

s1_error_t fpga_tx_rx(uint8_t *tx_buffer, size_t tx_len,
                      uint8_t *rx_buffer, size_t rx_len)
{
    return spi_tx_rx(tx_buffer, tx_len, rx_buffer, rx_len, true);
}