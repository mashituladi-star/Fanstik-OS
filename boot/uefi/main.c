#include "uefi.h"
static void uefi_print(EFI_SYSTEM_TABLE *st, const CHAR16 *text) { if (st != 0 && st->ConOut != 0 && st->ConOut->OutputString != 0 && text != 0) st->ConOut->OutputString(st->ConOut, (CHAR16 *)(uintptr_t)text); }
EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *st) {
    static const CHAR16 banner[] = u"Fanstik-OS | x86_64 UEFI bring-up\r\n";
    static const CHAR16 ready[] = u"UEFI application entry: OK\r\n";
    static const CHAR16 next[] = u"Kernel ELF is separate; kernel handoff is not wired yet.\r\n";
    (void)image_handle;
    if (st == 0 || st->ConOut == 0 || st->ConOut->OutputString == 0) return EFI_INVALID_PARAMETER;
    uefi_print(st, banner); uefi_print(st, ready); uefi_print(st, next);
    return EFI_SUCCESS;
}
