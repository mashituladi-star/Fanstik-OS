#include <fanstik/arch/gdt.h>
#include <fanstik/arch/idt.h>
#include <fanstik/boot_info.h>
#include <fanstik/mm/pmm.h>
#include <fanstik/panic.h>
#include <fanstik/serial.h>

void kernel_main(const struct fanstik_boot_info *boot_info)
{
    serial_init();
    serial_write_string("\r\nFanstik-OS kernel entry reached.\r\n");
    if (!fanstik_boot_info_valid(boot_info)) {
        kernel_panic("Invalid or missing boot information");
    }
    serial_write_string("Boot information validated.\r\n");

    gdt_init();
    serial_write_string("GDT initialized.\r\n");

    idt_init();
    serial_write_string("IDT initialized.\r\n");

    pmm_init(boot_info);
    serial_write_string("PMM initialized.\r\n");

    serial_write_string("Early kernel initialization complete.\r\n");

    /* Report memory stats. */
    serial_write_string("Total pages: ");
    {
        char buf[24];
        static const char hex[] = "0123456789ABCDEF";
        uint64_t val = pmm_total_pages();
        for (int i = 0; i < 16; i++) {
            buf[15 - i] = hex[(val >> (i * 4)) & 0xF];
        }
        buf[16] = '\0';
        serial_write_string("0x");
        serial_write_string(buf);
        serial_write_string("\r\nFree pages: ");
        val = pmm_free_pages();
        for (int i = 0; i < 16; i++) {
            buf[15 - i] = hex[(val >> (i * 4)) & 0xF];
        }
        buf[16] = '\0';
        serial_write_string("0x");
        serial_write_string(buf);
        serial_write_string("\r\n");
    }

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
