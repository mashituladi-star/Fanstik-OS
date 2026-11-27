# Architecture overview

## Design constraints

Fanstik-OS targets x86_64 machines booted through UEFI. Legacy BIOS is out of scope. The operating system is an independent implementation; it does not use the Linux kernel or inherit a Linux userspace.

Target operating-system code is freestanding C and x86_64 Assembly. Host-side tooling and CI scripts may use shell and YAML.

## UEFI application

boot/uefi is built as a PE/COFF x86_64 EFI application. Firmware enters efi_main with an image handle and system table. The current application checks the console output interface and prints a bring-up report. It does not yet load the kernel, parse ELF, obtain the final memory map, or call ExitBootServices.

## Kernel image

kernel/ is built as a freestanding ELF64 image linked at 0x00200000 for the initial identity-mapped bring-up design. The linker script separates executable and writable load segments. A future loader must provide a valid stack, preserve the boot-info pointer in RDI, and map the kernel at its linked addresses before jumping to _start.

## Boot information ABI

include/fanstik/boot_info.h defines a versioned structure for the future loader-to-kernel handoff. Addresses are physical until a virtual-memory policy is implemented. Do not reorder existing fields without introducing a new version or compatibility plan.

## Early diagnostics

The kernel serial driver targets COM1 (I/O base 0x3F8). It uses bounded polling to avoid waiting forever for the transmitter. Serial output is intended for QEMU logs and early hardware diagnostics.

## Planned subsystem boundaries

- arch/x86_64: CPU setup, GDT, IDT, exception entry, context switching.
- mm: physical page allocator, virtual address-space management, kernel heap.
- interrupts: exception handlers, PIC/APIC routing, timer source.
- scheduler: threads, runnable queues, context switching.
- syscall: user/kernel ABI and privilege transitions.
- drivers: console, storage, input, PCI enumeration, timers.
- fs: VFS and initial in-memory filesystem.
- userspace: init process, shell, system utilities.
- graphics: framebuffer console first, compositor/window system later.

These are planned boundaries, not implemented subsystems.
