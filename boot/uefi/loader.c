#include "loader.h"
#include "elf64.h"
#include <fanstik/serial.h>

/* Path to the kernel image on the EFI System Partition. */
#define KERNEL_PATH L"\\kernel.elf"

/* Kernel stack size in pages. */
#define KERNEL_STACK_PAGES 4

/* EFI_FILE_INFO_GUID for querying file information via GetInfo. */
static EFI_GUID EFI_FILE_INFO_GUID =
    { 0x09576e92, 0x6d3f, 0x11d2, { 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } };

/* Forward declarations for small helpers. */
static EFI_STATUS open_kernel_from_any_filesystem(EFI_HANDLE image_handle,
                                                  EFI_SYSTEM_TABLE *st,
                                                  void **out_buffer,
                                                  UINTN *out_size);
static EFI_STATUS open_file_on_filesystem(EFI_SYSTEM_TABLE *st,
                                          void *fs_iface,
                                          const CHAR16 *path,
                                          void **out_buffer,
                                          UINTN *out_size);
static EFI_STATUS get_framebuffer_info(EFI_SYSTEM_TABLE *st,
                                       struct fanstik_boot_info *boot_info);
static EFI_STATUS get_acpi_rsdp(EFI_SYSTEM_TABLE *st, UINT64 *out_rsdp);
static EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *st,
                                 struct fanstik_boot_info *boot_info,
                                 UINTN *out_map_key);
static void jump_to_kernel(UINT64 entry, const struct fanstik_boot_info *boot_info,
                           void *stack_top);
static void print_status(UINTN status);

