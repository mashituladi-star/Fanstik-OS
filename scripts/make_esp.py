#!/usr/bin/env python3
"""Создаёт минимальный FAT32-образ ESP для запуска Fanstik-OS в QEMU."""
import os
import struct
import sys
from typing import Iterable, List, Tuple

SECTOR_SIZE = 512
DISK_SIZE_MB = 64
TOTAL_SECTORS = (DISK_SIZE_MB * 1024 * 1024) // SECTOR_SIZE
RESERVED_SECTORS = 32
FAT_COUNT = 2
SECTORS_PER_CLUSTER = 1
FAT_SIZE_SECTORS = 1024
ROOT_CLUSTER = 2
EFI_CLUSTER = 3
BOOT_CLUSTER = 4
FIRST_FILE_CLUSTER = 5
FAT_EOC = 0x0FFFFFFF


def short_name(name: str) -> bytes:
    """Преобразует имя 8.3 в 11-байтовое поле FAT directory entry."""
    base, dot, extension = name.upper().partition(".")
    if not base or len(base) > 8 or len(extension) > 3:
        raise ValueError(f"неподдерживаемое имя FAT 8.3: {name}")
    return base.ljust(8).encode("ascii") + extension.ljust(3).encode("ascii")


def set_fat_entry(fat: bytearray, cluster: int, value: int) -> None:
    struct.pack_into("<I", fat, cluster * 4, value & FAT_EOC)


