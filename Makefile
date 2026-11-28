SHELL := /bin/sh

CLANG ?= clang
LLD_LINK ?= lld-link
LD_LLD ?= ld.lld
HOST_CC ?= cc
BUILD_DIR := build
KERNEL_CFLAGS := --target=x86_64-unknown-none-elf -std=c11 -O2 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-builtin -fno-pic -fno-pie -mno-red-zone -mcmodel=small
UEFI_CFLAGS := --target=x86_64-pc-win32-coff -std=c11 -Oz -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-builtin -fshort-wchar -mno-red-zone -Iinclude -Iboot/uefi
KERNEL_OBJECTS := $(BUILD_DIR)/entry.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/boot_info.o $(BUILD_DIR)/serial.o $(BUILD_DIR)/panic.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/exceptions.o $(BUILD_DIR)/exceptions_c.o $(BUILD_DIR)/pmm.o
UEFI_OBJECTS := $(BUILD_DIR)/uefi_main.obj $(BUILD_DIR)/loader.obj $(BUILD_DIR)/elf64.obj $(BUILD_DIR)/serial.obj
.PHONY: all bootloader kernel test test-esp test-pmm run clean check-tools
all: bootloader kernel
check-tools:
	@command -v $(CLANG) >/dev/null 2>&1 || { echo "error: clang was not found"; exit 1; }
	@command -v $(LLD_LINK) >/dev/null 2>&1 || { echo "error: lld-link was not found"; exit 1; }
	@command -v $(LD_LLD) >/dev/null 2>&1 || { echo "error: ld.lld was not found"; exit 1; }
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(BUILD_DIR)/uefi_main.obj: boot/uefi/main.c boot/uefi/uefi.h boot/uefi/loader.h | $(BUILD_DIR)
	$(CLANG) $(UEFI_CFLAGS) -c $< -o $@
$(BUILD_DIR)/loader.obj: boot/uefi/loader.c boot/uefi/uefi.h boot/uefi/elf64.h boot/uefi/loader.h include/fanstik/boot_info.h | $(BUILD_DIR)
	$(CLANG) $(UEFI_CFLAGS) -c $< -o $@
$(BUILD_DIR)/elf64.obj: boot/uefi/elf64.c boot/uefi/uefi.h boot/uefi/elf64.h | $(BUILD_DIR)
	$(CLANG) $(UEFI_CFLAGS) -c $< -o $@
$(BUILD_DIR)/serial.obj: boot/uefi/serial.c boot/uefi/uefi.h | $(BUILD_DIR)
	$(CLANG) $(UEFI_CFLAGS) -c $< -o $@
$(BUILD_DIR)/BOOTX64.EFI: $(UEFI_OBJECTS)
	$(LLD_LINK) /nologo /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:x64 /timestamp:0 /out:$@ $(UEFI_OBJECTS)
bootloader: check-tools $(BUILD_DIR)/BOOTX64.EFI
$(BUILD_DIR)/entry.o: kernel/arch/x86_64/entry.S | $(BUILD_DIR)
	$(CLANG) --target=x86_64-unknown-none-elf -ffreestanding -fno-pic -c $< -o $@
$(BUILD_DIR)/kernel.o: kernel/core/kernel.c include/fanstik/boot_info.h include/fanstik/serial.h include/fanstik/panic.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/boot_info.o: kernel/core/boot_info.c include/fanstik/boot_info.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/serial.o: kernel/drivers/serial.c include/fanstik/serial.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/panic.o: kernel/core/panic.c include/fanstik/serial.h include/fanstik/panic.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/gdt.o: kernel/arch/x86_64/gdt.c include/fanstik/arch/gdt.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/idt.o: kernel/arch/x86_64/idt.c include/fanstik/arch/gdt.h include/fanstik/arch/idt.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/exceptions.o: kernel/arch/x86_64/exceptions.S | $(BUILD_DIR)
	$(CLANG) --target=x86_64-unknown-none-elf -ffreestanding -fno-pic -c $< -o $@
$(BUILD_DIR)/exceptions_c.o: kernel/arch/x86_64/exceptions.c include/fanstik/arch/idt.h include/fanstik/panic.h include/fanstik/serial.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/pmm.o: kernel/mm/pmm.c include/fanstik/mm/pmm.h include/fanstik/boot_info.h include/fanstik/serial.h | $(BUILD_DIR)
	$(CLANG) $(KERNEL_CFLAGS) -Iinclude -c $< -o $@
$(BUILD_DIR)/kernel.elf: $(KERNEL_OBJECTS) linker/kernel.ld
	$(LD_LLD) -nostdlib -static -z max-page-size=0x1000 -T linker/kernel.ld -o $@ $(KERNEL_OBJECTS)
kernel: check-tools $(BUILD_DIR)/kernel.elf
test: test-esp test-pmm | $(BUILD_DIR)
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Iinclude tests/test_boot_info.c kernel/core/boot_info.c -o $(BUILD_DIR)/test_boot_info
	$(BUILD_DIR)/test_boot_info
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Iinclude tests/test_pmm.c kernel/mm/pmm.c -o $(BUILD_DIR)/test_pmm
	$(BUILD_DIR)/test_pmm
test-esp: | $(BUILD_DIR)
	python3 tests/test_esp_image.py
test-pmm: | $(BUILD_DIR)
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Iinclude tests/test_pmm.c kernel/mm/pmm.c -o $(BUILD_DIR)/test_pmm
	$(BUILD_DIR)/test_pmm
run: all
	./scripts/run-qemu.sh
clean:
	rm -rf $(BUILD_DIR)
