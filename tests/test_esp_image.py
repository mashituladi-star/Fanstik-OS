#!/usr/bin/env python3
"""Проверяет важные свойства ESP без зависимости от mtools."""
import importlib.util
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "scripts" / "make_esp.py"

spec = importlib.util.spec_from_file_location("make_esp", MODULE_PATH)
if spec is None or spec.loader is None:
    raise RuntimeError("не удалось загрузить scripts/make_esp.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def entry(data: bytes, offset: int) -> tuple[str, int, int, bool]:
    raw_name = data[offset:offset + 11]
    name = raw_name[:8].decode("ascii").rstrip()
    extension = raw_name[8:11].decode("ascii").rstrip()
    full_name = f"{name}.{extension}" if extension else name
    attributes = data[offset + 11]
    high = struct.unpack_from("<H", data, offset + 20)[0]
    low = struct.unpack_from("<H", data, offset + 26)[0]
    size = struct.unpack_from("<I", data, offset + 28)[0]
    return full_name, (high << 16) | low, size, (attributes & 0x10) != 0


def cluster_offset(cluster: int) -> int:
    return (module.RESERVED_SECTORS + module.FAT_COUNT * module.FAT_SIZE_SECTORS +
            (cluster - 2) * module.SECTORS_PER_CLUSTER) * module.SECTOR_SIZE


def main() -> None:
    efi = b"EFI-TEST" * 200
    kernel = b"KERNEL-TEST" * 200
    image = module.build_image([("BOOTX64.EFI", efi), ("KERNEL.ELF", kernel)])
    assert len(image) == 64 * 1024 * 1024
    assert image[510:512] == b"\x55\xAA"
    assert image[82:90] == b"FAT32   "

    root = [entry(image, cluster_offset(module.ROOT_CLUSTER) + i * 32)
            for i in range(2)]
    assert root[0][:2] == ("EFI", module.EFI_CLUSTER)
    assert root[1][0] == "KERNEL.ELF"

    efi_dir = entry(image, cluster_offset(module.EFI_CLUSTER))
    boot_dir = entry(image, cluster_offset(module.BOOT_CLUSTER))
    assert efi_dir[:2] == ("BOOT", module.BOOT_CLUSTER)
    assert boot_dir[:3] == ("BOOTX64.EFI", module.FIRST_FILE_CLUSTER, len(efi))
    assert image[cluster_offset(module.FIRST_FILE_CLUSTER):
                 cluster_offset(module.FIRST_FILE_CLUSTER) + len(efi)] == efi

    kernel_cluster = root[1][1]
    assert image[cluster_offset(kernel_cluster):
                 cluster_offset(kernel_cluster) + len(kernel)] == kernel
    print("ESP image tests: all checks passed")


if __name__ == "__main__":
    main()
