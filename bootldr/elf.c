#include <efi/efi.h>
#include <efi/efilib.h>

#include "elf.h"

uint8_t check_elf(void* file){
	Elf64_Ehdr *hdr = (Elf64_Ehdr*)file;
	if (*(uint32_t*)(&hdr->e_ident) != EI_MAG) return 1; //ei_mag
	if (hdr->e_ident[4] != 2) return 2; //ei_class
	if (hdr->e_ident[5] != 1) return 3; //ei_data
	if (hdr->e_ident[6] != 1) return 4; //ei_ver
	if (hdr->e_ident[7] != 0) return 5; //ei_abi
	if (hdr->e_ident[8] != 0) return 6; //ei_abiver
	if (hdr->e_type > 3) return 7;
	if (hdr->e_machine != 0x3e) return 8;
	if (hdr->e_version != 1) return 9;
	if (hdr->e_ehsize != 64) return 10;
	if (hdr->e_phentsize != 56) return 11;
	if (hdr->e_phnum == 0) return 12;
	if (hdr->e_phoff == 0) return 13;
	return 0;
}

Elf64_Shdr *get_section_by_name(Elf64_Ehdr* image, const char* name){
	Elf64_Shdr *shdrs = (void*)image + image->e_shoff;

	const char *shstrtab = (void*)image + shdrs[image->e_shstrndx].sh_offset;

	for (size_t i = 0; i < image->e_shnum; i++){
		if (!strcmpa(&shstrtab[shdrs[i].sh_name], name)){
			return &shdrs[i];
		};
	}

	return NULL;
}

Elf64_Dyn* get_dyn(Elf64_Ehdr* image){
	Elf64_Phdr *phdrs = (void*)image + image->e_phoff;

	for (size_t i = 0; i < image->e_phnum; i++){
		if (phdrs[i].p_type == PT_DYNAMIC) return (void*)image + phdrs[i].p_offset;
	}

	return NULL;
}

Elf64_Dyn* get_dynent(Elf64_Dyn* start, Elf64_Sxword tag){
	for (; start->d_tag != DT_NULL; start++){
		if (start->d_tag == tag) return start;
	}

	return NULL;
}

void* off2ptr(void* base, Elf64_Dyn* ent){
	return ent ? base + ent->d_un.d_val : NULL;
}