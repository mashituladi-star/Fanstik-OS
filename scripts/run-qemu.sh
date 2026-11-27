#!/usr/bin/env sh
set -e
ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="$ROOT_DIR/build"
ESP_IMG="$BUILD_DIR/esp.img"
QEMU_BIN=qemu-system-x86_64
if [ -n "$QEMU" ]; then QEMU_BIN=$QEMU; fi
if [ ! -f "$BUILD_DIR/BOOTX64.EFI" ]; then echo "error: run make bootloader first" >&2; exit 1; fi
if [ ! -f "$BUILD_DIR/kernel.elf" ]; then echo "error: run make kernel first" >&2; exit 1; fi
if ! command -v "$QEMU_BIN" >/dev/null 2>&1; then echo "error: qemu-system-x86_64 not found" >&2; exit 1; fi
find_first_file() { for candidate in "$@"; do if [ -n "$candidate" ] && [ -f "$candidate" ]; then printf '%s\n' "$candidate"; return 0; fi; done; return 1; }
if [ -z "$OVMF_CODE" ]; then OVMF_CODE=$(find_first_file /usr/share/OVMF/OVMF_CODE.fd /usr/share/OVMF/OVMF_CODE_4M.fd /usr/share/edk2/x64/OVMF_CODE.fd /usr/share/qemu/OVMF_CODE.fd || true); fi
if [ -z "$OVMF_CODE" ] || [ ! -f "$OVMF_CODE" ]; then echo "error: set OVMF_CODE to an OVMF firmware code image" >&2; exit 1; fi
if [ -z "$OVMF_VARS" ]; then OVMF_VARS=$(find_first_file /usr/share/OVMF/OVMF_VARS.fd /usr/share/OVMF/OVMF_VARS_4M.fd /usr/share/edk2/x64/OVMF_VARS.fd /usr/share/qemu/OVMF_VARS.fd || true); fi
if [ -z "$OVMF_VARS" ] || [ ! -f "$OVMF_VARS" ]; then echo "error: set OVMF_VARS to an OVMF variable-store template" >&2; exit 1; fi
python3 "$ROOT_DIR/scripts/make_esp.py" "$ESP_IMG" "$BUILD_DIR/BOOTX64.EFI" "$BUILD_DIR/kernel.elf"
cp "$OVMF_VARS" "$BUILD_DIR/OVMF_VARS.fd"
exec "$QEMU_BIN" -machine q35 -m 256M -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" -drive if=pflash,format=raw,file="$BUILD_DIR/OVMF_VARS.fd" -drive format=raw,file="$ESP_IMG" -serial stdio -monitor none -net none
