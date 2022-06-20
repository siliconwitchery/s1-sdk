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

    // Set the rails to default values
    LOG("[INFO] Setting all rails to default values");
    err = s1_pmic_set_vaux(3.55f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vaux() returned the error code %d", err);
    err = s1_pmic_set_vio(3.0f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    err = s1_pimc_set_vfpga(false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pimc_set_vfpga() returned the error code %d", err);

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
    LOG("[INFO] Enabling Vfpga for Vio range tests");
    err = s1_pimc_set_vfpga(true);
    LOG_FAIL(err != S1_SUCCESS, "s1_pimc_set_vfpga() returned the error code %d", err);

    err = s1_pmic_set_vio(0.7f, false);
    LOG_FAIL(err != S1_PMIC_INVALID_VALUE, "Vio incorrectly set below 0.8V");
    LOG_PASS(err == S1_PMIC_INVALID_VALUE, "Vio correctly refused to set below 0.8V");

    err = s1_pmic_set_vio(0.8f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    LOG_PASS(err == S1_SUCCESS, "Vio correctly set to 0.8V");

    err = s1_pmic_set_vio(3.475f, false);
    LOG_FAIL(err != S1_PMIC_INVALID_VALUE, "Vio incorrectly set above 3.45V");
    LOG_PASS(err == S1_PMIC_INVALID_VALUE, "Vio correctly refused to set above 3.45V");

    err = s1_pmic_set_vio(3.45f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    LOG_PASS(err == S1_SUCCESS, "Vio correctly set to 3.45V");

    // Test correct rounding of passed parameters for Vio
    LOG("[INFO] Testing correct rounding of Vio voltage parameters");
    err = s1_pmic_set_vio(3.01f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_get_vio() returned the error code %d", err);
    LOG_FAIL(vio != 3.0f, "Vio did not round down correctly. Vio = %f", (double)vio);
    LOG_PASS(vio == 3.0f, "Vio correctly rounded down to 3.0V");

    err = s1_pmic_set_vio(3.02f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_get_vio() returned the error code %d", err);
    LOG_FAIL(vio != 3.025f, "Vio did not round up correctly. Vio = %f", (double)vio);
    LOG_PASS(vio == 3.025f, "Vio correctly rounded up to 3.025V");

    // Test Vio warning when Vaux is not in a suitable range
    LOG("[INFO] Testing Vio configuration when Vaux is disabled");
    err = s1_pmic_set_vaux(0.0f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vaux() returned the error code %d", err);
    LOG_PASS(err == S1_SUCCESS, "Vaux correctly shutdown");

    err = s1_pmic_set_vio(1.0f, false);
    LOG_FAIL(err != S1_PMIC_VAUX_NOT_ENABLED, "Vio configured incorrectly while Vaux is disabled");
    LOG_PASS(err == S1_PMIC_VAUX_NOT_ENABLED, "Vio correctly returned that Vaux is not enabled");

    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(err != S1_PMIC_VAUX_NOT_ENABLED, "s1_pmic_get_vio() returned the error code %d", err);
    LOG_FAIL(vio != 1.0f, "Vio did not configure to 1.0V anyway. Vio = %f", (double)vio);
    LOG_PASS(vio == 1.0f, "Vio correctly configured to 1.0V anyway");

    err = s1_pmic_set_vaux(3.0f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vaux() returned the error code %d", err);
    LOG_PASS(err == S1_SUCCESS, "Vaux set to 3.0V");

    err = s1_pmic_set_vio(2.925f, false);
    LOG_FAIL(err != S1_PMIC_VAUX_TOO_LOW, "Vio incorrectly configured to above the LDO dropout threshold. Error = %d", err);
    LOG_PASS(err == S1_PMIC_VAUX_TOO_LOW, "Vio correctly returned dropout level warning");

    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(vio != 2.925f, "Vio did not configure to 2.925V anyway. Vio = %f", (double)vio);
    LOG_PASS(vio == 2.925f, "Vio correctly configured to 2.925V anyway");

    // Check load switch modes for Vio
    LOG("[INFO] Testing Vio load switch modes");
    err = s1_pmic_set_vio(0.0f, true);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    LOG_FAIL(vio != 0.0f || lsw_mode != true, "Vio as load switch (off) not correctly set");
    LOG_PASS(vio == 0.0f && lsw_mode == true, "Vio as load switch (off) correctly set");

    err = s1_pmic_set_vio(1.0f, true);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    err = s1_pmic_get_vio(&vio, &lsw_mode);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    LOG_FAIL(vio != 1.0f || lsw_mode != true, "Vio as load switch (on) not correctly set");
    LOG_PASS(vio == 1.0f && lsw_mode == true, "Vio as load switch (on) correctly set");

    err = s1_pmic_set_vaux(3.5f);
    LOG_FAIL(err != S1_PMIC_INVALID_VALUE, "Vaux incorrectly set to a high voltage while Vio is in load switch mode");
    LOG_PASS(err == S1_PMIC_INVALID_VALUE, "Vaux correctly refused to set to a high voltage while Vio is in load switch mode");

    err = s1_pmic_set_vio(0.0f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    err = s1_pmic_set_vaux(3.5f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vaux() returned the error code %d", err);
    err = s1_pmic_set_vio(0.0f, true);
    LOG_FAIL(err != S1_PMIC_VAUX_TOO_HIGH, "Vio incorrectly set to load switch mode while Vaux is too high");
    LOG_PASS(err == S1_PMIC_VAUX_TOO_HIGH, "Vio correctly refused to set to load switch mode while Vaux is too high");

    // Test Vaux ranges
    LOG("[INFO] Testing Vaux range limits");
    err = s1_pmic_set_vaux(0.75f);
    LOG_FAIL(err != S1_PMIC_INVALID_VALUE, "Vaux incorrectly set below 0.8V");
    LOG_PASS(err == S1_PMIC_INVALID_VALUE, "Vaux correctly refused to set below 0.8V");

    err = s1_pmic_set_vaux(0.8f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vaux() returned the error code %d", err);
    LOG_PASS(err == S1_SUCCESS, "Vaux correctly set to 0.8V");

    err = s1_pmic_set_vio(0.0f, false);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vio() returned the error code %d", err);
    err = s1_pmic_set_vaux(5.55f);
    LOG_FAIL(err != S1_PMIC_INVALID_VALUE, "Vaux incorrectly set above 5.5V");
    LOG_PASS(err == S1_PMIC_INVALID_VALUE, "Vaux correctly refused to set below 5.5V");

    err = s1_pmic_set_vaux(5.5f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vaux() returned the error code %d", err);
    LOG_PASS(err == S1_SUCCESS, "Vaux correctly set to 5.5V");

    // Test correct rounding of passed parameters for Vaux
    LOG("[INFO] Testing correct rounding of Vaux voltage parameters");
    err = s1_pmic_set_vaux(3.02f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_aux() returned the error code %d", err);
    float vaux;
    err = s1_pmic_get_vaux(&vaux);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_get_vaux() returned the error code %d", err);
    LOG_FAIL(vaux != 3.0f, "Vaux did not round down correctly. Vio = %f", (double)vaux);
    LOG_PASS(vaux == 3.0f, "Vaux correctly rounded down to 3.0V");

    err = s1_pmic_set_vaux(3.03f);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_set_vaux() returned the error code %d", err);
    err = s1_pmic_get_vaux(&vaux);
    LOG_FAIL(err != S1_SUCCESS, "s1_pmic_get_vaux() returned the error code %d", err);
    LOG_FAIL(vaux != 3.05f, "Vaux did not round up correctly. Vio = %f", (double)vaux);
    LOG_PASS(vaux == 3.05f, "Vaux correctly rounded up to 3.05V");

    return 0;
}