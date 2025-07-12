//future funcionality

#include <efi/efi.h>
#include <efi.efilib.h>

#include "fs152.h"

static UINT8 buffer[512] = {0};

static FileHeader_t* parseHeaderFromBuff(UINT8 *buff){
	static FileHeader_t ret;
	ret.sig = *(UINT16*)buff;
	if (ret.sig != 0x4844) ret.sig = 0;
	ret.flags = *(UINT16*)buff+2;
	ret.parent_addr = *(UINT64*)buff+4;
	ret.len_bytes = *(UINT64*)buff+12;
	ret.data_start_addr = *(UINT64*)buff+20;
	for (UINT8 i = 0; i < 16; i++){ret.filename[i] = buff[96+i];}
	return &ret;
}

UINT8 OpenVolume(VolumeHandle_t *VH, EFI_LBA addr, EFI_BLOCK_IO_PROTOCOL *blkio_prot, UINT32 mediaid){
	VH->volume_lba = addr;
	VH->prot = blkio_prot;
	VH->mediaid = mediaid;
	EFI_LBA counter = 0;
	for(;;){
		uefi_call_wrapper(blkio_prot->ReadBlocks,5,blkio_prot,mediaid,addr+counter,512,(VOID*)buffer);
		VH->rootdir = *parseHeaderFromBuff(buffer);
		if (VH->rootdir.sig != 0){
			if (VH->rootdir.parent_addr) counter = (EFI_LBA)parent_addr-1; else break;
		}
		counter++
	}
	return 0;
}

UINT8 ReadFile(VolumeHandle_t *VH, FileHeader_t *file, CHAR8 *buff){
	EFI_LBA data_lba = VH->volume_lba+(EFI_LBA)file->data_start_addr;
	UINT64 lenght = file->len_bytes;
	UINT64 len512 = lenght-lenght%512;
	if (len512) uefi_call_wrapper(VH->prot->ReadBlocks,5,VH->prot,VH->mediaid,data_lba,len512,(VOID*)buff);
	if (lenght%512) uefi_call_wrapper(VH->prot->ReadBlocks,5,VH->prot,VH->mediaid,data_lba+len512/512,512,(VOID*)buffer);
	CopyMem((VOID*)&buff[len512], (VOID*)&buffer, lenght%512);
	return 0;
}

UINT8 SearchInFolder(VolumeHandle_t *VH, FileHeader_t *file, FileHeader_t *folder, CHAR8 filename[]){
	if(!(folder->flags & 2000)){
		return 1;
	}
	EFI_LBA data_lba = VH->volume_lba+(EFI_LBA)(folder->data_start_addr);
	UINT8 *buff = (UINT8*)AllocatePool(folder->len_bytes);
	ReadFile(VH, folder, buff);
	EFI_LBA addr = 0;
	UINTN res = 0;
	for (UINT64 c = 0; c < folder->len_bytes; c += 8){
		addr = *(EFI_LBA*)buff+c;
		uefi_call_wrapper(VH->prot->ReadBlocks,5,VH->prot,VH->mediaid,data_lba+addr,512,(VOID*)buffer);
		file = parseHeaderFromBuff(buffer);
		res = strcmpa(filename, file->filename);
		if (!res) break;
	}
	FreePool((VOID*)buff);
	return 0;
}