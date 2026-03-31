# Pro Micro XID Bridge - build notes

This project is intended to be built with:
- AVR toolchain (`avr-gcc`, `avr-libc`, `avr-objcopy`)
- LUFA source tree at `../LUFA/LUFA`
- `make`

Expected build command:

```bash
cd pro_micro_xid
make clean
make
```

Expected output artifact:
- `pro_micro_xid_bridge.hex`

Example local setup:
1. Download LUFA and place it so that `../LUFA/LUFA/Build/lufa_build.mk` exists relative to this folder.
2. Ensure `avr-gcc --version` works.
3. Run `make`.

Notes:
- The VID/PID in `src/Descriptors.c` are placeholders and may need adjustment for final hardware validation.
- XID behavior still needs verification on a real Original Xbox.
