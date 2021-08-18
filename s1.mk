# Base S1 SDK files. Include this in your project's "Makefile"
# to automatically include dependences for the S1 functions and
# APIs.

ifeq ($(S1_SDK_ROOT),)
S1_SDK_ROOT := .
endif

# include $(S1_SDK_ROOT)/paths.mk

LINKER_SCRIPT := $(S1_SDK_ROOT)/s1.ld

SRC_FILES += $(S1_SDK_ROOT)/s1.c

INC_FOLDERS += $(S1_SDK_ROOT)