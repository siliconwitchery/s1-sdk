# S1 Software Development Kit

The S1 Module is Bluetooth/FPGA module based on the Nordic [nRF52811 chip](https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF52811). This Software Development Kit is based around Nordic's SDK for the nRF chips. It's intended to let you quickly configure the S1, while letting you easily work with Nordic's existing SDK and libraries.

**WARNING:** Be careful if you decide to access S1 core devices outside the provided APIs. The Power Management IC is capable of outputting high voltages, and misconfiguration may destroy the devices on the S1.

## Getting started

### 1. Install build tools:

1. Install the [ARM GCC Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

1. Extract the [Nordic nRF5 SDK](https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK) to your home folder

1. Install the [Nordic command line tools](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download)

1. Ensure `nrfjprog` runs from the command line

1. Install [APIO](https://github.com/FPGAwars/apio)

1. Ensure `apio` runs from the command line

1. Run `apio install -a` from the command line

1. Ensure `make` runs from the the command line. *MacOS users may need to run `xcode-select --install`. Windows users can use [GnuWin](http://gnuwin32.sourceforge.net/) or [chocolatey](https://chocolatey.org/packages/make)*

Optionally Install the [J-Link Software Pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) if you want the latest tools, though the Nordic command line tools already comes bundled with them.

### Setup the project:

1. Download or [clone](https://docs.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository) the S1 SDK by running `git clone https://github.com/siliconwitchery/s1-sdk.git` from the terminal.

1. Duplicate `paths_template.mk` and rename it to `paths.mk`.

1. Modify `paths.mk` with the paths of your installed tools.

### Test a build:

1. Open a terminal and navigate to the `s1-sdk` folder.

1. Run `make -C examples/fpga-blinky`

1. The project should build without errors.

1. To test a fpga verilog build, run `make -C examples/fpga-blinky build-verilog`

1. The APIO build process should run without errors.

### Setup VS Code:

The .vscode folder contains some handy build and debugging scripts if you decide to use VS Code as your IDE. To setup VS Code:

1. Install [VS Code](https://code.visualstudio.com)

1. Click the Extensions tab along the right bar.

1. Install the **Cortex-Debug** plugin by marus25.

1. Install the **C/C++** plugin by Microsoft.

1. Install the **Better C++ Syntax** plugin by Jeff Hykin.

1. Install the **Verilog-HDL/SystemVerilog** by mshr-h.

1. Duplicate the three files in the `.vscode` folder and rename them to. `tasks.json`, `launch.json` and `cpp_properties.json`

1. Modify these files to match your install paths.

1. On the Explorer tap, add the `s1-sdk` folder as well as the `nRF5 SDK` folder.

1. Press **Ctrl-Shift-B** (**Cmd-Shift-B** on MacOS) to build the test project.

1. It should build successfully. Check the terminal output to solve the error.

## Running on Hardware

### You will need:

1. Your custom S1 board, or the S1 Popout board *(Coming soon)*

1. Any model of [J-Link debugger](https://www.segger.com/products/debug-probes/j-link/models/model-overview/), or a [Nordic nRF52DK Board](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52-DK). The [Black Magic Probe]() is also supported, but you will need to adjust the launch script within the .vscode folder of this project. 

1. A [10 pin Cortex Debug Cable](https://www.adafruit.com/product/1675). It comes included with a J-Link mini EDU and Black magic probe, however for the nRF52DK, you will need to buy one.

### Wire it up:

![](Diagram of S1 Popout with nRF52DK)

### Debug:

A launch script in the `.vscode` folder demonstrates how to start a GDB debugging session over J-Link. To run it from VS Code:

1. Build the project.

1. Click the Run tab on the left bar.

1. Ensure the JLink launch configuration is selected.

1. Press the green play button.

1. The board should automatically flash, and wait at `main()`

1. Click Continue (F5) to run the application.

The environment is now fully configured to start developing with.

### Logging:

The Nordic SDK uses Segger's RTT protocol to return serial logs over a J-Link debugger. Read more about the protocol [here](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/).

Logging is enabled by default. To read the logs you can use:

- JLinkRTTClient.exe – If the GDB debugger is running.
- JLinkRTTViewer.exe – If the debugger is not running. Be sure to select `nRF52811_xxAA` and `SWD`.

## Project Structure

The S1 SDK contains minimal files for configuring the S1 Module peripherals. Outside of that you're free to use the Nordic SDK examples. Each example in the S1 SDK is closely based around a nordic example. By including `s1.mk` at the top of your own project's makefile, you will have access to the S1 configuration APIs.

Include `#include "s1.h"` in your own `main.c` file to access the S1 API.

Be sure to pull this repository occasionally to receive bugfixes and updated features.

## Help out

As this is a new project, we'd love for those using the S1 to let us know of any bugs or improvement you think of. File them on the [GitHub issue tab](https://github.com/siliconwitchery/s1-sdk/issues).