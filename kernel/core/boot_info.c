#include <fanstik/boot_info.h>

bool fanstik_boot_info_valid(const struct fanstik_boot_info *info)
{
    if (info == 0 || info->magic != FANSTIK_BOOT_INFO_MAGIC ||
        info->version != FANSTIK_BOOT_INFO_VERSION ||
        info->struct_size < sizeof(*info)) {
        return false;
    }
    if (info->framebuffer_base != 0 &&
        (info->framebuffer_width == 0 || info->framebuffer_height == 0 ||
         info->pixels_per_scanline < info->framebuffer_width)) {
        return false;
    }
    if ((info->memory_map == 0) != (info->memory_map_size == 0)) {
        return false;
    }
    if (info->memory_map_size != 0 && info->memory_descriptor_size < 40) {
        return false;
    }
    return true;
}
