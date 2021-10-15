# S1 Software Development Kit

This is the main SDK for the [S1 System-on-Module](https://www.siliconwitchery.com/module). It doesn't do much on it's own, you should use it with an example. Try it with the [FPGA blinky demo](#), or the [S1 ECG demo](https://github.com/siliconwitchery/s1-ecg-demo).

You will however need to setup your system in order to build everything.

## Here's how to do it

- First, download and extract the [Nordic nRF5 SDK](https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK) somewhere onto your system.

Then install the following tools: 

- [ARM GCC toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- [J-Link software pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) or your preferred debugging tool.
- [nRF command line tools](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download) is convenient if you're using a JLink.
- [yosys](https://yosyshq.net/yosys/download.html), [icestorm & NextPNR](http://www.clifford.at/icestorm/).
- Optionally [gtkwave](http://gtkwave.sourceforge.net) and [iVerilog](http://iverilog.icarus.com) are useful for simulation and verification.



### MacOS

We recommend using [homebrew](https://brew.sh) along with our [brew formula](https://github.com/siliconwitchery/homebrew-oss-fpga) to install everything.

### Windows

Most of these tools are available as installers, but some of them might need Cygwin or similar. It's best to Google how to do this, and we will update this page once we've tested it.

### Linux

Most of these tools are available from standard package managers, but you can build them for source quite easily.

## Overview of files:

We've designed this SDK so you can add it as a sub-module into your own git projects. The files here are therefore very lightweight and designed to be easy to use.

- `s1.mk` - This is the core makefile that ensures that the nRF code can be built. You should call it with your own `Makefile` where you specify your applications files and any other settings you may want.

- `s1.c` - These are the core functions that run on the nRF chip. Access them via `s1.h`

- `s1.h` - Here you'll find the APIs for configuration and runtime functions that run on the nRF chip. You can include this file and call them from your own application code.

- `s1.ld` - This is the linker file which determines the memory layout within the nRF chip when the code is built.

- `s1.pcf` - The FPGA pin configuration resides here. The names of the pins correspond to the pins of the FPGA, where `Dx` are the exposed pins, and the remaining pins are internal to the module.

That's it! Again in order to use these files, it's better to look at an example project, and copy that layout for your own application.

## Precautions

Be careful if you decide to access S1 core components outside of the SDK. The PMIC is capable of blowing up everything on the module if set incorrectly, and we're already tuned things to work as efficiently as possible.

It's also possible to misconfigure the battery charger and send dangerously high currents to small batteries. Be sure to read the datasheet of the PMIC, as well as our [documentation center](https://docs.siliconwitchery.com/hardware/s1-module/) before connecting batteries to the S1.

## Give us feedback

As this is a new project, we'd love for those using the S1 to let us know of any bugs or ideas for improvement. File them on the [GitHub issue tab](https://github.com/siliconwitchery/s1-sdk/issues).