EFI_STATUS loader_load_and_boot(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *st)
{
    EFI_STATUS status;
    void *kernel_image = 0;
    UINTN kernel_size = 0;
    Elf64_Ehdr ehdr;
    const Elf64_Phdr *phdrs;
    UINTN i;
    UINTN map_key = 0;
    struct fanstik_boot_info *boot_info = 0;
    void *stack_top = 0;
    UINT64 stack_pages_addr = 0;

    serial_init();
    serial_write_string("\r\n[Fanstik-OS loader] starting.\r\n");

    /* 1. Find a filesystem and read the kernel image. */
    status = open_kernel_from_any_filesystem(image_handle, st, &kernel_image, &kernel_size);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] failed to read kernel image: ");
        print_status(status);
        serial_write_string("\r\n");
        return status;
    }
    serial_write_string("[loader] kernel image read.\r\n");

    /* 2. Validate the ELF64 header. */
    status = elf64_validate(kernel_image, kernel_size, &ehdr);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] invalid ELF64 kernel image: ");
        print_status(status);
        serial_write_string("\r\n");
        st->BootServices->FreePool(kernel_image);
        return status;
    }
    serial_write_string("[loader] ELF64 header validated.\r\n");

    /* 3. Reserve each fixed-address PT_LOAD segment before writing to it. */
    phdrs = elf64_phdrs(&ehdr, kernel_image);
    for (i = 0; i < ehdr.e_phnum; i++) {
        UINT64 segment_start;
        UINTN pages;
        EFI_STATUS segment_status;

        if (phdrs[i].p_type != PT_LOAD || phdrs[i].p_memsz == 0) {
            continue;
        }
        segment_start = phdrs[i].p_paddr & ~(EFI_PAGE_SIZE - 1);
        pages = (UINTN)((phdrs[i].p_paddr + phdrs[i].p_memsz - segment_start +
                         EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE);
        segment_status = st->BootServices->AllocatePages(AllocateAddress,
                                                          EfiLoaderData,
                                                          pages, &segment_start);
        if (segment_status != EFI_SUCCESS) {
            serial_write_string("[loader] failed to reserve kernel segment: ");
            print_status(segment_status);
            serial_write_string("\r\n");
            st->BootServices->FreePool(kernel_image);
            return segment_status;
        }
        if (phdrs[i].p_filesz != 0) {
            __builtin_memcpy((void *)(uintptr_t)phdrs[i].p_paddr,
                             (const UINT8 *)kernel_image + phdrs[i].p_offset,
                             (size_t)phdrs[i].p_filesz);
        }
        __builtin_memset((void *)(uintptr_t)(phdrs[i].p_paddr + phdrs[i].p_filesz),
                         0, (size_t)(phdrs[i].p_memsz - phdrs[i].p_filesz));
    }
    st->BootServices->FreePool(kernel_image);
    serial_write_string("[loader] kernel segments loaded.\r\n");

    /* 4. Allocate and fill the boot-info structure. */
    status = st->BootServices->AllocatePool(EfiLoaderData,
                                            sizeof(struct fanstik_boot_info),
                                            (void **)&boot_info);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] failed to allocate boot-info: ");
        print_status(status);
        serial_write_string("\r\n");
        return status;
    }
    __builtin_memset(boot_info, 0, sizeof(*boot_info));
    boot_info->magic = FANSTIK_BOOT_INFO_MAGIC;
    boot_info->version = FANSTIK_BOOT_INFO_VERSION;
    boot_info->struct_size = (UINT32)sizeof(*boot_info);

    /* 5. Framebuffer info via GOP. */
    status = get_framebuffer_info(st, boot_info);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] no framebuffer (continuing without graphics): ");
        print_status(status);
        serial_write_string("\r\n");
    }

    /* 6. ACPI RSDP pointer from configuration tables. */
    status = get_acpi_rsdp(st, &boot_info->acpi_rsdp);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] no ACPI RSDP found: ");
        print_status(status);
        serial_write_string("\r\n");
    }

    /* 7. Allocate all remaining loader-owned memory before capturing the map. */
    status = st->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData,
                                             KERNEL_STACK_PAGES,
                                             &stack_pages_addr);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] failed to allocate kernel stack: ");
        print_status(status);
        serial_write_string("\r\n");
        st->BootServices->FreePool(boot_info);
        return status;
    }
    stack_top = (void *)(uintptr_t)(stack_pages_addr +
                                    KERNEL_STACK_PAGES * EFI_PAGE_SIZE);
    /* Align stack down to 16 bytes. */
    stack_top = (void *)((uintptr_t)stack_top & ~0xFULL);
    serial_write_string("[loader] kernel stack allocated.\r\n");

    /* 8. Capture the final UEFI memory map. */
    status = get_memory_map(st, boot_info, &map_key);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] failed to get memory map: ");
        print_status(status);
        serial_write_string("\r\n");
        st->BootServices->FreePool(boot_info);
        return status;
    }
    serial_write_string("[loader] boot-info complete.\r\n");

    serial_write_string("[loader] calling ExitBootServices...\r\n");

    /* 9. The map was captured after the final allocation; no Boot Services
     * calls are made between this key and ExitBootServices. */
    status = st->BootServices->ExitBootServices(image_handle, map_key);
    if (status != EFI_SUCCESS) {
        serial_write_string("[loader] ExitBootServices failed: ");
        print_status(status);
        serial_write_string("\r\n");
        st->BootServices->FreePool(boot_info);
        return status;
    }

    serial_write_string("[loader] jumping to kernel.\r\n");

    /* 10. Jump to the kernel entry point. RDI = boot_info, RSP = stack. */
    jump_to_kernel(ehdr.e_entry, boot_info, stack_top);

    /* Unreachable. */
    return EFI_LOAD_ERROR;
}

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static EFI_STATUS open_kernel_from_any_filesystem(EFI_HANDLE image_handle,
                                                  EFI_SYSTEM_TABLE *st,
                                                  void **out_buffer,
                                                  UINTN *out_size)
{
    EFI_STATUS status;
    EFI_LOADED_IMAGE_PROTOCOL *loaded = 0;
    void *iface = 0;
    EFI_HANDLE device = 0;

    /* First, try the device the bootloader was loaded from. */
    status = st->BootServices->HandleProtocol(image_handle,
                                              &EFI_LOADED_IMAGE_PROTOCOL_GUID,
                                              &iface);
    serial_write_string("[loader] LoadedImage HandleProtocol: ");
    print_status(status);
    serial_write_string("\r\n");
    if (status == EFI_SUCCESS) {
        loaded = (EFI_LOADED_IMAGE_PROTOCOL *)iface;
        device = loaded->DeviceHandle;

        /* Connect the controller so filesystem protocols get bound. */
        status = st->BootServices->ConnectController(device, 0, 0, 1);
        serial_write_string("[loader] ConnectController: ");
        print_status(status);
        serial_write_string("\r\n");

        status = st->BootServices->HandleProtocol(device,
                                                  &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
                                                  &iface);
        serial_write_string("[loader] DeviceHandle FS HandleProtocol: ");
        print_status(status);
        serial_write_string("\r\n");
        if (status == EFI_SUCCESS) {
            status = open_file_on_filesystem(st, iface, KERNEL_PATH,
                                             out_buffer, out_size);
            if (status == EFI_SUCCESS) {
                return EFI_SUCCESS;
            }
        }
    }

    /* Fallback: locate every device exposing the Simple File System protocol. */
    {
        UINTN num_handles = 0;
        EFI_HANDLE *handles = 0;
        UINTN i;

        num_handles = 0;
        handles = 0;
        status = st->BootServices->LocateHandleBuffer(2, /* ByProtocol */
                                                      &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
                                                      0, &num_handles, &handles);
        serial_write_string("[loader] LocateHandleBuffer FS: ");
        print_status(status);
        serial_write_string(" handles=");
        /* print num_handles as hex */
        {
            char hbuf[19];
            char hex[] = "0123456789ABCDEF";
            int j;
            hbuf[0] = '0'; hbuf[1] = 'x';
            for (j = 0; j < 16; j++) {
                hbuf[2 + j] = hex[(num_handles >> (60 - j * 4)) & 0xF];
            }
            hbuf[18] = '\0';
            serial_write_string(hbuf);
        }
        serial_write_string("\r\n");
        if (status != EFI_SUCCESS) {
            return status;
        }
        if (num_handles == 0 || handles == 0) {
            return EFI_NOT_FOUND;
        }

        for (i = 0; i < num_handles; i++) {
            void *fs_iface = 0;
            status = st->BootServices->HandleProtocol(handles[i],
                                                      &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
                                                      &fs_iface);
            if (status != EFI_SUCCESS) {
                continue;
            }
            status = open_file_on_filesystem(st, fs_iface, KERNEL_PATH,
                                             out_buffer, out_size);
            if (status == EFI_SUCCESS) {
                st->BootServices->FreePool(handles);
                return EFI_SUCCESS;
            }
        }

        st->BootServices->FreePool(handles);
    }

    return EFI_NOT_FOUND;
}

