#ifndef FANSTIK_PANIC_H
#define FANSTIK_PANIC_H
__attribute__((noreturn)) void kernel_panic(const char *message);
#endif
