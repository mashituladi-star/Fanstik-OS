#include <stdint.h>
#include <fanstik/serial.h>

#define COM1_BASE 0x3F8u
static inline void port_out8(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint8_t port_in8(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
void serial_init(void)
{
    port_out8(COM1_BASE + 1u, 0x00u);
    port_out8(COM1_BASE + 3u, 0x80u);
    port_out8(COM1_BASE + 0u, 0x03u);
    port_out8(COM1_BASE + 1u, 0x00u);
    port_out8(COM1_BASE + 3u, 0x03u);
    port_out8(COM1_BASE + 2u, 0xC7u);
    port_out8(COM1_BASE + 4u, 0x0Bu);
}
void serial_write_char(char value)
{
    unsigned int timeout = 100000u;
    while ((port_in8(COM1_BASE + 5u) & 0x20u) == 0u && timeout > 0u) {
        --timeout;
    }
    if (timeout > 0u) {
        port_out8(COM1_BASE, (uint8_t)value);
    }
}
void serial_write_string(const char *value)
{
    if (value == 0) return;
    while (*value != '\0') {
        serial_write_char(*value++);
    }
}
