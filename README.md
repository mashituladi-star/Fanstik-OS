# Fanstik-OS

**An independent x86_64 operating-system project written in C and Assembly.**

Fanstik-OS is being developed from the ground up for UEFI systems. It is not a Linux distribution, a Linux fork, or a re-skinned existing operating system. The long-term goal is to build an independently designed kernel, boot process, memory manager, drivers, filesystem, userspace, and graphical environment.

> **Development status: early bring-up scaffold.** The repository currently targets x86_64 UEFI and contains a PE/COFF UEFI console application, a separately built freestanding ELF kernel, a shared boot-information ABI, serial diagnostics, host-side tests, and a QEMU launch script. The UEFI application and kernel are not yet connected: the loader does not load the ELF kernel or transfer control to it. Do not treat this milestone as a usable operating system.

## Principles

- **Independent implementation:** no Linux kernel, distribution base, or copied OS runtime.
- **C and Assembly:** freestanding C for the kernel and UEFI application; x86_64 Assembly for the low-level entry path.
- **UEFI first:** x86_64 UEFI is the only supported boot mode in this project. Legacy BIOS is out of scope.
- **Small, testable milestones:** every subsystem should have a clear contract and tests where practical.
- **Honest status:** documentation distinguishes implemented, experimental, and planned features.

## Current milestone

| Component | Status |
| --- | --- |
| x86_64 UEFI application entry | Implemented |
| Firmware console output | Implemented |
| Freestanding ELF kernel build | Implemented |
| Kernel assembly entry stub | Implemented |
| COM1 serial diagnostics | Implemented |
| Shared boot information structure | Implemented |
| Host-side boot-info tests | Implemented |
| QEMU + OVMF launch script | Scaffolded; requires local firmware and QEMU |
| UEFI ELF loading and kernel handoff | Not implemented |
| Physical memory manager / paging | Not implemented |
| Interrupts / scheduler / userspace | Planned |

## Repository layout

~~~text
.
├── .github/workflows/ci.yml
├── boot/uefi/                 # Minimal x86_64 UEFI application
├── docs/                      # Architecture, build guide, roadmap
├── include/fanstik/            # Shared freestanding interfaces
├── kernel/
│   ├── arch/x86_64/            # Assembly entry and architecture code
│   ├── core/                   # Kernel core and boot-info validation
│   └── drivers/                # Early serial driver
├── linker/                    # Kernel ELF linker script
├── scripts/                   # Local build / QEMU helpers
├── tests/                     # Host-side unit tests
├── Makefile
└── README.md
~~~

## Quick start

### Build

Required tools:

- Clang/LLVM (clang, lld-link, ld.lld)
- GNU Make
- A C compiler for host-side tests (cc)

Then run:

~~~sh
make
make test
~~~

Build products are written to build/:

- BOOTX64.EFI — UEFI x64 application.
- kernel.elf — freestanding kernel ELF image; not yet loaded by the UEFI application.

### Run the UEFI application in QEMU

Install QEMU for x86_64 and an OVMF UEFI firmware image, then set OVMF_CODE to the firmware code image if it is not found in a standard location:

~~~sh
OVMF_CODE=/path/to/OVMF_CODE.fd make run
~~~

The launch script creates a temporary FAT directory image containing the removable-media boot path EFI/BOOT/BOOTX64.EFI. It boots the current UEFI application only; it does not boot the kernel yet.

## Development

Useful targets:

~~~sh
make                 # Build the UEFI application and kernel ELF
make bootloader      # Build only BOOTX64.EFI
make kernel          # Build only kernel.elf
make test            # Run host-side unit tests
make clean           # Remove build/
make run             # Launch the UEFI application in QEMU/OVMF
~~~

See docs/BUILDING.md, docs/ARCHITECTURE.md, and docs/ROADMAP.md.

## Scope and non-goals

The initial target is **x86_64 UEFI**. There is no BIOS boot path. The project does not depend on Arch Linux at runtime and is not an Arch installer. Development tools may be installed on any supported host OS; the resulting target is the Fanstik-OS image.

The project is at an early stage. No claim is made yet that it supports hardware beyond the firmware console in the UEFI application.

## License

Fanstik-OS is licensed under the MIT License. See LICENSE.
