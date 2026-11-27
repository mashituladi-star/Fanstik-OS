#include <fanstik/panic.h>
#include <fanstik/serial.h>

__attribute__((noreturn)) void kernel_panic(const char *message)
{
    serial_write_string("\r\n[FANSTIK PANIC] ");
    serial_write_string(message != 0 ? message : "unspecified failure");
    serial_write_string("\r\nSystem halted.\r\n");
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
