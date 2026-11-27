#include <fanstik/boot_info.h>
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
    serial_write_string("Early kernel initialization complete.\r\n");
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
