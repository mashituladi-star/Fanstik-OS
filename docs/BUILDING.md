# Building and running

## Build dependencies

- Clang targeting x86_64-pc-win32-coff and x86_64-unknown-none-elf
- lld-link and ld.lld
- GNU Make
- A host C compiler for unit tests
- For emulator runs: qemu-system-x86_64 and OVMF firmware code/variable images

No Linux distribution runtime is used by the target. The host operating system is only a development environment.

## Build

From the repository root:

~~~sh
make
make test
~~~

Artifacts:

- build/BOOTX64.EFI: removable-media x86_64 UEFI application.
- build/kernel.elf: freestanding ELF64 kernel image, built separately.

The UEFI app and kernel are deliberately separate at this milestone. The current app does not load or execute kernel.elf.

## Run under QEMU

Set OVMF_CODE to the path of the firmware code image. If OVMF_VARS is not in a standard location, set that variable too.

~~~sh
OVMF_CODE=/usr/share/OVMF/OVMF_CODE.fd make run
~~~

The launcher stages the EFI binary under EFI/BOOT/BOOTX64.EFI and starts QEMU with a writable copy of the firmware variable store. The UEFI application writes its progress report to the firmware console.

## Troubleshooting

- If clang is missing, install a Clang/LLVM toolchain.
- If lld-link or ld.lld is missing, install the LLVM linker tools.
- If QEMU cannot be found, install the x86_64 system emulator package.
- If OVMF firmware cannot be found, set OVMF_CODE and OVMF_VARS explicitly.
- If the host C compiler is not named cc, pass HOST_CC=your-compiler to make test.

## Clean build

~~~sh
make clean
make
make test
~~~
