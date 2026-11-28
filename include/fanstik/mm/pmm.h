#ifndef FANSTIK_MM_PMM_H
#define FANSTIK_MM_PMM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <fanstik/boot_info.h>

#define PMM_PAGE_SIZE 0x1000u

void pmm_init(const struct fanstik_boot_info *boot_info);
void pmm_init_with_bitmap(const struct fanstik_boot_info *boot_info,
                          uint8_t *bitmap_buf, uint64_t buf_size);
void pmm_test_setup(uint8_t *bitmap_buf, uint64_t bitmap_buf_size,
                    uint64_t total_pages_count);
void *pmm_alloc(void);
void pmm_free(void *addr);
uint64_t pmm_total_pages(void);
uint64_t pmm_free_pages(void);
uint64_t pmm_used_pages(void);

#endif /* FANSTIK_MM_PMM_H */
