# FPGA Demo Project
#
# Here we build fpga demo including the verilog project 
# which outputs the FPGA binary. Once the binary is made,
# we convert it into a c header file, add some formating,
# and then we add it into the nRF project build.
#
# To build the FPGA blinky demo, simply run:
#
#	cd examples/fpga-blinky
#	make fpga-blinky
#

# Set the root of the S1 SDK
S1_SDK_ROOT = ../..

SRC_FILES += $(S1_SDK_ROOT)/examples/fpga-blinky/main.c

# Includes headers from this folder
INC_FOLDERS += $(S1_SDK_ROOT)/examples/fpga-blinky

# Include the base s1 makefile. You can also write your own
include $(S1_SDK_ROOT)/s1.mk

build-verilog:
	# Build the FPGA project using APIO
	# If you dont have it, get it here:
	# https://github.com/FPGAwars/apio
	apio build

	# Use xxd to convert the bin into a c
	# header file. The file name becomes
	# the C variable name, so we must
	# change the name from "hardware" to
	# "fpga_blinky_bin".
	mv hardware.bin fpga-blinky.bin
	xxd -i fpga-blinky.bin fpga_blinky_bin.h

	# Then we want the FPGA binary array
	# stored in flash, so we append it
	# with "const" using the sed tool.
	sed -i '' '1s/^/const /' fpga_blinky_bin.h

# Serialise the build of the verilog and the
# nRF code. Otherwise they will happen in
# Parallel and get mixed up. However the nRF
# build itself will still run in parallel.
fpga-blinky: 
	$(MAKE) build-verilog 
	$(MAKE) default
	$(MAKE) flash

