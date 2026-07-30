#ifndef __BL_DYNLINK_H__
#define __BL_DYNLINK_H__

#include "elf.h"

typedef struct{
	uint64_t addr;
	//to be expanded
} symbol_t;

typedef struct{
	void *image;
	size_t imagesize;
	Elf64_Dyn *dynamic;
	Elf64_Sym *dynsym;
	char *dynstr;
	size_t symcnt;
	Elf64_Rela *rela;
	size_t relacnt;
	size_t neededcnt;
	void* tls_area;
	size_t tls_size;
	char *soname;
} libid_t;

typedef struct __attribute__((__packed__)){
	char* name;
	void* data;
	size_t size;
} loaded_file;

#define weakptr(t) t*

typedef libid_t* libid_weakptr;
typedef symbol_t* symbol_weakptr;

extern void dl_init(EFI_FILE_HANDLE workdir);
extern libid_weakptr dl_process(char *path, const bool global_f);
extern void dl_fini(loaded_file **files, size_t *files_cnt);

#endif
