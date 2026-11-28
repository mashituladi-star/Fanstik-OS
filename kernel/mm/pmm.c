#include <fanstik/mm/pmm.h>
#include <fanstik/serial.h>

/* End of the kernel image, provided by the linker script on target. */
extern uint8_t kernel_end[];

/* EFI_MEMORY_DESCRIPTOR copied locally so the kernel does not depend on UEFI headers. */
struct efi_memory_descriptor {
    uint32_t type;
    uint32_t pad;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
};

#define EFI_CONVENTIONAL_MEMORY 7

static uint8_t *bitmap = 0;
static uint64_t bitmap_size = 0;
static uint64_t total_pages = 0;
static uint64_t free_pages_count = 0;
static uint64_t used_pages_count = 0;

static inline uint64_t align_up(uint64_t value, uint64_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

static inline uint64_t page_index(void *addr)
{
    return ((uint64_t)(uintptr_t)addr) / PMM_PAGE_SIZE;
}

static inline void *page_addr(uint64_t index)
{
    return (void *)(uintptr_t)(index * PMM_PAGE_SIZE);
}

static inline void bitmap_set(uint64_t page)
{
    bitmap[page / 8u] |= (uint8_t)(1u << (page % 8u));
}

static inline void bitmap_clear(uint64_t page)
{
    bitmap[page / 8u] &= (uint8_t)~(1u << (page % 8u));
}

static inline bool bitmap_test(uint64_t page)
{
    return (bitmap[page / 8u] >> (page % 8u)) & 1u;
}

static void *memset_void(void *ptr, int value, size_t count)
{
    unsigned char *p = (unsigned char *)ptr;
    while (count-- > 0) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

/* Core initialisation shared by pmm_init and pmm_init_with_bitmap. */
static void pmm_init_core(const struct fanstik_boot_info *boot_info)
{
    const void *map = (const void *)(uintptr_t)boot_info->memory_map;
    uint64_t map_size = boot_info->memory_map_size;
    uint64_t desc_size = boot_info->memory_descriptor_size;
    uint64_t max_addr = 0;

    /* First pass: find the highest address. */
    for (uint64_t i = 0; i * desc_size < map_size; i++) {
        const struct efi_memory_descriptor *d =
            (const struct efi_memory_descriptor *)((const uint8_t *)map + i * desc_size);
        uint64_t end = d->physical_start + d->number_of_pages * PMM_PAGE_SIZE;
        if (end > max_addr) {
            max_addr = end;
        }
    }

    total_pages = align_up(max_addr, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;

    /* Zero the bitmap so every page starts as free. */
    memset_void(bitmap, 0, (size_t)bitmap_size);

    /* Mark the bitmap pages themselves as used. */
    uint64_t bitmap_pages = bitmap_size / PMM_PAGE_SIZE;
    uint64_t bitmap_first_page = page_index(bitmap);
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        bitmap_set(bitmap_first_page + i);
    }

    /* Second pass: mark memory map pages. Conventional memory is free,
     * everything else is reserved (used). */
    for (uint64_t i = 0; i * desc_size < map_size; i++) {
        const struct efi_memory_descriptor *d =
            (const struct efi_memory_descriptor *)((const uint8_t *)map + i * desc_size);
        uint64_t start_page = d->physical_start / PMM_PAGE_SIZE;
        bool usable = (d->type == EFI_CONVENTIONAL_MEMORY);

        for (uint64_t j = 0; j < d->number_of_pages; j++) {
            if (usable) {
                bitmap_clear(start_page + j);
            } else {
                bitmap_set(start_page + j);
            }
        }
    }

    /* Mark the kernel image as used. */
    uint64_t kernel_end_addr = (uint64_t)(uintptr_t)kernel_end;
    uint64_t kernel_start_page = 0x200000u / PMM_PAGE_SIZE;
    uint64_t kernel_pages =
        (kernel_end_addr - 0x200000u + PMM_PAGE_SIZE - 1u) / PMM_PAGE_SIZE;
    for (uint64_t i = 0; i < kernel_pages; i++) {
        uint64_t page = kernel_start_page + i;
        if (page < total_pages) {
            bitmap_set(page);
        }
    }

    /* Count free and used pages. Only count pages within total_pages. */
    free_pages_count = 0;
    used_pages_count = 0;
    for (uint64_t i = 0; i < total_pages; i++) {
        if (bitmap_test(i)) {
            used_pages_count++;
        } else {
            free_pages_count++;
        }
    }
}

void pmm_init(const struct fanstik_boot_info *boot_info)
{
    /* Place the bitmap immediately after the kernel image. */
    uint64_t bitmap_pages = 4096; /* conservative upper bound for early stage */
    uint64_t kernel_end_addr = (uint64_t)(uintptr_t)kernel_end;
    uint64_t bitmap_start = align_up(kernel_end_addr, PMM_PAGE_SIZE);
    bitmap = (uint8_t *)(uintptr_t)bitmap_start;
    bitmap_size = bitmap_pages * PMM_PAGE_SIZE;

    pmm_init_core(boot_info);
}

/* For host-side tests: set up PMM state directly without parsing a memory map. */
void pmm_test_setup(uint8_t *bitmap_buf, uint64_t bitmap_buf_size,
                    uint64_t total_pages_count)
{
    bitmap = bitmap_buf;
    bitmap_size = bitmap_buf_size;
    total_pages = total_pages_count;
    memset_void(bitmap, 0, (size_t)bitmap_size);
    free_pages_count = total_pages_count;
    used_pages_count = 0;
}

/* For host-side tests: provide a pre-allocated bitmap buffer. */
void pmm_init_with_bitmap(const struct fanstik_boot_info *boot_info,
                          uint8_t *bitmap_buf, uint64_t buf_size)
{
    bitmap = bitmap_buf;
    bitmap_size = buf_size;
    pmm_init_core(boot_info);
}

void *pmm_alloc(void)
{
    /* Page 0 is reserved on x86 (IDT / IVT area), so start from page 1. */
    for (uint64_t i = 1; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            free_pages_count--;
            used_pages_count++;
            return page_addr(i);
        }
    }
    return 0;
}

void pmm_free(void *addr)
{
    if (addr == 0) {
        return;
    }
    uint64_t page = page_index(addr);
    if (page >= total_pages) {
        return;
    }
    if (!bitmap_test(page)) {
        return;
    }
    bitmap_clear(page);
    free_pages_count++;
    used_pages_count--;
}

uint64_t pmm_total_pages(void)
{
    return total_pages;
}

uint64_t pmm_free_pages(void)
{
    return free_pages_count;
}

uint64_t pmm_used_pages(void)
{
    return used_pages_count;
}
