#include <fanstik/arch/gdt.h>
#include <fanstik/arch/idt.h>

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idt_ptr;

/* Exception stubs defined in exceptions.S. */
extern void exception_stub_0(void);
extern void exception_stub_1(void);
extern void exception_stub_2(void);
extern void exception_stub_3(void);
extern void exception_stub_4(void);
extern void exception_stub_5(void);
extern void exception_stub_6(void);
extern void exception_stub_7(void);
extern void exception_stub_8(void);
extern void exception_stub_9(void);
extern void exception_stub_10(void);
extern void exception_stub_11(void);
extern void exception_stub_12(void);
extern void exception_stub_13(void);
extern void exception_stub_14(void);
extern void exception_stub_15(void);
extern void exception_stub_16(void);
extern void exception_stub_17(void);
extern void exception_stub_18(void);
extern void exception_stub_19(void);
extern void exception_stub_20(void);
extern void exception_stub_21(void);
extern void exception_stub_22(void);
extern void exception_stub_23(void);
extern void exception_stub_24(void);
extern void exception_stub_25(void);
extern void exception_stub_26(void);
extern void exception_stub_27(void);
extern void exception_stub_28(void);
extern void exception_stub_29(void);
extern void exception_stub_30(void);
extern void exception_stub_31(void);

static void *exception_stubs[32] = {
    exception_stub_0,  exception_stub_1,  exception_stub_2,  exception_stub_3,
    exception_stub_4,  exception_stub_5,  exception_stub_6,  exception_stub_7,
    exception_stub_8,  exception_stub_9,  exception_stub_10, exception_stub_11,
    exception_stub_12, exception_stub_13, exception_stub_14, exception_stub_15,
    exception_stub_16, exception_stub_17, exception_stub_18, exception_stub_19,
    exception_stub_20, exception_stub_21, exception_stub_22, exception_stub_23,
    exception_stub_24, exception_stub_25, exception_stub_26, exception_stub_27,
    exception_stub_28, exception_stub_29, exception_stub_30, exception_stub_31,
};

void idt_set_entry(uint8_t num, uint64_t handler, uint16_t selector,
                   uint8_t ist, uint8_t type_attr)
{
    idt[num].offset_low = (uint16_t)(handler & 0xFFFFu);
    idt[num].selector = selector;
    idt[num].ist = ist;
    idt[num].type_attr = type_attr;
    idt[num].offset_mid = (uint16_t)((handler >> 16) & 0xFFFFu);
    idt[num].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFFu);
    idt[num].reserved = 0;
}

void idt_init(void)
{
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_entry(i, 0, 0, 0, 0);
    }

    /* Set up the first 32 CPU exception handlers. Exceptions with error
     * codes: 8, 10, 11, 12, 13, 14, 17, 20, 30. The stubs already push the
     * vector number in RDI and error code in RSI, so the type attribute is
     * a normal 64-bit interrupt gate. */
    for (int i = 0; i < 32; i++) {
        idt_set_entry((uint8_t)i, (uint64_t)(uintptr_t)exception_stubs[i],
                      GDT_SEL_KERNEL_CODE, 0, IDT_TYPE_INTERRUPT_GATE);
    }

    idt_ptr.limit = (uint16_t)(sizeof(idt) - 1);
    idt_ptr.base = (uint64_t)(uintptr_t)idt;

    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}
