#ifndef FANSTIK_UEFI_H
#define FANSTIK_UEFI_H

#include <stdint.h>
#include <stddef.h>

/* ========================================================================
 * UEFI base types (x86_64). Calling convention for UEFI services is
 * Microsoft x64 ABI on x86_64.
 * ======================================================================== */

typedef uintptr_t EFI_STATUS;
typedef uintptr_t UINTN;
typedef void *EFI_HANDLE;
typedef void *EFI_EVENT;
typedef uint16_t CHAR16;
typedef uint8_t BOOLEAN;
typedef void VOID;
typedef uint64_t UINT64;
typedef uint32_t UINT32;
typedef uint16_t UINT16;
typedef uint8_t UINT8;

#define EFIAPI __attribute__((ms_abi))

#define EFI_SUCCESS ((EFI_STATUS)0)
#define EFI_ERROR_BIT ((EFI_STATUS)1 << (sizeof(EFI_STATUS) * 8 - 1))
#define EFI_LOAD_ERROR (EFI_ERROR_BIT | (EFI_STATUS)1)
#define EFI_INVALID_PARAMETER (EFI_ERROR_BIT | (EFI_STATUS)2)
#define EFI_UNSUPPORTED (EFI_ERROR_BIT | (EFI_STATUS)3)
#define EFI_BUFFER_TOO_SMALL (EFI_ERROR_BIT | (EFI_STATUS)5)
#define EFI_NOT_FOUND (EFI_ERROR_BIT | (EFI_STATUS)14)
#define EFI_DEVICE_ERROR (EFI_ERROR_BIT | (EFI_STATUS)7)
#define EFI_WRITE_PROTECTED (EFI_ERROR_BIT | (EFI_STATUS)8)
#define EFI_OUT_OF_RESOURCES (EFI_ERROR_BIT | (EFI_STATUS)9)
#define EFI_ALREADY_STARTED (EFI_ERROR_BIT | (EFI_STATUS)20)

typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef struct {
    UINT32 Type;
    UINT32 Pad;
    UINT64 PhysicalStart;
    UINT64 VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

#define EFI_MEMORY_DESCRIPTOR_VERSION 1

#define EfiReservedMemoryType 0
#define EfiLoaderCode 1
#define EfiLoaderData 2
#define EfiBootServicesCode 3
#define EfiBootServicesData 4
#define EfiRuntimeServicesCode 5
#define EfiRuntimeServicesData 6
#define EfiConventionalMemory 7
#define EfiUnusableMemory 8
#define EfiACPIReclaimMemory 9
#define EfiACPIMemoryNVS 10
#define EfiMemoryMappedIO 11
#define EfiMemoryMappedIOPortSpace 12
#define EfiPalCode 13
#define EfiPersistentMemory 14

#define AllocateAnyPages 0
#define AllocateMaxAddress 1
#define AllocateAddress 2

#define EFI_PAGE_SIZE 0x1000ULL

/* ------------------------------------------------------------------ */
/* GUID                                                                 */
/* ------------------------------------------------------------------ */

typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

#define EFG_GUID(d1, d2, d3, b4, b5, b6, b7, b8, b9, b10, b11) \
    { (d1), (d2), (d3), { (b4), (b5), (b6), (b7), (b8), (b9), (b10), (b11) } }

/* ------------------------------------------------------------------ */
/* Text output protocol (already partially present; kept complete)      */
/* ------------------------------------------------------------------ */

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_OUTPUT_STRING)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, CHAR16 *String);

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET Reset;
    EFI_TEXT_OUTPUT_STRING OutputString;
    void *TestString;
    void *QueryMode;
    void *SetMode;
    void *SetAttribute;
    void *ClearScreen;
    void *SetCursorPosition;
    void *EnableCursor;
    void *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

struct EFI_BOOT_SERVICES;

/* ------------------------------------------------------------------ */
/* System table                                                       */
/* ------------------------------------------------------------------ */

typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16 *FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    void *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    void *RuntimeServices;
    struct EFI_BOOT_SERVICES *BootServices;
    UINTN NumberOfTableEntries;
    void *ConfigurationTable;
} EFI_SYSTEM_TABLE;

typedef struct {
    EFI_GUID VendorGuid;
    void *VendorTable;
} EFI_CONFIGURATION_TABLE;

