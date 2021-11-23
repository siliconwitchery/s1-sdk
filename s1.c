/**
 * @file  s1.c
 * @brief S1 Module Core Functions
 *        
 *        Various functions to setup and configure the S1 Module. To access 
 *        these functions, use the s1.h header file.
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

#include <string.h>

#include "nrf_gpio.h"
#include "nrfx_gpiote.h"
#include "nrfx_saadc.h"
#include "nrfx_spim.h"
#include "nrfx_twim.h"
#include "nrf52811.h"
#include "s1.h"

// Instances for I2C and SPI
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(0);
static const nrfx_twim_t i2c = NRFX_TWIM_INSTANCE(0);

// Interupt driven pending flag for when the FPGA_DONE_PIN goes high
static bool fpga_done_flag_pending = false;

// The PMIC control interface pins and I2C address
#define PMIC_SDA_PIN NRF_GPIO_PIN_MAP(0, 14)
#define PMIC_SCL_PIN NRF_GPIO_PIN_MAP(0, 17)
#define PMIC_I2C_ADDRESS 0x48

// ADC input for the battery monitor pin from the PMIC
#define PMIC_AMUX_PIN NRF_SAADC_INPUT_AIN1

static uint8_t pmic_read_reg(uint8_t reg)
{
    uint8_t rx_buffer;
    nrfx_twim_xfer_desc_t i2c_xfer = NRFX_TWIM_XFER_DESC_TXRX(PMIC_I2C_ADDRESS, &reg, 1, &rx_buffer, 1);
    nrfx_err_t err = nrfx_twim_xfer(&i2c, &i2c_xfer, 0);
    if (err != NRF_SUCCESS)
    {
        NRFX_DELAY_US(100);
        LOG("Double read");
        err = nrfx_twim_xfer(&i2c, &i2c_xfer, 0);
        APP_ERROR_CHECK(err);
    }
    return rx_buffer;
}

static void pmic_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2];
    nrfx_twim_xfer_desc_t i2c_xfer = NRFX_TWIM_XFER_DESC_TX(0x48, buffer, 2);
    buffer[0] = reg;
    buffer[1] = value;
    APP_ERROR_CHECK(nrfx_twim_xfer(&i2c, &i2c_xfer, 0));
}

s1_error_t flash_tx_rx(uint8_t *tx_buffer, size_t tx_len,
                       uint8_t *rx_buffer, size_t rx_len)
{
    // SPI hardware configuration
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
    spi_config.mosi_pin = SPI_SO_PIN;
    spi_config.miso_pin = SPI_SI_PIN;
    spi_config.sck_pin = SPI_CLK_PIN;
    spi_config.ss_pin = SPI_CS_PIN;

    // Initialise the SPI if it was not already
    nrfx_spim_init(&spi, &spi_config, NULL, NULL);

    nrfx_spim_xfer_desc_t spi_xfer = NRFX_SPIM_XFER_TRX(tx_buffer, tx_len,
                                                        rx_buffer, rx_len);

    APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &spi_xfer, 0));

    // TODO return error code for SPI errors
}

s1_error_t s1_pmic_set_vaux(float voltage)
{
    // Check if voltage is a valid range
    if (voltage < 0.8f || voltage > 5.5f)
    {
        return S1_INVALID_SETTING;
    }

    // If voltage > than 3.46, then LDO0 must not
    // be in LSW mode.
    if (voltage > 3.46f && (pmic_read_reg(0x39) & 0x08))
    {
        return S1_INVALID_SETTING;
    }

    // If 0V, we shutdown the SSB2
    if (voltage == 0.0f)
    {
        pmic_write_reg(0x2E, 0x0C);
        return S1_SUCCESS;
    }

    // Set LDO target voltage
    uint8_t voltage_setting = (uint8_t)((voltage - 0.8f) / 0.05f);
    pmic_write_reg(0x2D, voltage_setting);

    // Enable SSB2
    // - Buck Boost mode
    // - Discharge resistor enable
    // - 1A limit
    pmic_write_reg(0x2E, 0x0E);

    return S1_SUCCESS;
}

s1_error_t s1_pmic_set_vio(float voltage)
{
    // Check if voltage is a valid range
    // 3.46V limit is to protect FPGA
    if (voltage < 0.8f || voltage > 3.46f)
    {
        return S1_INVALID_SETTING;
    }

    // If 0V, we shutdown the LDO
    if (voltage == 0.0f)
    {
        pmic_write_reg(0x39, 0x0C);
        return S1_SUCCESS;
    }

    // Set LDO target voltage
    uint8_t voltage_setting = (uint8_t)((voltage - 0.8f) / 0.025f);
    pmic_write_reg(0x38, voltage_setting);

    // Enable LDO0
    // - LDO regulator mode
    // - Discharge resistor active
    // - Enable
    pmic_write_reg(0x39, 0x0E);

    return S1_SUCCESS;
}

void s1_pimc_fpga_vcore(bool enable)
{
    // Ensure SSB1 is 1.2V
    pmic_write_reg(0x2B, 0x08);

    // If enable
    if (enable)
    {
        // Enable SSB1
        // - 0.333A limit
        // - Buck mode
        pmic_write_reg(0x2C, 0x7E);
        return;
    }

    // Disable LDO0 (Vio). Required to avoid
    // IO voltages damaging the FPGA core.
    pmic_write_reg(0x39, 0x0C);

    // Disable SSB1 (Vfpga)
    pmic_write_reg(0x2C, 0x7C);
}

s1_error_t s1_flash_wakeup(void)
{
    // Wake up the flash
    uint8_t wake_seq[4] = {0xAB, 0, 0, 0};
    uint8_t wake_res[5] = {0};
    flash_tx_rx((uint8_t *)&wake_seq, 4, (uint8_t *)&wake_res, 5);
    NRFX_DELAY_US(3); // tRES1 required to come out of sleep

    // Reset sequence has to happen as two transfers
    uint8_t reset_seq[3] = {0x66, 0x99};
    flash_tx_rx((uint8_t *)&reset_seq, 1, NULL, 0);
    flash_tx_rx((uint8_t *)&reset_seq + 1, 1, NULL, 0);
    NRFX_DELAY_US(30); // tRST to fully reset

    // Check if the capacity ID corresponds to 32M
    uint8_t cap_id_reg[1] = {0x9F};
    uint8_t cap_id_res[4] = {0};
    flash_tx_rx((uint8_t *)&cap_id_reg, 1, (uint8_t *)&cap_id_res, 4);

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
    flash_tx_rx((uint8_t *)&erase_seq, 1, NULL, 0);
    flash_tx_rx((uint8_t *)&erase_seq + 1, 1, NULL, 0);
}

bool s1_flash_is_busy(void)
{
    // Read status register
    uint8_t status_reg[1] = {0x05};
    uint8_t status_res[2] = {0};
    flash_tx_rx((uint8_t *)&status_reg, 1, (uint8_t *)&status_res, 2);

    if (!(status_res[1] & 0x01))
    {
        return false;
    }

    return true;
}

s1_error_t s1_flash_page_from_image(uint32_t offset,
                                    unsigned char *image)
{
    uint8_t tx[260];

    // Disable write protection
    tx[0] = 0x06;
    flash_tx_rx((uint8_t *)&tx, 1, NULL, 0);

    // Write page comand with 24bit address
    // Lowest byte of address is always 0
    tx[0] = 0x02;
    tx[1] = (uint8_t)(offset >> 16);
    tx[2] = (uint8_t)(offset >> 8);
    tx[3] = 0x00; // Lower byte 0 to avoid partial pages

    // Copy page from image and transfer
    memcpy(tx + 4, image + offset, 256);
    flash_tx_rx((uint8_t *)&tx, 260, NULL, 0);

    // TODO include return codes for SPI errors
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

void fpga_done_pin_interrupt(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin == FPGA_DONE_PIN && action == NRF_GPIOTE_POLARITY_LOTOHI)
    {
        fpga_done_flag_pending = true;
    }
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

s1_error_t s1_init(void)
{
    // Configure FPGA reset pin as output (low signal holds FPGA in reset)
    nrf_gpio_cfg_output(FPGA_RESET_PIN);

    // Set up done pin as an interrupt. Goes high when configuration is done
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    APP_ERROR_CHECK(nrfx_gpiote_init());
    APP_ERROR_CHECK(nrfx_gpiote_in_init(FPGA_DONE_PIN, &config, fpga_done_pin_interrupt));
    nrfx_gpiote_in_event_enable(FPGA_DONE_PIN, true);

    // I2C hardware configuration
    nrfx_twim_config_t pmic_twi_config = NRFX_TWIM_DEFAULT_CONFIG;
    pmic_twi_config.scl = PMIC_SCL_PIN;
    pmic_twi_config.sda = PMIC_SDA_PIN;
    APP_ERROR_CHECK(nrfx_twim_init(&i2c, &pmic_twi_config, NULL, NULL));
    nrfx_twim_enable(&i2c);

    // Check PMIC Chip ID
    if (pmic_read_reg(0x14) != 0x7A)
    {
        return S1_PMIC_ERROR;
    }

    // TODO setup analog pins here?

    return S1_SUCCESS;
}