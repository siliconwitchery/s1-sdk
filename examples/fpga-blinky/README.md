# FPGA Blinky Example.

This example demonstrates how to load an FPGA binary on the S1 Module. Here, the binary is stored within the nRF52 application code. On startup, it copies the binary to the S1's SPI flash, then allows the FPGA to boot from it. This mechanism is how you would boot the FPGA normally, and the initial binary could be received over bluetooth rather than be baked in. The same boot mechanism would apply, and the external flash would allow for the nRF flash area to be reserved for your application.

On power up, the following operations take place. The process of erasing and flashing the SPI flash takes around **30 seconds**, After which, you will see D3 (White LED on the S1 Popout Board) blink.

1. S1 Initializes
1. IO and FPGA voltages are set
1. Flash is erased
1. Flash is programmed with the  FPGA binary
1. SPI control is handed to the FPGA and it boots

## Files

The example is self documented. See how it works by studying the files:

- `main.c` – Main nRF52 application running the above tasks.
- `fpga_blinky.v` – Verilog code for blinking the D3 pin.
- `fpga_blink_bin.h` – The complied FPGA binary. If you modify the FPGA project, you should run `make build-verilog` to recreate it.
- `Makefile` – Project makefile with build configuration.

## Building

To build the verilog project, run `make -C examples/fpga-blinky build-verilog` from the `s1-sdk` root folder.

To build the nRF project, run `make -C examples/fpga-blinky` from the `s1-sdk` root folder.