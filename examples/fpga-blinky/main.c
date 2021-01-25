/**
 * @file  examples/fpga-blinky/main.c
 * @brief Simple FPGA blinky Application running on S1
 *        
 *        Includes basic configuration of the S1 module,
 *        and operations required to boot the FPGA. The
 *        fpga-blinky.v file should be build and binary
 *        turned into a header file containing the FPGA
 *        boot image. The Makefile shows how to do this.
 * 
 * @attention (c) 2021 Silicon Witchery (info@siliconwitchery.com)
 *
 *        Licensed under a Creative Commons Attribution 
 *        4.0 International License. This code is provided
 *        as-is and no warranty is given.
*/

#include "nrf52811.h"
#include "nrf_gpio.h"
#include "nrfx_saadc.h"
#include "nrfx_spim.h"
#include "nrfx_twim.h"
#include "app_scheduler.h"
#include "s1.h"
#include "fpga_blinky_bin.h"

S1_TASK_DEF(fpga_boot_task_id);

typedef enum 
{
    STARTED,
    ERASING,
    FLASHING,
    DONE
} fpga_boot_state_t;

static fpga_boot_state_t fpga_boot_state = STARTED;
static uint32_t pages_remaining;

static void fpga_boot_task(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    switch (fpga_boot_state)
    {
        case STARTED:
            LOG("FPGA boot started.\n");
            // Configure power
            // s1_pimc_fpga_vcore(true);
            // s1_pmic_set_vio(3.0);
            // s1_pmic_set_vaux(3.3);
            // s1_pmic_set_iaux(0.5);
            // s1_pmic_set_vaux_boost_mode(false);

            // Wake the flash and erase
            // s1_flash_wake();
            // s1_flash_erase_all();
            fpga_boot_state = ERASING;
            break;

        case ERASING:
            LOG("Erasing flash.\n");

            // If the erase is complete, we start
            // flashing
            // if (!s1_flash_is_busy())
            {
                pages_remaining = (uint32_t) ceil((float) fpga_blinky_bin_len / 256.0);
                fpga_boot_state = FLASHING;
            }
            break;

        case FLASHING:
            LOG("Flashing.\n");
            // if (pages_remaining == 0)
            {
                fpga_boot_state = DONE;
                break;
            }
            // if (!s1_flash_is_busy())
            {
                // s1_flash_page_from_image(0, &fpga_blinky_bin);
            }
            break;

        case DONE:
            LOG("Starting FPGA.\n");

            // Boot the FPGA and stop this task
            // s1_fpga_boot();
            app_timer_stop(fpga_boot_task_id);
            break;
    }
}

#include "nrfx_clock.h"
void clock_event_handler(nrfx_clock_evt_type_t event)
{
}

int s1_app_setup(void)
{
    LOG_CLEAR();
    LOG("S1 FPGA Blinky Example.\r\n");
    LOG("Built: %s %s – SDK Version: %s\r\n", __DATE__, __TIME__, __S1_SDK_VERSION__);

    ret_code_t err_code;

    err_code = nrfx_clock_init(clock_event_handler);
    APP_ERROR_CHECK(err_code);
    nrfx_clock_lfclk_start();

    APP_SCHED_INIT(sizeof(uint32_t), 5);

    err_code = app_timer_create(&fpga_boot_task_id,
                                APP_TIMER_MODE_REPEATED,
                                fpga_boot_task);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(fpga_boot_task_id, 
                               APP_TIMER_TICKS(2000),
                               NULL);

    APP_ERROR_CHECK(err_code);
}