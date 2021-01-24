# S1 Software Development Kit

The S1 Module is Bluetooth/FPGA module based on the Nordic [nRF52811 chip](https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF52811). The Software Development Kit is therefore based around the SDK for the Nordic chipsets. It's intended to act as a minimal set of configuration functions to let you easily configure the Bluetooth IC, configure the Power Management IC, and FPGA.

**WARNING:** The S1 module features an advanced Power Management IC. However this IC is capable of outputting voltages that can destroy the devices on the module. It's therefore safer to use the PMIC configuration calls provided by the S1 SDK to avoid accidentally setting high voltages. Be careful if you wish to modify or write your own functions for configuring the PMIC.

## Getting started

### 1. Install the build tools:

1. Install the [ARM GCC Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

1. Extract the [Nordic nRF5 SDK](https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK) to your home folder

1. Install the [Nordic command line tools](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download)

1. Ensure `nrfjprog` runs from command line

1. Install [APIO](https://github.com/FPGAwars/apio)

1. Ensure `apio` runs from command line

1. Run `apio install -a` from command line

1. Ensure `make` runs from the command line. *MacOS users may need to run `xcode-select --install`. Windows users can use [GnuWin](http://gnuwin32.sourceforge.net/) or [chocolatey](https://chocolatey.org/packages/make)*

Optionally Install the [J-Link Software Pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) if you want the latest tools, though the Nordic command line tools already comes bundled with them.

### Setup the project:

1. Download or [clone](https://docs.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository) the latest S1 SDK.

1. Duplicate `paths_template.mk` and rename it to `paths.mk`.

1. Modify `paths.mk` with the paths of the installed tools above.

### Test a build:

1. Open a terminal and navigate to the `examples/minimal` folder.

1. Run `make`

1. The project should build without errors.

### Setup VS Code:

VS Code isn't strictly necessary your IDE, but there are some handy scripts included to help you build and debug.

1. Install [VS Code](https://code.visualstudio.com)

1. Click the Extensions tab along the right bar.

1. Install the **Cortex-Debug** plugin by marus25

1. Install the **C/C++** plugin by Microsoft

1. Install the **Better C++ Syntax** plugin by Jeff Hykin

1. Install the **Verilog-HDL/SystemVerilog** by mshr-h

1. Duplicate the three files in the `.vscode` folder and rename them to. `tasks.json`, `launch.json` and `cpp_properties.json`

1. Modify these files to match your install paths

1. On the Explorer tap, add the `s1-sdk` folder as well as the `nRF5 SDK` folder.

1. Press **Ctrl-Shift-B** (**Cmd-Shift-B** on MacOS) to build the test project.

1. It should build successfully. Check the terminal output to solve the error.

## Running on Hardware

### You will need:

1. Your custom S1 board, or the S1 Popout board *(Coming soon)*

1. Any model of [J-Link debugger](https://www.segger.com/products/debug-probes/j-link/models/model-overview/), or a [Nordic nRF52DK Board](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52-DK). The [Black Magic Probe]() is also supported, but you will need to adjust the launch script within the .vscode folder of this project. 

1. A [10 pin Cortex Debug Cable](https://www.adafruit.com/product/1675). One comes included with a J-Link mini EDU and Black magic probe, however for the nRF52DK, you will need to buy one seperate.

### Wire it up:

![](Diagram of S1 Popout with nRF52DK)

### Debug:

There is a VS Code launch script in the `.vscode` folder that will launch a GDB debugging session.
1. Build the project

1. Click the Run tab on the left bar

1. Ensure the JLink launch configuration is selected

1. Press the green play button.

1. The board should automatically flash, and wait at the breakpoint on `main()`

The environment is now fully configured to start developing with.

### Logging:

The Nordic SDK uses Segger's RTT protocol to return serial logs over the J-Link debugger. Read more about the protocol [here](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/).

Logging is enabled by default. To read the logs you can use:

- JLinkRTTClient.exe – If the GDB debugger is running.
- JLinkRTTViewer.exe – If the debugger is not running. 
## Project Structure

The SDK structure is as follows:
```
.
├── .vscode
│   ├── c_cpp_properties.json
│   ├── c_cpp_properties_template.json
│   ├── launch.json
│   ├── launch_template.json
│   ├── tasks.json
│   └── tasks_template.json
├── examples
│   ├── fpga-blinky
│   │   ├── Makefile
│   │   ├── apio.ini
│   │   ├── fpga-blinky.pcf
│   │   ├── fpga-blinky.v
│   │   ├── main.c
│   │   └── sdk_config.h
│   └── minimal
│       ├── Makefile
│       ├── main.c
│       └── sdk_config.h
├── paths.mk
├── paths_template.mk
├── s1.c
├── s1.h
├── s1.ld
└── s1.mk
```

### Core files:

| File | Description |
|-|-|
| `s1.h` | Core APIs Header. Use these functions to configure the S1 |
| `s1.c` | Core functions for the S1. Access them through the above header |
| `s1.mk` | Base makefile which describes how S1 projects are built. It should be called by the `Makefile` in your project folder. |
| `s1.ld` | Linker script for the nRF52811 chip |
| `paths_template.mk` | Template paths file. Copy this and modify for your paths |
| `paths.mk` | Your renamed paths file |

### VS Code files:

| File | Description |
|-|-|
| `tasks_template.json` | Template build tasks script. Copy this and modify for your paths |
| `tasks.json` | Your renamed tasks file |
| `launch_template.json` |  |
| `launch.json` | Your renamed launch file |
| `c_cpp_properties_template.json` |  |
| `c_cpp_properties.json` | Your renamed properties file |

###  Examples

Each example is loosely based on examples from the nRF5 SDK. You can make your own project by duplicating one of them and using it as a starting point.

#### Minimal example:

#### FPGA Blinky: