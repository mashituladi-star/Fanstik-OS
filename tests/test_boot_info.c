#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fanstik/boot_info.h>

static struct fanstik_boot_info valid_info(void)
{
    struct fanstik_boot_info info;
    memset(&info, 0, sizeof(info));
    info.magic = FANSTIK_BOOT_INFO_MAGIC;
    info.version = FANSTIK_BOOT_INFO_VERSION;
    info.struct_size = (uint32_t)sizeof(info);
    return info;
}

int main(void)
{
    struct fanstik_boot_info info = valid_info();
    assert(fanstik_boot_info_valid(0) == false);
    assert(fanstik_boot_info_valid(&info) == true);
    info.magic = 0;
    assert(fanstik_boot_info_valid(&info) == false);
    info = valid_info();
    info.version++;
    assert(fanstik_boot_info_valid(&info) == false);
    info = valid_info();
    info.struct_size = 1;
    assert(fanstik_boot_info_valid(&info) == false);
    info = valid_info();
    info.framebuffer_base = 0x100000;
    info.framebuffer_width = 1024;
    info.framebuffer_height = 768;
    info.pixels_per_scanline = 1000;
    assert(fanstik_boot_info_valid(&info) == false);
    info.pixels_per_scanline = 1024;
    assert(fanstik_boot_info_valid(&info) == true);
    info = valid_info();
    info.memory_map = 0x200000;
    info.memory_map_size = 128;
    info.memory_descriptor_size = 48;
    assert(fanstik_boot_info_valid(&info) == true);
    puts("boot-info tests: all checks passed");
    return 0;
}
