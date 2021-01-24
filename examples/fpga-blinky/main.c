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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "nrf_gpio.h"
#include "nrfx_saadc.h"
#include "nrfx_spim.h"
#include "nrfx_twim.h"
#include "nrf52811.h"

#include "fpga_blinky_bin.h"
#include "s1_pinout.h"
#include "s1_debug.h"
#include "s1_sdk_version.h"

int main(void)
{
    LOG_CLEAR();
    LOG("S1 SDK Version: %s\r\n", __S1_SDK_VERSION__);
    LOG("Project Built: %s at %s.\r\n", __DATE__, __TIME__);

    while (true)
    {
        __NOP(); // Do nothing
    }
}