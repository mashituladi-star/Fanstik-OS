#include <fanstik/arch/idt.h>
#include <fanstik/panic.h>
#include <fanstik/serial.h>

static const char *exception_names[32] = {
    "Divide Error",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved",
};

static void print_hex64(uint64_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; i++) {
        buf[2 + i] = hex[(value >> (60 - i * 4)) & 0xF];
    }
    buf[18] = '\0';
    serial_write_string(buf);
}

void exception_handler_common(uint8_t vector, uint64_t error_code)
{
    __asm__ volatile("cli");
    serial_write_string("\r\n[FANSTIK EXCEPTION] ");
    if (vector < 32u) {
        serial_write_string(exception_names[vector]);
    } else {
        serial_write_string("Unknown");
    }
    serial_write_string(" (vector ");
    serial_write_string("0x");
    print_hex64((uint64_t)vector);
    serial_write_string(")");
    if (vector == 8u || vector == 10u || vector == 11u || vector == 12u ||
        vector == 13u || vector == 14u || vector == 17u || vector == 20u ||
        vector == 30u) {
        serial_write_string(", error code: 0x");
        print_hex64(error_code);
    }
    serial_write_string("\r\n");
    kernel_panic("unhandled CPU exception");
}