/* ------------------------------------------------------------------ */
/* Boot services                                                      */
/* ------------------------------------------------------------------ */

typedef EFI_STATUS (EFIAPI *EFI_RAISE_TPL)(UINTN NewTpl);
typedef EFI_STATUS (EFIAPI *EFI_RESTORE_TPL)(UINTN OldTpl);
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_PAGES)(UINT32 Type, UINT32 MemoryType, UINTN Pages, UINT64 *Memory);
typedef EFI_STATUS (EFIAPI *EFI_FREE_PAGES)(UINT64 Memory, UINTN Pages);
typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP)(UINTN *MemoryMapSize, EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN *MapKey, UINTN *DescriptorSize, UINT32 *DescriptorVersion);
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(UINT32 PoolType, UINTN BufferSize, void **Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL)(void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_CREATE_EVENT)(UINT32 Type, UINTN NotifyTpl, void *NotifyFunction, void *NotifyContext, EFI_EVENT *Event);
typedef EFI_STATUS (EFIAPI *EFI_SET_TIMER)(EFI_EVENT Event, UINT32 TriggerType, UINT64 TriggerTime);
typedef EFI_STATUS (EFIAPI *EFI_WAIT_FOR_EVENT)(UINTN NumberOfEvents, EFI_EVENT *Event, UINTN *Index);
typedef EFI_STATUS (EFIAPI *EFI_SIGNAL_EVENT)(EFI_EVENT Event);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_EVENT)(EFI_EVENT Event);
typedef EFI_STATUS (EFIAPI *EFI_CHECK_EVENT)(EFI_EVENT Event, UINTN *Index);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE)(EFI_HANDLE *Handle, EFI_GUID *Protocol, void *Interface, void *InterfaceAttributes);
typedef EFI_STATUS (EFIAPI *EFI_REINSTALL_PROTOCOL_INTERFACE)(EFI_HANDLE Handle, EFI_GUID *Protocol, void *OldInterface, void *NewInterface);
typedef EFI_STATUS (EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE)(EFI_HANDLE Handle, EFI_GUID *Protocol, void *Interface);
typedef EFI_STATUS (EFIAPI *EFI_HANDLE_PROTOCOL)(EFI_HANDLE Handle, EFI_GUID *Protocol, void **Interface);
typedef EFI_STATUS (EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY)(EFI_GUID *Protocol, EFI_EVENT Event, void **Registration);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE)(UINT32 SearchType, EFI_GUID *Protocol, void *SearchKey, UINTN *BufferSize, EFI_HANDLE *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_DEVICE_PATH)(EFI_GUID *Protocol, void **DevicePath, EFI_HANDLE *Device);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_CONFIGURATION_TABLE)(EFI_GUID *Guid, void *Table);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_LOAD)(BOOLEAN BootPolicy, EFI_HANDLE ParentImageHandle, void *DevicePath, UINTN *BufferSize, void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_START)(EFI_HANDLE ImageHandle, UINTN *ExitDataSize, CHAR16 **ExitData);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_UNLOAD)(EFI_HANDLE ImageHandle);
typedef EFI_STATUS (EFIAPI *EFI_EXIT)(EFI_HANDLE ImageHandle, EFI_STATUS ExitStatus, UINTN ExitDataSize, CHAR16 *ExitData);
typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE ImageHandle, UINTN MapKey);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_MONOTONIC_COUNT)(UINT64 *Count);
typedef EFI_STATUS (EFIAPI *EFI_STALL)(UINTN Microseconds);
typedef EFI_STATUS (EFIAPI *EFI_SET_WATCHDOG_TIMER)(UINTN Timeout, UINT64 WatchdogCode, UINTN DataSize, CHAR16 *WatchdogData);
typedef EFI_STATUS (EFIAPI *EFI_CONNECT_CONTROLLER)(EFI_HANDLE ControllerHandle, EFI_HANDLE *DriverImageHandle, void *DevicePath, BOOLEAN Recursive);
typedef EFI_STATUS (EFIAPI *EFI_DISCONNECT_CONTROLLER)(EFI_HANDLE ControllerHandle, EFI_HANDLE DriverImageHandle, EFI_HANDLE ChildHandle);
typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL)(EFI_HANDLE Handle, EFI_GUID *Protocol, void **Interface, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle, UINT32 Attributes);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_PROTOCOL)(EFI_HANDLE Handle, EFI_GUID *Protocol, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle);
typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL_INFORMATION)(EFI_HANDLE Handle, EFI_GUID *Protocol, void **ProtocolBuffer, UINTN *BufferCount);
typedef EFI_STATUS (EFIAPI *EFI_PROTOCOLS_PER_HANDLE)(EFI_HANDLE Handle, EFI_GUID ***ProtocolBuffer, UINTN *ProtocolBufferCount);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE_BUFFER)(UINT32 SearchType, EFI_GUID *Protocol, void *SearchKey, UINTN *NoHandles, EFI_HANDLE **Buffer);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL)(EFI_GUID *Protocol, void *Registration, void **Interface);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES)(EFI_HANDLE *Handle, ...);
typedef EFI_STATUS (EFIAPI *EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES)(EFI_HANDLE Handle, ...);

