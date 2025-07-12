#ifndef __BL_ELF_H__
#define __BL_ELF_H__

#define EI_MAG 0x464c457f //little-endian

typedef struct __attribute__((__packed__)){
	//UINT8 e_ident[16]
	UINT32 ei_mag;
	UINT8 ei_class;
	UINT8 ei_data;
	UINT8 ei_ver;
	UINT8 ei_abi;
	UINT8 ei_abiver;
	UINT8 ei_pad[7];
	//end e_ident
	UINT16 e_type;
	UINT16 e_machine;
	UINT32 e_ver;
	UINT64 e_entry;
	UINT64 e_phoff;
	UINT64 e_shoff;
	UINT32 e_flags;
	UINT16 e_ehsize;
	UINT16 e_phentsize;
	UINT16 e_phnum;
	UINT16 e_shentsize;
	UINT16 e_shnum;
	UINT16 e_shstrndx;
} elf_hdr_t;

typedef struct __attribute__((__packed__)){
	UINT32 p_type;
	UINT32 p_flags;
	UINT64 p_offset;
	UINT64 p_vaddr;
	UINT64 p_paddr;
	UINT64 p_filesz;
	UINT64 p_memsz;
	UINT64 p_align;
} elf_prog_hdr_t;

static inline UINT8 check_elf(void* file){
	elf_hdr_t *hdr = (elf_hdr_t*)file;
	if (hdr->ei_mag != EI_MAG) return 1;
	if (hdr->ei_class != 2) return 2;
	if (hdr->ei_data != 1) return 3;
	if (hdr->ei_ver != 1) return 4;
	if (hdr->ei_abi != 0) return 5;
	if (hdr->e_type-2 > 1) return 6;
	if (hdr->e_machine != 0x3e) return 7;
	if (hdr->e_ver != 1) return 8;
	if (hdr->e_ehsize != 64) return 9;
	if (hdr->e_phentsize != 56) return 10;
	if (hdr->e_phnum == 0) return 11;
	if (hdr->e_phoff == 0) return 12;
	return 0;
}

#endif