static EFI_STATUS open_file_on_filesystem(EFI_SYSTEM_TABLE *st,
                                          void *fs_iface,
                                          const CHAR16 *path,
                                          void **out_buffer,
                                          UINTN *out_size)
{
    EFI_STATUS status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = (EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *)fs_iface;
    EFI_FILE_PROTOCOL *root = 0;
    EFI_FILE_PROTOCOL *file = 0;
    UINTN info_size = 0;
    UINT8 file_info[512];
    UINT64 file_size = 0;
    UINTN read_size = 0;
    void *buffer = 0;

    status = fs->OpenVolume(fs, &root);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = root->Open(root, &file, (CHAR16 *)path,
                        EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        root->Close(root);
        return status;
    }

    /* Query required buffer size for EFI_FILE_INFO. */
    info_size = 0;
    status = file->GetInfo(file, &EFI_FILE_INFO_GUID, &info_size, 0);
    if (status != EFI_BUFFER_TOO_SMALL || info_size == 0) {
        file->Close(file);
        root->Close(root);
        return status == EFI_SUCCESS ? EFI_DEVICE_ERROR : status;
    }

    if (info_size > sizeof(file_info)) {
        file->Close(file);
        root->Close(root);
        return EFI_OUT_OF_RESOURCES;
    }

    status = file->GetInfo(file, &EFI_FILE_INFO_GUID, &info_size, file_info);
    if (status != EFI_SUCCESS) {
        file->Close(file);
        root->Close(root);
        return status;
    }

    /* EFI_FILE_INFO layout: Size(0x00), FileSize(0x08), PhysicalSize(0x10). */
    file_size = *(UINT64 *)((UINT8 *)file_info + 0x08);
    if (file_size == 0 || file_size > (UINT64)(UINTN)-1) {
        file->Close(file);
        root->Close(root);
        return EFI_INVALID_PARAMETER;
    }
    status = st->BootServices->AllocatePool(EfiLoaderData, (UINTN)file_size,
                                            &buffer);
    if (status != EFI_SUCCESS) {
        file->Close(file);
        root->Close(root);
        return status;
    }

    read_size = (UINTN)file_size;
    status = file->Read(file, &read_size, buffer);
    file->Close(file);
    root->Close(root);

    if (status != EFI_SUCCESS || read_size != (UINTN)file_size) {
        st->BootServices->FreePool(buffer);
        return EFI_DEVICE_ERROR;
    }

    *out_buffer = buffer;
    *out_size = (UINTN)file_size;
    return EFI_SUCCESS;
}

