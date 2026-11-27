#include "uefi.h"
#include "loader.h"

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *st)
{
    static const CHAR16 banner[] = u"Fanstik-OS | x86_64 UEFI bootloader\r\n";
    static const CHAR16 ready[] = u"UEFI application entry: OK\r\n";

    if (st == 0 || st->ConOut == 0 || st->ConOut->OutputString == 0) {
        return EFI_INVALID_PARAMETER;
    }

    st->ConOut->OutputString(st->ConOut, (CHAR16 *)(uintptr_t)banner);
    st->ConOut->OutputString(st->ConOut, (CHAR16 *)(uintptr_t)ready);

    {
        EFI_STATUS status = loader_load_and_boot(image_handle, st);

        /* loader_load_and_boot does not return on success. */
        static const CHAR16 failed[] = u"Fanstik-OS | kernel handoff failed\r\n";
        st->ConOut->OutputString(st->ConOut, (CHAR16 *)(uintptr_t)failed);
        return status;
    }
}
