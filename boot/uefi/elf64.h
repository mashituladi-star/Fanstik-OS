#ifndef FANSTIK_UEFI_ELF64_H
#define FANSTIK_UEFI_ELF64_H

#include "uefi.h"

/* ELF64 types */
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASS64 2
#define ELFDATA2LSB 1
#define EV_CURRENT 1

#define ET_EXEC 2
#define ET_DYN 3

#define EM_X86_64 62

#define PT_NULL 0
#define PT_LOAD 1

#define EF_X86_64_PIC 0x00000001

typedef struct {
    unsigned char e_ident[16];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

/* PF flags */
#define PF_X 1
#define PF_W 2
#define PF_R 4

/* Validate an ELF64 image located at `image` of size `image_size`.
 * Fills `out` with parsed header information on success.
 * Returns EFI_SUCCESS on success, an UEFI error status otherwise. */
EFI_STATUS elf64_validate(const void *image, UINTN image_size, Elf64_Ehdr *out);

/* Return pointer to the program header table for a validated ELF image. */
const Elf64_Phdr *elf64_phdrs(const Elf64_Ehdr *ehdr, const void *image);

#endif /* FANSTIK_UEFI_ELF64_H */
