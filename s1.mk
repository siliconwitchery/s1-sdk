# PROJECT_NAME     := s1_module_test
TARGETS          := nrf52811_xxaa
OUTPUT_DIRECTORY := .build

ifeq ($(S1_SDK_ROOT),)
S1_SDK_ROOT := .
endif

include $(S1_SDK_ROOT)/paths.mk

$(OUTPUT_DIRECTORY)/nrf52811_xxaa.out: \
  LINKER_SCRIPT  := $(S1_SDK_ROOT)/nrf52811.ld

# Source files common to all targets
SRC_FILES += \
  $(S1_SDK_ROOT)/s1.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_saadc.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spim.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_twim.c \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52811.S \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52811.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52811.c \
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(S1_SDK_ROOT) \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/components/libraries/ringbuf \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/external/fprintf \
  $(SDK_ROOT)/external/segger_rtt \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/modules/nrfx/mdk \

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT += -flto # Link time optimisation
OPT += -g3 -Og # Debugging information

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DFLOAT_ABI_SOFT
CFLAGS += -DNRF52811_XXAA
CFLAGS += -DNRFX_COREDEP_DELAY_US_LOOP_CYCLES=3
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall
CFLAGS += -mfloat-abi=soft
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# C++ flags common to all targets
CXXFLAGS += $(OPT)

# Assembler flags common to all targets
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=soft
ASMFLAGS += -DFLOAT_ABI_SOFT
ASMFLAGS += -DNRF52811_XXAA
ASMFLAGS += -DNRFX_COREDEP_DELAY_US_LOOP_CYCLES=3

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -Wl,--gc-sections # let linker dump unused sections
LDFLAGS += --specs=nano.specs # use newlib in nano version
LDFLAGS += -u _printf_float # required to print floats

nrf52811_xxaa: CFLAGS += -D__HEAP_SIZE=2048
nrf52811_xxaa: CFLAGS += -D__STACK_SIZE=2048
nrf52811_xxaa: ASMFLAGS += -D__HEAP_SIZE=2048
nrf52811_xxaa: ASMFLAGS += -D__STACK_SIZE=2048

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

# Always recomple main.c to include latest __DATE__ and __TIME__ 
$(shell touch main.c)

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: default flash erase reset help

default: nrf52811_xxaa

flash: nrf52811_xxaa
	nrfjprog -f nrf52 --program $(OUTPUT_DIRECTORY)/nrf52811_xxaa.hex --sectorerase -r

erase:
	nrfjprog -f nrf52 --eraseall

clean:
	find . -type d -name '.build' -exec rm -r {} +

reset:
	nrfjprog -f nrf52 -r

sdk_config:
	java -jar $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar sdk_config.h

help:
	@echo Usage
	@echo "  make            - Make the application"
	@echo "  make flash      - Make and flash the application to chip"
	@echo "  make clean      - Clean all the existing build files"
	@echo "  make erase      - Fully erase the chip"
	@echo "  make reset      - Reset the chip"
	@echo "  make sdk_config - External tool for editing sdk_config.h"