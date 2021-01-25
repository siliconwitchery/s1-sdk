/**
 * @file  examples/fpga-blinky/main.c
 * @brief Simple FPGA blinky Application running on S1
 *        
 *        Includes basic configuration of the S1 module,
 *        and operations required to boot the FPGA. The
 *        FPGA verilog project can be built by running
 *        "make build-verilog" from this folder.
 * 
 * @attention (c) 2021 Silicon Witchery (info@siliconwitchery.com)
 *
 *        Licensed under a Creative Commons Attribution 
 *        4.0 International License. This code is provided
 *        as-is and no warranty is given.
*/

#include "app_scheduler.h"
#include "app_timer.h"
#include "fpga_blinky_bin.h"
#include "nrf_gpio.h"
#include "nrf52811.h"
#include "nrfx_clock.h"
#include "nrfx_saadc.h"
#include "nrfx_spim.h"
#include "nrfx_twim.h"
#include "s1.h"

APP_TIMER_DEF(fpga_boot_task_id);

typedef enum 
{
    STARTED,
    ERASING,
    FLASHING,
    BOOTING,
    DONE
} fpga_boot_state_t;

static fpga_boot_state_t fpga_boot_state = STARTED;
static uint32_t pages_remaining;


/**
 * @brief Clock event callback. Not used but required to have.
 */
void clock_event_handler(nrfx_clock_evt_type_t event){}


/**
 * @brief Timer based state machine for flashing the FPGA
 *        image and booting the FPGA. As some of the flash
 *        operations take a lot of time, using a timer based
 *        state machine avoids the main thread hanging while
 *        waiting for flash operations to complete.
 */
static void fpga_boot_task(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    switch (fpga_boot_state)
    {
        // Configure power and erase the flash
        case STARTED:
            s1_pimc_fpga_vcore(true);
            s1_pmic_set_vio(3.0);
            s1_pmic_set_vaux(3.3);
            s1_fpga_hold_reset();
            s1_flash_wakeup();
            s1_flash_erase_all();
            fpga_boot_state = ERASING;
            LOG("Erasing flash. Takes up to 80 seconds.");
            break;

        // Wait for erase to complete
        case ERASING:
            if (!s1_flash_is_busy())
            {
                pages_remaining = (uint32_t) ceil((float) fpga_blinky_bin_len / 256.0);
                fpga_boot_state = FLASHING;
                LOG("Flashing pages.");
            }
            break;

        // Flash every page until done
        case FLASHING:
            if (!s1_flash_is_busy())
            {
                s1_flash_page_from_image(0, &fpga_blinky_bin);
                pages_remaining--;
            }
            
            if (pages_remaining == 0)
            {
                fpga_boot_state = BOOTING;
                // s1_fpga_boot();
                LOG("Flashing done.");
                break;
            }
            break;

        // Wait for CDONE pin to go high then stop the task
        // and put the flash into deep sleep.
        case BOOTING:
            //if (s1_fpga_booted())
            {
                //s1_flash_deep_sleep();
                app_timer_stop(fpga_boot_task_id);
                fpga_boot_state = DONE;
                LOG("FPGA started.");
            }
            break;
    }
}


/**
 * @brief Main application entry for the fpga-blinky demo.
 */
int main(void)
{
    // Log some stuff about this project
    LOG_CLEAR();
    LOG("S1 FPGA Blinky Demo – Built: %s %s – SDK Version: %s.",
        __DATE__,
        __TIME__,
        __S1_SDK_VERSION__);

    // Initialise S1 base setting
    APP_ERROR_CHECK( s1_init() );

    // Initialise LFXO required by the App Timer
    APP_ERROR_CHECK( nrfx_clock_init(clock_event_handler) );
    nrfx_clock_lfclk_start();

    // Initialise the App Timer
    APP_ERROR_CHECK( app_timer_init() );
    APP_SCHED_INIT(sizeof(uint32_t), 5);

    // Create and start a timer for the FPGA flash/boot task
    APP_ERROR_CHECK( app_timer_create(&fpga_boot_task_id,
                                      APP_TIMER_MODE_REPEATED,
                                      fpga_boot_task) ); 

    APP_ERROR_CHECK( app_timer_start(fpga_boot_task_id, 
                                     APP_TIMER_TICKS(5),
                                     NULL) );

    // The CPU is free to do nothing in the meanwhile
    for(;;)
    {
        __WFI();
    }
}