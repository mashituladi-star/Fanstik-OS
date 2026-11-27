#ifndef FANSTIK_UEFI_LOADER_H
#define FANSTIK_UEFI_LOADER_H

#include "uefi.h"
#include <fanstik/boot_info.h>

/* Load the Fanstik kernel from the EFI System Partition, prepare the
 * boot-info structure and transfer control to the kernel entry point.
 *
 * Returns only on failure; on success it never returns (jumps to kernel).
 */
EFI_STATUS loader_load_and_boot(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *st);

#endif /* FANSTIK_UEFI_LOADER_H */
