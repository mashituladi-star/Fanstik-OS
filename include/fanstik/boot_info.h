#ifndef FANSTIK_BOOT_INFO_H
#define FANSTIK_BOOT_INFO_H
#include <stdbool.h>
#include <stdint.h>
#define FANSTIK_BOOT_INFO_MAGIC UINT64_C(0x46414E5354494B31)
#define FANSTIK_BOOT_INFO_VERSION UINT32_C(1)
enum fanstik_pixel_format { FANSTIK_PIXEL_FORMAT_UNKNOWN=0, FANSTIK_PIXEL_FORMAT_RGB=1, FANSTIK_PIXEL_FORMAT_BGR=2, FANSTIK_PIXEL_FORMAT_BITMASK=3 };
/* Versioned loader-to-kernel ABI. Addresses are physical until VM setup. */
struct fanstik_boot_info {
    uint64_t magic; uint32_t version; uint32_t struct_size;
    uint64_t framebuffer_base; uint32_t framebuffer_width; uint32_t framebuffer_height;
    uint32_t pixels_per_scanline; uint32_t pixel_format;
    uint64_t memory_map; uint64_t memory_map_size; uint64_t memory_descriptor_size;
    uint32_t memory_descriptor_version; uint32_t reserved; uint64_t acpi_rsdp;
};
bool fanstik_boot_info_valid(const struct fanstik_boot_info *info);
#endif