def append_cluster_chain(fat: bytearray, next_cluster: int, data: bytes,
                         image: bytearray, data_start_sector: int) -> Tuple[int, int]:
    cluster_size = SECTOR_SIZE * SECTORS_PER_CLUSTER
    count = max(1, (len(data) + cluster_size - 1) // cluster_size)
    first = next_cluster
    for index in range(count):
        cluster = next_cluster + index
        following = cluster + 1 if index + 1 < count else FAT_EOC
        set_fat_entry(fat, cluster, following)
        chunk = data[index * cluster_size:(index + 1) * cluster_size]
        offset = (data_start_sector + (cluster - 2) * SECTORS_PER_CLUSTER) * SECTOR_SIZE
        image[offset:offset + len(chunk)] = chunk
    return first, next_cluster + count


def directory_entry(name: str, cluster: int, size: int, directory: bool) -> bytes:
    entry = bytearray(32)
    entry[0:11] = short_name(name)
    entry[11] = 0x10 if directory else 0x20
    struct.pack_into("<H", entry, 20, (cluster >> 16) & 0xFFFF)
    struct.pack_into("<H", entry, 26, cluster & 0xFFFF)
    struct.pack_into("<I", entry, 28, size)
    return bytes(entry)


def write_directory(image: bytearray, data_start_sector: int, cluster: int,
                    entries: Iterable[bytes]) -> None:
    payload = b"".join(entries) + b"\x00"
    if len(payload) > SECTOR_SIZE * SECTORS_PER_CLUSTER:
        raise ValueError("каталог ESP не помещается в один кластер")
    offset = (data_start_sector + (cluster - 2) * SECTORS_PER_CLUSTER) * SECTOR_SIZE
    image[offset:offset + len(payload)] = payload


def build_image(files: List[Tuple[str, bytes]]) -> bytes:
    """Создаёт образ с файлами BOOTX64.EFI и KERNEL.ELF."""
    data_start_sector = RESERVED_SECTORS + FAT_COUNT * FAT_SIZE_SECTORS
    image = bytearray(TOTAL_SECTORS * SECTOR_SIZE)
    fat = bytearray(FAT_SIZE_SECTORS * SECTOR_SIZE)

    # FAT32 boot sector. The cluster count is above the FAT32 minimum.
    boot = bytearray(SECTOR_SIZE)
    boot[0:3] = b"\xEB\x58\x90"
    boot[3:11] = b"FANSTIK "
    struct.pack_into("<H", boot, 11, SECTOR_SIZE)
    boot[13] = SECTORS_PER_CLUSTER
    struct.pack_into("<H", boot, 14, RESERVED_SECTORS)
    boot[16] = FAT_COUNT
    struct.pack_into("<H", boot, 17, 0)
    struct.pack_into("<H", boot, 19, 0)
    boot[21] = 0xF8
    struct.pack_into("<H", boot, 22, 0)
    struct.pack_into("<H", boot, 24, 63)
    struct.pack_into("<H", boot, 26, 255)
    struct.pack_into("<I", boot, 28, 0)
    struct.pack_into("<I", boot, 32, TOTAL_SECTORS)
    struct.pack_into("<I", boot, 36, FAT_SIZE_SECTORS)
    struct.pack_into("<H", boot, 40, 0)
    struct.pack_into("<H", boot, 42, 0)
    struct.pack_into("<I", boot, 44, ROOT_CLUSTER)
    struct.pack_into("<H", boot, 48, 1)
    struct.pack_into("<H", boot, 50, 6)
    boot[64] = 0x80
    boot[66] = 0x29
    struct.pack_into("<I", boot, 67, 0x46414E31)
    boot[71:82] = b"FANSTIK ESP"
    boot[82:90] = b"FAT32   "
    boot[510:512] = b"\x55\xAA"
    image[0:SECTOR_SIZE] = boot
    image[6 * SECTOR_SIZE:7 * SECTOR_SIZE] = boot

    # FSInfo sector and its backup copy.
    fsinfo = bytearray(SECTOR_SIZE)
    struct.pack_into("<I", fsinfo, 0, 0x41615252)
    struct.pack_into("<I", fsinfo, 484, 0x61417272)
    struct.pack_into("<I", fsinfo, 488, 0xFFFFFFFF)
    struct.pack_into("<I", fsinfo, 492, 0xFFFFFFFF)
    struct.pack_into("<H", fsinfo, 510, 0xAA55)
    image[SECTOR_SIZE:2 * SECTOR_SIZE] = fsinfo
    image[7 * SECTOR_SIZE:8 * SECTOR_SIZE] = fsinfo

    set_fat_entry(fat, 0, 0x0FFFFFF8)
    set_fat_entry(fat, 1, FAT_EOC)
    set_fat_entry(fat, ROOT_CLUSTER, FAT_EOC)
    set_fat_entry(fat, EFI_CLUSTER, FAT_EOC)
    set_fat_entry(fat, BOOT_CLUSTER, FAT_EOC)

    next_cluster = FIRST_FILE_CLUSTER
    file_clusters = {}
    for name, data in files:
        cluster, next_cluster = append_cluster_chain(
            fat, next_cluster, data, image, data_start_sector
        )
        file_clusters[name.upper()] = (cluster, len(data))

    fat_offset = RESERVED_SECTORS * SECTOR_SIZE
    image[fat_offset:fat_offset + len(fat)] = fat
    second_fat_offset = (RESERVED_SECTORS + FAT_SIZE_SECTORS) * SECTOR_SIZE
    image[second_fat_offset:second_fat_offset + len(fat)] = fat

    boot_file = file_clusters.get("BOOTX64.EFI")
    kernel_file = file_clusters.get("KERNEL.ELF")
    if boot_file is None or kernel_file is None:
        raise ValueError("ESP требует BOOTX64.EFI и KERNEL.ELF")

    write_directory(image, data_start_sector, ROOT_CLUSTER, [
        directory_entry("EFI", EFI_CLUSTER, 0, True),
        directory_entry("KERNEL.ELF", kernel_file[0], kernel_file[1], False),
    ])
    write_directory(image, data_start_sector, EFI_CLUSTER, [
        directory_entry("BOOT", BOOT_CLUSTER, 0, True),
    ])
    write_directory(image, data_start_sector, BOOT_CLUSTER, [
        directory_entry("BOOTX64.EFI", boot_file[0], boot_file[1], False),
    ])
    return bytes(image)


def main() -> None:
    out = sys.argv[1] if len(sys.argv) > 1 else "build/esp.img"
    efi_path = sys.argv[2] if len(sys.argv) > 2 else "build/BOOTX64.EFI"
    kernel_path = sys.argv[3] if len(sys.argv) > 3 else "build/kernel.elf"
    files = []
    for path, name in ((efi_path, "BOOTX64.EFI"), (kernel_path, "KERNEL.ELF")):
        if not os.path.exists(path):
            raise SystemExit(f"error: файл не найден: {path}")
        with open(path, "rb") as stream:
            files.append((name, stream.read()))
    image = build_image(files)
    os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
    with open(out, "wb") as stream:
        stream.write(image)
    print(f"wrote {out} ({len(image)} bytes)")


if __name__ == "__main__":
    main()
