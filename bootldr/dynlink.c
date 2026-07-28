#include <efi/efi.h>
#include <efi/efilib.h>
#include "ldrerr.h"
#include "hashtab.h"
#include "dynlink.h"
#include "tsv.h"
#include <alloca.h>

//from bootloader.c (TODO: create bootloader.h)
#define CONV_PTR(p) (void*)(((UINT64)(p))|0xffff800000000000ull)
extern __attribute__((noreturn)) VOID Fatality(UINT32 code, UINT32 subcode);
extern UINTN ReadFile(EFI_FILE_HANDLE fileDir, CHAR16 *filename, VOID** buffer, UINTN addend);
extern VOID FileDestructor(VOID* file, UINTN filesz);

static EFI_FILE_HANDLE dl_workdir;

static hashtab_t global_exports;
static hashtab_t loaded_libs;

// uint32_t gnuhash(const char *name){
// 	uint32_t h = 5381;
// 	for (; *name; name++){
// 		h = h * 33 + (unsigned char)(*name);
// 	}
// 	return (size_t)h;
// }

#define foreach_needed(d, code) for (Elf64_Dyn *c = d; c->d_tag != DT_NULL; c++) { if (c->d_tag == DT_NEEDED) { code }}

#define USE_ALLOCA

void dl_init(EFI_FILE_HANDLE workdir){
	dl_workdir = workdir;
	ht_init(&global_exports, sizeof(symbol_t), 16);
	ht_init(&loaded_libs, sizeof(libid_t), 4);
}

static void parse_exports(hashtab_t *const ctx, libid_t *lib){
	for (size_t i = 0; i < lib->symcnt; i++){
		if (lib->dynsym[i].st_shndx != STN_UNDEF && lib->dynsym[i].st_value != 0 && ELF64_ST_BIND(lib->dynsym[i].st_info) == STB_GLOBAL){
			symbol_t sym = {
				.addr = (uint64_t)CONV_PTR(lib->image + lib->dynsym[i].st_value)
			};
			ht_insert(ctx, &lib->dynstr[lib->dynsym[i].st_name], &sym);
		}
	}
}

static libid_weakptr get_file(hashtab_t *const ctx, char* path){
	//checking if library loaded already
	libid_weakptr ret = ht_read(&loaded_libs, path);
	if (ret) return ret;
	libid_t nl; //new library

	//convert path to UTF-16LE
	CHAR16* pool = makechar16(path);
	if (!pool) goto fail;

	nl.imagesize = ReadFile(dl_workdir, pool, &nl.image, 0);
	//Print(L"image: %p\n", nl.image);

	if (!nl.image) goto fail;
	if (check_elf(nl.image)) goto fail;

	nl.dynamic = get_dyn(nl.image);
	if (!nl.dynamic) goto fail;

	nl.dynsym = off2ptr(nl.image,get_dynent(nl.dynamic, DT_SYMTAB));
	nl.dynstr = off2ptr(nl.image,get_dynent(nl.dynamic, DT_STRTAB));
	Elf64_Shdr *ds = get_section_by_name(nl.image, ".dynsym");
	if (!ds) goto fail;
	nl.symcnt = ds->sh_size/sizeof(Elf64_Sym);
	nl.rela = off2ptr(nl.image,get_dynent(nl.dynamic, DT_RELA));
	nl.relacnt = (get_dynent(nl.dynamic, DT_RELASZ)->d_un.d_val)/sizeof(Elf64_Rela);
	if (!nl.dynsym || !nl.dynstr || !nl.symcnt || !nl.rela || !nl.relacnt){
		goto fail;
	}
	//Print(L"NL rela: %p\n", nl.rela);
	//Print(L"NL relacnt: %p\n", nl.relacnt);

	foreach_needed(nl.dynamic, 
		nl.neededcnt++;
	)

	if ((ret = ht_insert(&loaded_libs, path, &nl))){ 
		parse_exports(ctx, &nl);
		FreePool(pool);
		return ret;
	}

fail:
	FileDestructor(nl.image, nl.imagesize);
	Print(L"FATAL: Unable to process file: %s\n", pool);
	FreePool(pool);
	Fatality(LDR_FILE, 1);
}

static void setup_namespace(hashtab_t *const ctx, libid_t *lib){
	foreach_needed(lib->dynamic,
		size_t old_size = ctx->entries;
		libid_weakptr needed_lib = get_file(ctx, &lib->dynstr[c->d_un.d_val]);
		if (old_size != ctx->entries) setup_namespace(ctx, needed_lib);
		//already loaded, stop recursion
	)
}

static void resolve(hashtab_t *const ctx, libid_t *lib){
	if (!lib->neededcnt) return; //already resolved, stop recursion
	//Print(L"R rela: %p\n", lib->rela);
	
	for (size_t i = 0; i < lib->relacnt; i++){
		void* got_entry = lib->image+lib->rela[i].r_offset;
		//Print(L"%llu ", ELF64_R_TYPE(lib->rela[i].r_info));
		switch (ELF64_R_TYPE(lib->rela[i].r_info)){
			case R_X86_64_RELATIVE:
				//Print(L"RELATIVE\n");
				*(uint64_t*)got_entry = (uint64_t)CONV_PTR(lib->image+lib->rela[i].r_addend);
				break;
			case R_X86_64_JUMP_SLOT:
				Print(L"Warning: forced eager binding for R_X86_64_JUMP_SLOT\n");
				/* FALLTHRU */
			case R_X86_64_GLOB_DAT:
				//Print(L"GLOB_DAT\n");
				char *symname = &lib->dynstr[lib->dynsym[ELF64_R_SYM(lib->rela[i].r_info)].st_name];
				symbol_t *target = ht_read(ctx, symname);
				if (!target && ctx != &global_exports) target = ht_read(&global_exports, symname);
				if (!target) Fatality(LDR_ELF, 24);
				*(uint64_t*)got_entry = target->addr + lib->rela[i].r_addend;
				break;
			case R_X86_64_NONE:
				//Print(L"NONE\n");
				break;
			default:
				Fatality(LDR_ELF, ELF64_R_TYPE(lib->rela[i].r_info));
				//break;
		}
	}
	lib->neededcnt = 0; //resolved

	foreach_needed(lib->dynamic,
		// no more insertions to loaded_libs
		resolve(ctx, ht_read(&loaded_libs, &lib->dynstr[c->d_un.d_val]));
	)
}

libid_weakptr dl_process(char *path, const bool global_f){
#ifndef USE_ALLOCA
	hashtab_t local;
	hashtab_t *const ctx = global_f ? &global_exports : &local;
#else
	hashtab_t *const ctx = global_f ? &global_exports : alloca(sizeof(hashtab_t));
#endif
	if (!global_f) ht_init(ctx, sizeof(symbol_t), 8);

	libid_weakptr module = get_file(ctx, path);

	if (!module) Fatality(LDR_MEM, 2);

	setup_namespace(ctx, module);
	resolve(ctx, get_file(ctx, path));

	module = get_file(ctx, path);

	// destroy ctx if local
	if (!global_f) ht_destroy(ctx);
	return module;
}

void dl_fini(){
	ht_destroy(&global_exports);
	ht_destroy(&loaded_libs);
}