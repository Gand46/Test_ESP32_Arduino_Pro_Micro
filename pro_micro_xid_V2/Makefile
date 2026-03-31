MCU          = atmega32u4
ARCH         = AVR8
BOARD        = USER
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = pro_micro_xid_bridge
SRC          = src/main.c src/Descriptors.c src/UART.c src/Bridge.c src/BridgeProtocol.c $(LUFA_SRC_USB)

LUFA_PATH    ?= ../LUFA/LUFA
ifeq ($(wildcard $(LUFA_PATH)/Build/LUFA/lufa-sources.mk),)
LUFA_PATH    := ../../LUFA/LUFA
endif

LUFA_OPTS    = USB_DEVICE_ONLY

CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -DUSE_FLASH_DESCRIPTORS -IConfig/ -Isrc -I. -std=gnu11 -Wall -Wextra -Werror
LD_FLAGS     =

AVRDUDE_PROGRAMMER = avr109
AVRDUDE_PORT       = /dev/ttyACM0
AVRDUDE_FLAGS      = -V -q -q

# Default target
all:

.PHONY: build_txt
build_txt: all
	@mkdir -p build
	@for ext in elf hex bin eep map lss sym; do \
		if [ -f $(TARGET).$$ext ]; then \
			cp $(TARGET).$$ext build/$(TARGET).$$ext.txt; \
		fi; \
	done

DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/atprogram.mk