typedef struct EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;
    EFI_RAISE_TPL RaiseTPL;
    EFI_RESTORE_TPL RestoreTPL;
    EFI_ALLOCATE_PAGES AllocatePages;
    EFI_FREE_PAGES FreePages;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL AllocatePool;
    EFI_FREE_POOL FreePool;
    EFI_CREATE_EVENT CreateEvent;
    EFI_SET_TIMER SetTimer;
    EFI_WAIT_FOR_EVENT WaitForEvent;
    EFI_SIGNAL_EVENT SignalEvent;
    EFI_CLOSE_EVENT CloseEvent;
    EFI_CHECK_EVENT CheckEvent;
    EFI_INSTALL_PROTOCOL_INTERFACE InstallProtocolInterface;
    EFI_REINSTALL_PROTOCOL_INTERFACE ReinstallProtocolInterface;
    EFI_UNINSTALL_PROTOCOL_INTERFACE UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL HandleProtocol;
    void *Reserved;
    EFI_REGISTER_PROTOCOL_NOTIFY RegisterProtocolNotify;
    EFI_LOCATE_HANDLE LocateHandle;
    EFI_LOCATE_DEVICE_PATH LocateDevicePath;
    EFI_INSTALL_CONFIGURATION_TABLE InstallConfigurationTable;
    EFI_IMAGE_LOAD LoadImage;
    EFI_IMAGE_START StartImage;
    EFI_IMAGE_UNLOAD UnloadImage;
    EFI_EXIT Exit;
    EFI_EXIT_BOOT_SERVICES ExitBootServices;
    EFI_GET_NEXT_MONOTONIC_COUNT GetNextMonotonicCount;
    EFI_STALL Stall;
    EFI_SET_WATCHDOG_TIMER SetWatchdogTimer;
    EFI_CONNECT_CONTROLLER ConnectController;
    EFI_DISCONNECT_CONTROLLER DisconnectController;
    EFI_OPEN_PROTOCOL OpenProtocol;
    EFI_CLOSE_PROTOCOL CloseProtocol;
    EFI_OPEN_PROTOCOL_INFORMATION OpenProtocolInformation;
    EFI_PROTOCOLS_PER_HANDLE ProtocolsPerHandle;
    EFI_LOCATE_HANDLE_BUFFER LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL LocateProtocol;
    EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES InstallMultipleProtocolInterfaces;
    EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES UninstallMultipleProtocolInterfaces;
} EFI_BOOT_SERVICES;

/* ------------------------------------------------------------------ */
/* Loaded Image protocol                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    UINT32 Revision;
    EFI_HANDLE ParentHandle;
    struct EFI_SYSTEM_TABLE *SystemTable;
    EFI_HANDLE DeviceHandle;
    void *FilePath;
    VOID *Reserved;
    UINT32 LoadOptionsSize;
    void *LoadOptions;
    VOID *ImageBase;
    UINT64 ImageSize;
    UINT8 ImageCodeType;
    UINT8 ImageDataType;
    VOID *Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

/* ------------------------------------------------------------------ */
/* Simple File System protocol                                          */
/* ------------------------------------------------------------------ */

struct EFI_FILE_PROTOCOL;
struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_VOLUME_OPEN)(struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This, struct EFI_FILE_PROTOCOL **Root);
typedef EFI_STATUS (EFIAPI *EFI_VOLUME_GET_INFO)(struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This, EFI_GUID *InformationType, UINTN *BufferSize, void *Buffer);

