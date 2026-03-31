# ESP32 Bluepad32 -> Pro Micro XID bridge (project skeleton)

This project contains a practical v1 split in two firmwares:

- `esp32_bluepad32/`: ESP32 firmware using Bluepad32 to receive Bluetooth gamepads and send a compact binary frame over UART.
- `pro_micro_xid/`: ATmega32U4 / Pro Micro firmware using LUFA + avr-gcc to present a vendor-specific/XID-style USB device to the original Xbox and translate frames from the ESP32.

## Status

This is a **developer-grade scaffold** focused on architecture, latency and clean separation of responsibilities.

- The ESP32 side is close to drop-in for Arduino + Bluepad32.
- The Pro Micro side is a **serious starting point** for LUFA/AVR development, but you should still expect bench testing and descriptor tuning against a real Xbox.
- USB descriptor IDs are left as configurable placeholders so you can replace them with values validated from a known-good implementation.

## Wiring

### UART link

- ESP32 TX -> Pro Micro RX1
- ESP32 RX -> Pro Micro TX1 (optional for rumble / feedback)
- GND -> GND

If your Pro Micro is a **5V** board, protect the ESP32 RX input from the Pro Micro TX level.

## Binary bridge packet

The packet is fixed-size and sends the full state, not deltas:

- magic / version / sequence / flags
- dpad / buttons / misc buttons
- brake / throttle
- lx / ly / rx / ry
- crc8

That makes recovery from drops or reconnection simpler and avoids state drift.

## Build notes

### ESP32

Use Arduino + Bluepad32 on an **original ESP32** if you want the best controller compatibility, especially for controllers that need Bluetooth Classic / BR/EDR.

### Pro Micro

Build with `avr-gcc` + LUFA. The provided `Makefile` assumes LUFA is available in `../LUFA` relative to the Pro Micro project root.

## What to validate on hardware

1. USB enumeration against the Xbox.
2. Correct descriptor / control request behavior.
3. Raw Bluepad32 button mask assumptions for your controllers.
4. Stick polarity and analog scaling.
5. Rumble return path.

