#include "elf64.h"

static int elf64_ident_valid(const unsigned char ident[16])
{
    return ident[0] == ELFMAG0 &&
           ident[1] == ELFMAG1 &&
           ident[2] == ELFMAG2 &&
           ident[3] == ELFMAG3 &&
           ident[4] == ELFCLASS64 &&
           ident[5] == ELFDATA2LSB &&
           ident[6] == EV_CURRENT;
}

EFI_STATUS elf64_validate(const void *image, UINTN image_size, Elf64_Ehdr *out)
{
    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)image;
    const Elf64_Phdr *phdrs;
    UINTN i;
    int has_load_segment = 0;
    int entry_is_executable = 0;

    if (image == 0 || out == 0) {
        return EFI_INVALID_PARAMETER;
    }
    if (image_size < sizeof(Elf64_Ehdr)) {
        return EFI_INVALID_PARAMETER;
    }
    if (!elf64_ident_valid(ehdr->e_ident)) {
        return EFI_INVALID_PARAMETER;
    }
    /* The first loader only supports a fixed-address kernel image. */
    if (ehdr->e_type != ET_EXEC) {
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_machine != EM_X86_64 || ehdr->e_version != EV_CURRENT) {
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_ehsize != sizeof(Elf64_Ehdr) || ehdr->e_phoff == 0 ||
        ehdr->e_phnum == 0 || ehdr->e_phentsize != sizeof(Elf64_Phdr)) {
        return EFI_INVALID_PARAMETER;
    }
    /* Check the table without allowing multiplication or addition overflow. */
    if (ehdr->e_phoff > image_size ||
        ehdr->e_phnum > (image_size - (UINTN)ehdr->e_phoff) / sizeof(Elf64_Phdr)) {
        return EFI_INVALID_PARAMETER;
    }

    phdrs = (const Elf64_Phdr *)((const UINT8 *)image + ehdr->e_phoff);
    for (i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr *phdr = &phdrs[i];
        UINT64 segment_end;

        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        if (phdr->p_memsz < phdr->p_filesz ||
            phdr->p_offset > image_size ||
            phdr->p_filesz > image_size - phdr->p_offset ||
            phdr->p_paddr > UINT64_MAX - phdr->p_memsz ||
            phdr->p_vaddr > UINT64_MAX - phdr->p_memsz) {
            return EFI_INVALID_PARAMETER;
        }
        if (phdr->p_align > 1 &&
            ((phdr->p_vaddr - phdr->p_offset) & (phdr->p_align - 1)) != 0) {
            return EFI_INVALID_PARAMETER;
        }
        if (phdr->p_memsz == 0) {
            continue;
        }
        if (phdr->p_align != 0 && (phdr->p_align & (phdr->p_align - 1)) != 0) {
            return EFI_INVALID_PARAMETER;
        }

        segment_end = phdr->p_paddr + phdr->p_memsz;
        for (UINTN previous = 0; previous < i; previous++) {
            const Elf64_Phdr *other = &phdrs[previous];
            UINT64 other_end;
            if (other->p_type != PT_LOAD || other->p_memsz == 0) {
                continue;
            }
            other_end = other->p_paddr + other->p_memsz;
            if (phdr->p_paddr < other_end && other->p_paddr < segment_end) {
                return EFI_INVALID_PARAMETER;
            }
        }
        has_load_segment = 1;
        if (ehdr->e_entry >= phdr->p_vaddr &&
            ehdr->e_entry - phdr->p_vaddr < phdr->p_memsz &&
            (phdr->p_flags & PF_X) != 0) {
            entry_is_executable = 1;
        }
    }

    if (!has_load_segment || !entry_is_executable) {
        return EFI_INVALID_PARAMETER;
    }

    __builtin_memcpy(out, ehdr, sizeof(Elf64_Ehdr));
    return EFI_SUCCESS;
}

const Elf64_Phdr *elf64_phdrs(const Elf64_Ehdr *ehdr, const void *image)
{
    if (ehdr == 0 || image == 0) {
        return 0;
    }
    return (const Elf64_Phdr *)((const UINT8 *)image + ehdr->e_phoff);
}
