#ifndef FANSTIK_UEFI_H
#define FANSTIK_UEFI_H
#include <stdint.h>
#include <uchar.h>
typedef uintptr_t EFI_STATUS;
typedef uintptr_t UINTN;
typedef void *EFI_HANDLE;
typedef char16_t CHAR16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
#define EFI_SUCCESS ((EFI_STATUS)0)
#define EFI_INVALID_PARAMETER ((EFI_STATUS)2)
typedef struct { UINT64 Signature; UINT32 Revision; UINT32 HeaderSize; UINT32 CRC32; UINT32 Reserved; } EFI_TABLE_HEADER;
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (*EFI_TEXT_RESET)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, unsigned char ExtendedVerification);
typedef EFI_STATUS (*EFI_TEXT_OUTPUT_STRING)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, CHAR16 *String);
typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL { EFI_TEXT_RESET Reset; EFI_TEXT_OUTPUT_STRING OutputString; void *TestString; void *QueryMode; void *SetMode; void *SetAttribute; void *ClearScreen; void *SetCursorPosition; void *EnableCursor; void *Mode; } EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef struct { EFI_TABLE_HEADER Hdr; CHAR16 *FirmwareVendor; UINT32 FirmwareRevision; EFI_HANDLE ConsoleInHandle; void *ConIn; EFI_HANDLE ConsoleOutHandle; EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut; EFI_HANDLE StandardErrorHandle; EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr; void *RuntimeServices; void *BootServices; UINTN NumberOfTableEntries; void *ConfigurationTable; } EFI_SYSTEM_TABLE;
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);
#endif
