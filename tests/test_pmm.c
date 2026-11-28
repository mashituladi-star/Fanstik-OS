#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <fanstik/boot_info.h>
#include <fanstik/mm/pmm.h>

/* Provided by the linker script on target; dummy for host tests. */
uint8_t kernel_end[4096];

#define TEST_PAGE_SIZE 0x1000ull

int main(void)
{
    /* Total addressable pages: 256 MiB worth of pages. */
    uint64_t total_pages = (256ull * 1024ull * 1024ull) / TEST_PAGE_SIZE;
    uint8_t bitmap_buf[8192]; /* enough for ~512K pages */

    /* Set up PMM state directly: all pages are free. */
    pmm_test_setup(bitmap_buf, sizeof(bitmap_buf), total_pages);

    assert(pmm_total_pages() == total_pages);
    assert(pmm_free_pages() == total_pages);
    assert(pmm_used_pages() == 0);
    assert(pmm_free_pages() + pmm_used_pages() == pmm_total_pages());

    /* Allocate and free a single page. */
    void *page1 = pmm_alloc();
    assert(page1 != 0);
    assert(((uintptr_t)page1 & (TEST_PAGE_SIZE - 1)) == 0u);
    assert(pmm_free_pages() == total_pages - 1);
    assert(pmm_used_pages() == 1);

    pmm_free(page1);
    assert(pmm_free_pages() == total_pages);
    assert(pmm_used_pages() == 0);

    /* Allocate many pages; they must be distinct and page-aligned. */
    void *pages[100];
    for (int i = 0; i < 100; i++) {
        pages[i] = pmm_alloc();
        assert(pages[i] != 0);
        assert(((uintptr_t)pages[i] & (TEST_PAGE_SIZE - 1)) == 0u);
        for (int j = 0; j < i; j++) {
            assert(pages[i] != pages[j]);
        }
    }
    assert(pmm_free_pages() == total_pages - 100);
    assert(pmm_used_pages() == 100);

    /* Free them all. */
    for (int i = 0; i < 100; i++) {
        pmm_free(pages[i]);
    }
    assert(pmm_free_pages() == total_pages);
    assert(pmm_used_pages() == 0);

    /* Freeing NULL or an already-free page must be a safe no-op. */
    pmm_free(0);
    pmm_free(page1); /* double free of an already-freed page is ignored */
    assert(pmm_free_pages() == total_pages);
    assert(pmm_used_pages() == 0);

    printf("pmm tests: all checks passed\n");
    return 0;
}
