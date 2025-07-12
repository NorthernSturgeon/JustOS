//future funcionality

#ifndef __BL_SDFS_H__
#define __BL_SDFS_H__

typedef struct{
	UINT16 sig;
	UINT16 flags;
	UINT64 parent_addr;
	UINT64 len_bytes;
	UINT64 data_start_addr;
	CHAR8 filename[16];
} FileHeader_t;

typedef struct{
	EFI_LBA volume_lba;
	FileHeader_t rootdir;
	EFI_BLOCK_IO_PROTOCOL *prot;
	UINT32 mediaid;
} VolumeHandle_t;

extern UINTN OpenVolume(VolumeHandle_t *VH, EFI_LBA addr, EFI_BLOCK_IO_PROTOCOL *blkio_prot, UINT32 mediaid);
extern UINTN ReadFile(VolumeHandle_t *VH, FileHeader_t *file, CHAR8 *buff);
extern UINTN SearchInFolder(VolumeHandle_t *VH, FileHeader_t *file, FileHeader_t *folder, CHAR8 *filename);

#endif