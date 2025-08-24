#ifndef __RTSVCS_H__
#define __RTSVCS_H__

#define ovmf_r11337

#define EFI_ERROR(a)(((int64_t)a)<0)
#define EFIERR(a)(0x8000000000000000|a)

#define EFI_SUCCESS 0
#define EFI_LOAD_ERROR EFIERR(1)
#define EFI_INVALID_PARAMETER EFIERR(2)
#define EFI_UNSUPPORTED EFIERR(3)
#define EFI_BAD_BUFFER_SIZE EFIERR(4)
#define EFI_BUFFER_TOO_SMALL EFIERR(5)
#define EFI_NOT_READY EFIERR(6)
#define EFI_DEVICE_ERROR EFIERR(7)
#define EFI_WRITE_PROTECTED EFIERR(8)
#define EFI_OUT_OF_RESOURCES EFIERR(9)
#define EFI_VOLUME_CORRUPTED EFIERR(10)
#define EFI_VOLUME_FULL EFIERR(11)
#define EFI_NO_MEDIA EFIERR(12)
#define EFI_MEDIA_CHANGED EFIERR(13)
#define EFI_NOT_FOUND EFIERR(14)
#define EFI_ACCESS_DENIED EFIERR(15)
#define EFI_NO_RESPONSE EFIERR(16)
#define EFI_NO_MAPPING EFIERR(17)
#define EFI_TIMEOUT EFIERR(18)
#define EFI_NOT_STARTED EFIERR(19)
#define EFI_ALREADY_STARTED EFIERR(20)
#define EFI_ABORTED EFIERR(21)
#define EFI_ICMP_ERROR EFIERR(22)
#define EFI_TFTP_ERROR EFIERR(23)
#define EFI_PROTOCOL_ERROR EFIERR(24)
#define EFI_INCOMPATIBLE_VERSION EFIERR(25)
#define EFI_SECURITY_VIOLATION EFIERR(26)
#define EFI_CRC_ERROR EFIERR(27)
#define EFI_END_OF_MEDIA EFIERR(28)
#define EFI_END_OF_FILE EFIERR(31)
#define EFI_INVALID_LANGUAGE EFIERR(32)
#define EFI_COMPROMISED_DATA EFIERR(33)

#define EFI_WARN_UNKNOWN_GLYPH 1
#define EFI_WARN_DELETE_FAILURE 2
#define EFI_WARN_WRITE_FAILURE 3
#define EFI_WARN_BUFFER_TOO_SMALL 4

typedef struct {          
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t Data4[8]; 
} EFI_GUID;

typedef struct {          
	uint16_t Year;       // 1998 - 20XX
	uint8_t Month;      // 1 - 12
	uint8_t Day;        // 1 - 31
	uint8_t Hour;       // 0 - 23
	uint8_t Minute;     // 0 - 59
	uint8_t Second;     // 0 - 59
	uint8_t Pad1;
	uint32_t Nanosecond; // 0 - 999,999,999
	int16_t TimeZone;   // -1440 to 1440 or 2047
	uint8_t Daylight;
	uint8_t Pad2;
} EFI_TIME;

#define EFI_TIME_ADJUST_DAYLIGHT    0x01
#define EFI_TIME_IN_DAYLIGHT        0x02
#define EFI_UNSPECIFIED_TIMEZONE    0x07FF

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

#ifdef ovmf_r11337
#define KERNEL_MEMORY_TYPE EfiRuntimeServicesData
#define PTZONE_MEMORY_TYPE KERNEL_MEMORY_TYPE
#else
#define KERNEL_MEMORY_TYPE 0x80000000
#define PTZONE_MEMORY_TYPE 0x80000001
#endif

typedef struct {
	uint32_t Type; // Field size is 32 bits followed by 32 bit pad
	uint32_t Pad;
	uint64_t PhysicalStart; // Field size is 64 bits
	uint64_t VirtualStart; // Field size is 64 bits
	uint64_t NumberOfPages; // Field size is 64 bits
	uint64_t Attribute; // Field size is 64 bits
} EFI_MEMORY_DESCRIPTOR;

typedef int64_t (__attribute__((ms_abi)) *EFI_SET_VIRTUAL_ADDRESS_MAP) (
	uint64_t MemoryMapSize,
	uint64_t DescriptorSize,
	uint32_t DescriptorVersion,
	EFI_MEMORY_DESCRIPTOR *VirtualMap
);

#define EFI_OPTIONAL_PTR 0x00000001
#define EFI_INTERNAL_FNC 0x00000002 // Pointer to internal runtime fnc
#define EFI_INTERNAL_PTR 0x00000004 // Pointer to internal runtime data

typedef int64_t (__attribute__((ms_abi)) *EFI_CONVERT_POINTER) (
	uint64_t DebugDisposition,
	void **Address
);

#define EFI_GLOBAL_VARIABLE { 0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C} }

#define EFI_VARIABLE_NON_VOLATILE 0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS 0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD 0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020
#define EFI_VARIABLE_APPEND_WRITE 0x00000040
#define EFI_MAXIMUM_VARIABLE_SIZE 1024

