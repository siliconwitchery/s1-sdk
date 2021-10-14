# Base SDK makefile.
# ------------------
#
# Copyright 2021 Silicon Witchery AB
#
# Permission to use, copy, modify, and/or distribute this 
# software for any purpose with or without fee is hereby
# granted, provided that the above copyright notice and this
# permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
# ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO 
# EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, 
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, 
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
# OF THIS SOFTWARE.
#
# ------------------


# You don't need to change anything here, but include this file in your own
# Makefile. From there you can override anything marked with ?=, or append to
# anything marked with += such as .c and .h files, or optimization flags.


# As a minimum, you only need to set these variables from you Makefile.
PROJECT_NAME ?= s1_sdk_standalone
NRF_SDK_PATH ?= ${HOME}/nRF5_SDK
S1_SDK_PATH ?= s1-sdk

# Optionally, you can change the build directory.
OUTPUT_DIRECTORY ?= .build

# You can add more source files using 'SRC_FILES += ' in your Makefile.
SRC_FILES += \
  $(S1_SDK_PATH)/s1.c \
  $(NRF_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52811.S \
  $(NRF_SDK_PATH)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(NRF_SDK_PATH)/components/libraries/atomic/nrf_atomic.c \
  $(NRF_SDK_PATH)/components/libraries/balloc/nrf_balloc.c \
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

# These C flags are required, but you can add more in your Makefile.
CFLAGS += $(OPT)
CFLAGS += -DAPP_TIMER_V2
CFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
CFLAGS += -DFLOAT_ABI_SOFT
CFLAGS += -DNRF52811_XXAA
CFLAGS += -DNRFX_COREDEP_DELAY_US_LOOP_CYCLES=3
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall
CFLAGS += -mfloat-abi=soft
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# C++ flags can be added if needed.
CXXFLAGS += $(OPT)

# These assembly flags are required, but you can add more in your Makefile.
ASMFLAGS += -DAPP_TIMER_V2
ASMFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=soft
ASMFLAGS += -DFLOAT_ABI_SOFT
ASMFLAGS += -DNRF52811_XXAA
ASMFLAGS += -DNRFX_COREDEP_DELAY_US_LOOP_CYCLES=3

# As well as linker flags.
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(NRF_SDK_PATH)/modules/nrfx/mdk -T$(S1_SDK_PATH)/s1.ld
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -Wl,--gc-sections # let linker dump unused sections
LDFLAGS += --specs=nano.specs # use newlib in nano version
LDFLAGS += -u _printf_float # required to print floats

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

# Always recomple s1.c to include the latest date, time and version stamps.
$(shell touch s1.c)

# These are the standard build tasks. 
# You can add more in your own Makefile.
.PHONY: default flash erase reset clean nrf_sdk_config

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

# "make sdk_config" will open the nRF SDK configuration utility
nrf_sdk_config:
	java -jar $(NRF_SDK_PATH)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar sdk_config.h