/**
 * @file  s1_tests.c
 *
 * @brief S1 Module tests.
 *
 *        If no main() function is provided, the S1 SDK will automatically build
 *        this test application. It can be useful to checking proper
 *        functionality of your device.
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

#include "s1.h"

/**
 * @brief Macro for logging passed tests in green.
 */
#define LOG_PASS(cond, format, ...)                                                                               \
    do                                                                                                            \
    {                                                                                                             \
        if (cond)                                                                                                 \
        {                                                                                                         \
            char _debug_log_buffer[SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - 1] = "";                                    \
            snprintf(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - 1,                                     \
                     "\r\n" RTT_CTRL_TEXT_BRIGHT_GREEN "[PASS] " RTT_CTRL_RESET format, ##__VA_ARGS__);           \
            SEGGER_RTT_Write(0, _debug_log_buffer, strnlen(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP)); \
        }                                                                                                         \
    } while (0)

/**
 * @brief Macro for logging failed tests in red.
 */
#define LOG_FAIL(cond, format, ...)                                                                               \
    do                                                                                                            \
    {                                                                                                             \
        if (cond)                                                                                                 \
        {                                                                                                         \
            char _debug_log_buffer[SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - 1] = "";                                    \
            snprintf(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - 1,                                     \
                     "\r\n" RTT_CTRL_TEXT_BRIGHT_RED "[FAIL] " RTT_CTRL_RESET format, ##__VA_ARGS__);             \
            SEGGER_RTT_Write(0, _debug_log_buffer, strnlen(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP)); \
        }                                                                                                         \
    } while (0)

/**
 * @brief Test application.
 */
int main(void)
{
    // Error variable we'll use to check all the functions
    s1_error_t err;

    // Log some stuff about this project
    LOG_CLEAR();
    LOG("S1 Module Tests – Built: %s %s – SDK Version: %s.",
        __DATE__,
        __TIME__,
        __S1_SDK_VERSION__);

    // Initialise the S1 module
    err = s1_init();
    LOG_PASS(err == S1_SUCCESS, "S1 started");
    LOG_FAIL(err != S1_SUCCESS, "S1 init error. Code: %d", err);

    // Enable Vio and Vfpga to their nominal voltages
    LOG("[INFO] Enabling Vfpga and Vio to their nominal voltages");
    err = s1_pimc_set_vfpga(true);
    LOG_FAIL(err != S1_SUCCESS, "s1_pimc_set_vfpga() returned the error code %d", err);
    err = s1_pmic_set_vio(1.8f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);

    float vio;
    bool lsw_mode;
    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_get_vio() returned the error code %d", err);
    LOG_FAIL(vio != 1.8f || lsw_mode == true, "Vio did not configure correctly");
    LOG_PASS(vio == 1.8f && lsw_mode == false, "Vio started correctly");

    bool vfpga_enabled;
    err = s1_pimc_get_vfpga(&vfpga_enabled);
    LOG_FAIL(err != S1_SUCCESS, "s1_pimc_get_vfpga() returned the error code %d", err);
    LOG_FAIL(vfpga_enabled == false, "Vfpga did not enable");
    LOG_PASS(vfpga_enabled == true, "Vfpga enabled correctly");

    // Disable Vfpga and make sure Vio disables also
    LOG("[INFO] Disabling Vfpga and checking Vio disables also");
    err = s1_pimc_set_vfpga(false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pimc_set_vfpga() returned the error code %d", err);
    err = s1_pimc_get_vfpga(&vfpga_enabled);
    LOG_FAIL(vfpga_enabled == true, "Vfpga did not disable");
    LOG_PASS(vfpga_enabled == false, "Vfpga disabled");

    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_get_vio() returned the error code %d", err);
    LOG_FAIL(vio != 0.0f, "Vio did not disable");
    LOG_PASS(vio == 0.0f, "Vio disabled automatically");

    // Attempt to re-enable Vio without enabling Vfpga first
    LOG("[INFO] Attempting to re-enable Vio without enabling Vfpga first");
    err = s1_pmic_set_vio(1.8f, false);
    LOG_FAIL(err != S1_PMIC_VFPGA_NOT_ENABLED, "s1_pmic_set_vio() returned the error code %d", err);
    LOG_PASS(err == S1_PMIC_VFPGA_NOT_ENABLED, "Vio correctly refused to turn on in LDO mode");

    err = s1_pmic_set_vio(1.0f, true);
    LOG_FAIL(err != S1_PMIC_VFPGA_NOT_ENABLED, "s1_pmic_set_vio() returned the error code %d", err);
    LOG_PASS(err == S1_PMIC_VFPGA_NOT_ENABLED, "Vio correctly refused to turn on in LSW mode");

    // Enable Vfpga again, and attempt to set Vio out of normal ranges

    return 0;
}