typedef int64_t (__attribute__((ms_abi)) *EFI_GET_VARIABLE) (
	uint16_t *VariableName,
	EFI_GUID *VendorGuid,
	uint32_t *Attributes,
	uint64_t *DataSize,
	void *Data
);

typedef int64_t (__attribute__((ms_abi)) *EFI_GET_NEXT_VARIABLE_NAME) (
	uint64_t *VariableNameSize,
	uint16_t *VariableName,
	EFI_GUID *VendorGuid
);


typedef int64_t (__attribute__((ms_abi)) *EFI_SET_VARIABLE) (
	uint16_t *VariableName,
	EFI_GUID *VendorGuid,
	uint32_t Attributes,
	uint64_t DataSize,
	void *Data
);

typedef struct {
	uint32_t Resolution; // 1e-6 parts per million
	uint32_t Accuracy; // hertz
	uint8_t SetsToZero; // Set clears sub-second time
} EFI_TIME_CAPABILITIES;


typedef int64_t (__attribute__((ms_abi)) *EFI_GET_TIME) (
	EFI_TIME *Time,
	EFI_TIME_CAPABILITIES *Capabilities
);

typedef int64_t (__attribute__((ms_abi)) *EFI_SET_TIME) (
	EFI_TIME *Time
);

typedef int64_t (__attribute__((ms_abi)) *EFI_GET_WAKEUP_TIME) (
	uint8_t *Enabled,
	uint8_t *Pending,
	EFI_TIME *Time
);

typedef int64_t (__attribute__((ms_abi)) *EFI_SET_WAKEUP_TIME) (
	uint8_t Enable,
	EFI_TIME *Time
);

typedef enum {
	EfiResetCold,
	EfiResetWarm,
	EfiResetShutdown
} EFI_RESET_TYPE;

typedef int64_t (__attribute__((ms_abi)) *EFI_RESET_SYSTEM) (
	EFI_RESET_TYPE ResetType,
	int64_t ResetStatus,
	uint64_t DataSize,
	uint16_t *ResetData
);

typedef int64_t (__attribute__((ms_abi)) *EFI_GET_NEXT_MONOTONIC_COUNT) (
	uint64_t *Count
);

typedef int64_t (__attribute__((ms_abi)) *EFI_GET_NEXT_HIGH_MONO_COUNT) (
	uint32_t *HighCount
);

typedef struct {
	uint64_t Length;
	union {
		uint64_t DataBlock;
		uint64_t ContinuationPointer;
	} Union;
} EFI_CAPSULE_BLOCK_DESCRIPTOR;

typedef struct {
	EFI_GUID CapsuleGuid;
	uint32_t HeaderSize;
	uint32_t Flags;
	uint32_t CapsuleImageSize;
} EFI_CAPSULE_HEADER;

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET 0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE 0x00020000
#define CAPSULE_FLAGS_INITIATE_RESET 0x00040000

typedef int64_t (__attribute__((ms_abi)) *EFI_UPDATE_CAPSULE) (
	EFI_CAPSULE_HEADER **CapsuleHeaderArray,
	uint64_t CapsuleCount,
	uint64_t ScatterGatherList
);

typedef int64_t (__attribute__((ms_abi)) *EFI_QUERY_CAPSULE_CAPABILITIES) (
	EFI_CAPSULE_HEADER **CapsuleHeaderArray,
	uint64_t CapsuleCount,
	uint64_t *MaximumCapsuleSize,
	EFI_RESET_TYPE *ResetType
);

typedef int64_t (__attribute__((ms_abi)) *EFI_QUERY_VARIABLE_INFO) (
	uint32_t Attributes,
	uint64_t *MaximumVariableStorageSize,
	uint64_t *RemainingVariableStorageSize,
	uint64_t *MaximumVariableSize
);

typedef struct{
	uint64_t Signature;
	uint32_t Revision;
	uint32_t HeaderSize;
	uint32_t CRC32;
	uint32_t Reserved;
} EFI_TABLE_HEADER;

typedef struct{
	EFI_TABLE_HEADER Hdr;
	EFI_GET_TIME GetTime;
	EFI_SET_TIME SetTime;
	EFI_GET_WAKEUP_TIME GetWakeupTime;
	EFI_SET_WAKEUP_TIME SetWakeupTime;
	EFI_SET_VIRTUAL_ADDRESS_MAP SetVirtualAddressMap;
	EFI_CONVERT_POINTER ConvertPointer;
	EFI_GET_VARIABLE GetVariable;
	EFI_GET_NEXT_VARIABLE_NAME GetNextVariableName;
	EFI_SET_VARIABLE SetVariable;
	EFI_GET_NEXT_HIGH_MONO_COUNT GetNextHighMonotonicCount;
	EFI_RESET_SYSTEM ResetSystem;
	EFI_UPDATE_CAPSULE UpdateCapsule;
	EFI_QUERY_CAPSULE_CAPABILITIES QueryCapsuleCapabilities;
	EFI_QUERY_VARIABLE_INFO QueryVariableInfo;
} rtsvcs_t;

#endif