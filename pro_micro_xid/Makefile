MCU          = atmega32u4
ARCH         = AVR8
BOARD        = USER
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = pro_micro_xid_bridge
SRC          = src/main.c src/Descriptors.c src/UART.c src/Bridge.c src/BridgeProtocol.c

LUFA_PATH    = ../LUFA/LUFA
LUFA_OPTS    = -DUSB_DEVICE_ONLY

CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/ -Isrc -I. -std=gnu11 -Wall -Wextra -Werror
LD_FLAGS     =

AVRDUDE_PROGRAMMER = avr109
AVRDUDE_PORT       = /dev/ttyACM0
AVRDUDE_FLAGS      = -V -q -q

include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