static EFI_STATUS get_framebuffer_info(EFI_SYSTEM_TABLE *st,
                                       struct fanstik_boot_info *boot_info)
{
    EFI_STATUS status;
    UINTN num_handles = 0;
    EFI_HANDLE *handles = 0;
    UINTN i;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;
    void *iface = 0;

    status = st->BootServices->LocateHandleBuffer(2, /* ByProtocol */
                                                  &EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
                                                  0, &num_handles, &handles);
    if (status != EFI_SUCCESS) {
        return status;
    }
    if (num_handles == 0 || handles == 0) {
        return EFI_NOT_FOUND;
    }

    for (i = 0; i < num_handles; i++) {
        status = st->BootServices->HandleProtocol(handles[i],
                                                  &EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
                                                  &iface);
        if (status != EFI_SUCCESS) {
            continue;
        }
        gop = (EFI_GRAPHICS_OUTPUT_PROTOCOL *)iface;
        break;
    }

    st->BootServices->FreePool(handles);

    if (gop == 0 || gop->Mode == 0 || gop->Mode->Info == 0) {
        return EFI_NOT_FOUND;
    }

    boot_info->framebuffer_base = gop->Mode->FrameBufferBase;
    boot_info->framebuffer_width = gop->Mode->Info->HorizontalResolution;
    boot_info->framebuffer_height = gop->Mode->Info->VerticalResolution;
    boot_info->pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;

    switch (gop->Mode->Info->PixelFormat) {
    case EFI_PIXEL_FORMAT_RGB:
        boot_info->pixel_format = FANSTIK_PIXEL_FORMAT_RGB;
        break;
    case EFI_PIXEL_FORMAT_BGR:
        boot_info->pixel_format = FANSTIK_PIXEL_FORMAT_BGR;
        break;
    case EFI_PIXEL_FORMAT_BITMASK:
        boot_info->pixel_format = FANSTIK_PIXEL_FORMAT_BITMASK;
        break;
    default:
        boot_info->pixel_format = FANSTIK_PIXEL_FORMAT_UNKNOWN;
        break;
    }

    return EFI_SUCCESS;
}

