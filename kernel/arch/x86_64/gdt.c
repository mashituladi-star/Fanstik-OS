#include <fanstik/arch/gdt.h>

struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr gdt_ptr;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t granularity)
{
    gdt[index].limit_low = (uint16_t)(limit & 0xFFFFu);
    gdt[index].base_low = (uint16_t)(base & 0xFFFFu);
    gdt[index].base_middle = (uint8_t)((base >> 16) & 0xFFu);
    gdt[index].access = access;
    gdt[index].granularity = granularity;
    gdt[index].base_high = (uint8_t)((base >> 24) & 0xFFu);
}

void gdt_init(void)
{
    /* 0: null descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);
    /* 1: kernel code segment, base=0, limit=4GiB, executable, readable, ring 0 */
    gdt_set_entry(1, 0, 0xFFFFFu, 0x9Au, 0xA0u);
    /* 2: kernel data segment, base=0, limit=4GiB, writable, ring 0 */
    gdt_set_entry(2, 0, 0xFFFFFu, 0x92u, 0xC0u);

    gdt_ptr.limit = (uint16_t)(sizeof(gdt) - 1);
    gdt_ptr.base = (uint64_t)(uintptr_t)gdt;

    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));
    /* In x86-64 long mode, DS/ES/SS are ignored for addressing, but loading
     * them keeps the state consistent. CS is fixed by the CPU and is not
     * changed by a far jump. */
    __asm__ volatile("mov %0, %%ds" : : "r"(GDT_SEL_KERNEL_DATA));
    __asm__ volatile("mov %0, %%es" : : "r"(GDT_SEL_KERNEL_DATA));
    __asm__ volatile("mov %0, %%ss" : : "r"(GDT_SEL_KERNEL_DATA));
}
