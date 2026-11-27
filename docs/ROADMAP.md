# Roadmap

The roadmap is milestone-based. A milestone is complete only when its acceptance checks pass and its status is updated here.

## M0 — repository and toolchain

- [x] Establish repository structure and coding conventions.
- [x] Add an x86_64 UEFI console application.
- [x] Add a separate freestanding ELF64 kernel build.
- [x] Add host-side tests and CI configuration.
- [ ] Verify build and QEMU launch on a clean external machine.

## M1 — UEFI loader and kernel handoff

- [ ] Locate the kernel image on the EFI system partition.
- [ ] Read the complete kernel through UEFI file protocols.
- [ ] Validate ELF64 headers and program-header bounds.
- [ ] Allocate and load PT_LOAD segments with overflow and overlap checks.
- [ ] Discover GOP framebuffer and ACPI RSDP configuration table.
- [ ] Capture the final UEFI memory map and safely retry GetMemoryMap/ExitBootServices.
- [ ] Populate the versioned boot-info structure.
- [ ] Allocate and align a kernel stack.
- [ ] Transfer control using the documented x86_64 ABI.
- [ ] Verify the first kernel serial message in QEMU.

## M2 — CPU and memory foundation

- [ ] Establish GDT and IDT.
- [ ] Add exception handlers and panic diagnostics.
- [ ] Implement physical page allocation from the UEFI memory map.
- [ ] Define virtual-memory layout and kernel heap allocation.
- [ ] Add allocator and address-range tests.

## M3 — interrupts and time

- [ ] Add interrupt-controller abstraction and timer source.
- [ ] Add interrupt-safe synchronization primitives.
- [ ] Measure and log timer ticks in QEMU.

## M4 — execution model

- [ ] Add kernel threads and context switching.
- [ ] Add a deterministic scheduler test.
- [ ] Define process and address-space ownership.
- [ ] Add user-mode transition and syscall entry/exit.

## M5 — devices and storage

- [ ] Enumerate PCI devices.
- [ ] Define driver registration and device lifetimes.
- [ ] Add basic block-device support.
- [ ] Implement VFS and an initial read-only filesystem.
- [ ] Add an in-memory initramfs format and tooling.

## M6 — userspace and interface

- [ ] Start a first userspace init process.
- [ ] Add a small command shell and system utilities.
- [ ] Add a framebuffer text console.
- [ ] Define graphics/input boundaries before beginning a desktop environment.

## Quality gates

- Every loader input is bounds-checked before dereferencing.
- All address and size arithmetic is checked for overflow.
- No UEFI boot service is used after successful ExitBootServices.
- Kernel code builds without a hosted C runtime.
- Each milestone has reproducible build steps and test evidence.
- Documentation never describes planned code as implemented.
