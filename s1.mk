# Base SDK makefile.
# ------------------
#
# Copyright 2021 Silicon Witchery AB
#
# Permission to use, copy, modify, and/or distribute this software for any 
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, 
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
# PERFORMANCE OF THIS SOFTWARE.
#
# ------------------


# You don't need to change anything here, but include this file in your own
# Makefile. From there you can override anything marked with ?=, or append to
# anything marked with += such as .c, .h files, or optimization flags.


# As a minimum, you only need to set these variables from you Makefile.
PROJECT_NAME ?= s1_sdk_standalone
NRF_SDK_PATH ?= ${HOME}/nRF5_SDK
S1_SDK_PATH ?= s1-sdk

# Optionally, you can change the build directory.
OUTPUT_DIRECTORY ?= .build

# There's also a sim directory which can be used for verilog test bench outputs.
SIM_DIRECTORY ?= .sim

# If using a bluetooth stack, the linker file must be changed to one with
# correct memory addresses for the bluetooth softdevice. Otherwise no bluetooth
# stack memory will be allocated using this basic linker file.
LINKER_FILE ?= $(S1_SDK_PATH)/s1.ld

# The GNU GCC prefix shouldn't change, but if you need to change it, you can
GNU_PREFIX ?= arm-none-eabi

# You can add more source files using 'SRC_FILES += ' in your Makefile.
SRC_FILES += \
  $(S1_SDK_PATH)/s1.c \
  $(NRF_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52811.S \
  $(NRF_SDK_PATH)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(NRF_SDK_PATH)/components/libraries/atomic/nrf_atomic.c \
  $(NRF_SDK_PATH)/components/libraries/balloc/nrf_balloc.c \
  $(NRF_SDK_PATH)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(NRF_SDK_PATH)/components/libraries/memobj/nrf_memobj.c \
  $(NRF_SDK_PATH)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(NRF_SDK_PATH)/components/libraries/scheduler/app_scheduler.c \
  $(NRF_SDK_PATH)/components/libraries/sortlist/nrf_sortlist.c \
  $(NRF_SDK_PATH)/components/libraries/strerror/nrf_strerror.c \
  $(NRF_SDK_PATH)/components/libraries/timer/app_timer2.c \
  $(NRF_SDK_PATH)/components/libraries/timer/drv_rtc.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_error_handler_gcc.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_error_weak.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_error.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_util_platform.c \
  $(NRF_SDK_PATH)/components/libraries/util/nrf_assert.c \
  $(NRF_SDK_PATH)/external/fprintf/nrf_fprintf_format.c \
  $(NRF_SDK_PATH)/external/fprintf/nrf_fprintf.c \
  $(NRF_SDK_PATH)/external/segger_rtt/SEGGER_RTT_printf.c \
  $(NRF_SDK_PATH)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
  $(NRF_SDK_PATH)/external/segger_rtt/SEGGER_RTT.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_saadc.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_spim.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_twim.c \
  $(NRF_SDK_PATH)/modules/nrfx/mdk/system_nrf52811.c \
  $(NRF_SDK_PATH)/modules/nrfx/soc/nrfx_atomic.c \

# As well as more include paths using 'INC_FOLDERS += '.
INC_FOLDERS += \
  . \
  $(S1_SDK_PATH) \
  $(NRF_SDK_PATH)/components \
  $(NRF_SDK_PATH)/components/drivers_nrf/nrf_soc_nosd \
  $(NRF_SDK_PATH)/components/libraries/atomic \
  $(NRF_SDK_PATH)/components/libraries/atomic_fifo \
  $(NRF_SDK_PATH)/components/libraries/balloc \
  $(NRF_SDK_PATH)/components/libraries/bsp \
  $(NRF_SDK_PATH)/components/libraries/delay \
  $(NRF_SDK_PATH)/components/libraries/experimental_section_vars \
  $(NRF_SDK_PATH)/components/libraries/log \
  $(NRF_SDK_PATH)/components/libraries/log/src \
  $(NRF_SDK_PATH)/components/libraries/memobj \
  $(NRF_SDK_PATH)/components/libraries/ringbuf \
  $(NRF_SDK_PATH)/components/libraries/scheduler \
  $(NRF_SDK_PATH)/components/libraries/sortlist \
  $(NRF_SDK_PATH)/components/libraries/timer \
  $(NRF_SDK_PATH)/components/libraries/strerror \
  $(NRF_SDK_PATH)/components/libraries/util \
  $(NRF_SDK_PATH)/components/toolchain/cmsis/include \
  $(NRF_SDK_PATH)/external/fprintf \
  $(NRF_SDK_PATH)/external/segger_rtt \
  $(NRF_SDK_PATH)/integration/nrfx \
  $(NRF_SDK_PATH)/modules/nrfx \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/include \
  $(NRF_SDK_PATH)/modules/nrfx/hal \
  $(NRF_SDK_PATH)/modules/nrfx/mdk \

# Libraries can also be added.
LIB_FILES += \

# These are some sensible warnings and optimizations. You can add more, or
# override them completely 
WARN ?= -Wall -Wextra -Wpedantic -Wvla -Wnull-dereference -Wswitch-enum \
        -Wundef -Wdouble-promotion -Wformat=2 -Wconversion -Wformat-truncation \
        -Wstack-usage=1000 -Wshadow -Wduplicated-cond -Wduplicated-branches

OPT ?= -std=gnu17 -pedantic -Os -g3 -fno-common -fstack-usage \
	     -ffunction-sections -fdata-sections -flto  -fno-strict-aliasing \
       -fno-builtin -fshort-enums

# These are some sensible default C flags, but you can add more in your 
# Makefile. If you're using a bluetooth stack, then you'll have to add a few
# specific flags to configure the softdevice.
CFLAGS += $(OPT)
CFLAGS += $(WARN)
CFLAGS += -DAPP_TIMER_V2
CFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
CFLAGS += -DFLOAT_ABI_SOFT
CFLAGS += -DNRF52811_XXAA
CFLAGS += -DNRFX_COREDEP_DELAY_US_LOOP_CYCLES=3
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mfloat-abi=soft
CFLAGS += -mthumb -mabi=aapcs

# C++ flags can also be added if needed.
CXXFLAGS += $(OPT)
CXXFLAGS += $(WARN)

# These assembly flags are required, but you can add more in your Makefile.
ASMFLAGS += $(OPT)
ASMFLAGS += $(WARN)
ASMFLAGS += -DAPP_TIMER_V2
ASMFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
ASMFLAGS += -DFLOAT_ABI_SOFT
ASMFLAGS += -DNRF52811_XXAA
ASMFLAGS += -DNRFX_COREDEP_DELAY_US_LOOP_CYCLES=3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mfloat-abi=soft
ASMFLAGS += -mthumb -mabi=aapcs

# As well as linker flags.
LDFLAGS += $(OPT)
LDFLAGS += --specs=nano.specs # Uses newlib in nano version
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mthumb -mabi=aapcs -L$(NRF_SDK_PATH)/modules/nrfx/mdk -T$(LINKER_FILE)
LDFLAGS += -u _printf_float # This allows us to print floats with printf
LDFLAGS += -Wl,--gc-sections # Let's the linker dump unused sections

# Here we set the stack and heap.
$(PROJECT_NAME): CFLAGS += -D__HEAP_SIZE=2048
$(PROJECT_NAME): CFLAGS += -D__STACK_SIZE=2048
$(PROJECT_NAME): ASMFLAGS += -D__HEAP_SIZE=2048
$(PROJECT_NAME): ASMFLAGS += -D__STACK_SIZE=2048

# Standard libraries are added at the end of the linker input.
LIB_FILES += -lc -lnosys -lm

# Final bit of magic happens in the nRF SDK common makefile.
TEMPLATE_PATH := $(NRF_SDK_PATH)/components/toolchain/gcc
TARGETS := $(PROJECT_NAME)
include $(TEMPLATE_PATH)/Makefile.common
$(foreach target, $(TARGETS), $(call define_target, $(target)))

# Always recompile s1.c to include the latest date, time and version stamps.
$(shell touch $(S1_SDK_PATH)/s1.c)


# Below are the standard build tasks. You can add more in your own Makefile.

# This line tells make that "default", "flash", etc. aren't files, but recipes
.PHONY: default flash erase reset clean

# "make" simply builds the project
default: $(PROJECT_NAME)

# "make flash" will use the nrfjprog tool to flash the nRF chip using a JLink
flash: $(PROJECT_NAME)
	nrfjprog -f nrf52 --program $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex --sectorerase -r

# "make erase" fully erases the nRF chip
erase:
	nrfjprog -f nrf52 --eraseall

# "make reset" will reset the nRF chip
reset:
	nrfjprog -f nrf52 -r