static EFI_STATUS get_acpi_rsdp(EFI_SYSTEM_TABLE *st, UINT64 *out_rsdp)
{
    UINTN i;
    EFI_CONFIGURATION_TABLE *table = (EFI_CONFIGURATION_TABLE *)st->ConfigurationTable;

    for (i = 0; i < st->NumberOfTableEntries; i++) {
        EFI_GUID *guid = &table[i].VendorGuid;
        int is_acpi20 = (guid->Data1 == EFI_ACPI_20_TABLE_GUID.Data1 &&
                         guid->Data2 == EFI_ACPI_20_TABLE_GUID.Data2 &&
                         guid->Data3 == EFI_ACPI_20_TABLE_GUID.Data3 &&
                         guid->Data4[0] == EFI_ACPI_20_TABLE_GUID.Data4[0] &&
                         guid->Data4[1] == EFI_ACPI_20_TABLE_GUID.Data4[1] &&
                         guid->Data4[2] == EFI_ACPI_20_TABLE_GUID.Data4[2] &&
                         guid->Data4[3] == EFI_ACPI_20_TABLE_GUID.Data4[3] &&
                         guid->Data4[4] == EFI_ACPI_20_TABLE_GUID.Data4[4] &&
                         guid->Data4[5] == EFI_ACPI_20_TABLE_GUID.Data4[5] &&
                         guid->Data4[6] == EFI_ACPI_20_TABLE_GUID.Data4[6] &&
                         guid->Data4[7] == EFI_ACPI_20_TABLE_GUID.Data4[7]);
        int is_acpi1 = (guid->Data1 == EFI_ACPI_TABLE_GUID.Data1 &&
                        guid->Data2 == EFI_ACPI_TABLE_GUID.Data2 &&
                        guid->Data3 == EFI_ACPI_TABLE_GUID.Data3 &&
                        guid->Data4[0] == EFI_ACPI_TABLE_GUID.Data4[0] &&
                        guid->Data4[1] == EFI_ACPI_TABLE_GUID.Data4[1] &&
                        guid->Data4[2] == EFI_ACPI_TABLE_GUID.Data4[2] &&
                        guid->Data4[3] == EFI_ACPI_TABLE_GUID.Data4[3] &&
                        guid->Data4[4] == EFI_ACPI_TABLE_GUID.Data4[4] &&
                        guid->Data4[5] == EFI_ACPI_TABLE_GUID.Data4[5] &&
                        guid->Data4[6] == EFI_ACPI_TABLE_GUID.Data4[6] &&
                        guid->Data4[7] == EFI_ACPI_TABLE_GUID.Data4[7]);

        if (is_acpi20 || is_acpi1) {
            *out_rsdp = (UINT64)(uintptr_t)table[i].VendorTable;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}

static EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *st,
                                 struct fanstik_boot_info *boot_info,
                                 UINTN *out_map_key)
{
    EFI_STATUS status;
    UINTN map_size = 0, map_key = 0, desc_size = 0;
    UINT32 desc_ver = 0;
    UINTN allocation_size;
    void *map_buf = 0;

    if (out_map_key == 0) {
        return EFI_INVALID_PARAMETER;
    }

    status = st->BootServices->GetMemoryMap(&map_size, 0, &map_key,
                                            &desc_size, &desc_ver);
    if (status != EFI_BUFFER_TOO_SMALL || desc_size == 0) {
        return status;
    }
    if (map_size > (UINTN)-1 - 2 * desc_size) {
        return EFI_OUT_OF_RESOURCES;
    }
    allocation_size = map_size + 2 * desc_size;

    status = st->BootServices->AllocatePool(EfiLoaderData, allocation_size,
                                            &map_buf);
    if (status != EFI_SUCCESS) {
        return status;
    }

    map_size = allocation_size;
    status = st->BootServices->GetMemoryMap(&map_size, map_buf, &map_key,
                                            &desc_size, &desc_ver);
    if (status != EFI_SUCCESS) {
        st->BootServices->FreePool(map_buf);
        return status;
    }

    boot_info->memory_map = (UINT64)(uintptr_t)map_buf;
    boot_info->memory_map_size = map_size;
    boot_info->memory_descriptor_size = desc_size;
    boot_info->memory_descriptor_version = desc_ver;
    boot_info->reserved = (UINT32)(map_key & 0xFFFFFFFFULL);
    *out_map_key = map_key;
    return EFI_SUCCESS;
}

static void jump_to_kernel(UINT64 entry, const struct fanstik_boot_info *boot_info,
                           void *stack_top)
{
    __asm__ volatile(
        "cli\n"
        "cld\n"
        "mov %0, %%rsp\n"
        "mov %1, %%rdi\n"
        "jmp *%2\n"
        : : "r"(stack_top), "r"(boot_info), "r"(entry)
        : "rsp", "rdi", "memory"
    );
}

static void print_status(UINTN status)
{
    char hex[] = "0123456789ABCDEF";
    char buf[19];
    int i;
    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 16; i++) {
        buf[2 + i] = hex[(status >> (60 - i * 4)) & 0xF];
    }
    buf[18] = '\0';
    serial_write_string(buf);
}