typedef struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64 Revision;
    EFI_VOLUME_OPEN OpenVolume;
    EFI_VOLUME_GET_INFO GetVolumeInfo;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN)(struct EFI_FILE_PROTOCOL *This, struct EFI_FILE_PROTOCOL **NewHandle, CHAR16 *FileName, UINT64 OpenMode, UINT64 Attributes);
typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE)(struct EFI_FILE_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_FILE_DELETE)(struct EFI_FILE_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_FILE_READ)(struct EFI_FILE_PROTOCOL *This, UINTN *BufferSize, void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE)(struct EFI_FILE_PROTOCOL *This, UINTN *BufferSize, const void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION)(struct EFI_FILE_PROTOCOL *This, UINT64 *Position);
typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION)(struct EFI_FILE_PROTOCOL *This, UINT64 Position);
typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_INFO)(struct EFI_FILE_PROTOCOL *This, EFI_GUID *InformationType, UINTN *BufferSize, void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_INFO)(struct EFI_FILE_PROTOCOL *This, EFI_GUID *InformationType, UINTN BufferSize, const void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH)(struct EFI_FILE_PROTOCOL *This);

typedef struct EFI_FILE_PROTOCOL {
    UINT64 Revision;
    EFI_FILE_OPEN Open;
    EFI_FILE_CLOSE Close;
    EFI_FILE_DELETE Delete;
    EFI_FILE_READ Read;
    EFI_FILE_WRITE Write;
    EFI_FILE_GET_POSITION GetPosition;
    EFI_FILE_SET_POSITION SetPosition;
    EFI_FILE_GET_INFO GetInfo;
    EFI_FILE_SET_INFO SetInfo;
    EFI_FILE_FLUSH Flush;
} EFI_FILE_PROTOCOL;

#define EFI_FILE_MODE_READ 0x0000000000000001ULL
#define EFI_FILE_MODE_WRITE 0x0000000000000002ULL
#define EFI_FILE_MODE_CREATE 0x8000000000000000ULL

#define EFI_FILE_READ_ONLY 0x0000000000000001ULL
#define EFI_FILE_HIDDEN 0x0000000000000002ULL
#define EFI_FILE_SYSTEM 0x0000000000000004ULL
#define EFI_FILE_RESERVED 0x0000000000000008ULL
#define EFI_FILE_DIRECTORY 0x0000000000000010ULL
#define EFI_FILE_ARCHIVE 0x0000000000000020ULL
#define EFI_FILE_VALID_ATTR 0x0000000000000037ULL

/* ------------------------------------------------------------------ */
/* Graphics Output Protocol                                             */
/* ------------------------------------------------------------------ */

struct EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
    UINT32 RedMask;
    UINT32 GreenMask;
    UINT32 BlueMask;
    UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

#define EFI_PIXEL_FORMAT_RGB 0
#define EFI_PIXEL_FORMAT_BGR 1
#define EFI_PIXEL_FORMAT_BITMASK 2

typedef struct {
    UINT32 Version;
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelFormat;
    EFI_PIXEL_BITMASK PixelInformation;
    UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32 MaxMode;
    UINT32 Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN SizeOfInfo;
    UINT64 FrameBufferBase;
    UINT64 FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, UINT32 ModeNumber, UINTN *SizeOfInfo, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, UINT32 ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT)(VOID);

typedef struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE SetMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

/* ------------------------------------------------------------------ */
/* Well-known GUIDs                                                     */
/* ------------------------------------------------------------------ */

static EFI_GUID EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID =
    { 0x964e5b22, 0x6459, 0x11d2, { 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } };
static EFI_GUID EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID =
    { 0x9042a9de, 0x23dc, 0x4a38, { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } };
static EFI_GUID EFI_LOADED_IMAGE_PROTOCOL_GUID =
    { 0x5B1B31A1, 0x9562, 0x11d2, { 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } };
static EFI_GUID EFI_ACPI_TABLE_GUID =
    { 0xEB9D2D30, 0x2D88, 0x11D3, { 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d } };
static EFI_GUID EFI_ACPI_20_TABLE_GUID =
    { 0x8868E871, 0xE4F1, 0x11D3, { 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81 } };

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

#endif /* FANSTIK_UEFI_H */
