/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * \file dynlink.c
 */

#include "bootloader.h"
#include "ldrerr.h"
#include "hashtab.h"
#include "dynlink.h"
#include "tsv.h"
#include <alloca.h>

static EFI_FILE_HANDLE dl_workdir;
static hashtab_t global_exports;
static hashtab_t loaded_libs;

#define foreach_needed(d, code) for (Elf64_Dyn *c = d; c->d_tag != DT_NULL; c++) { if (c->d_tag == DT_NEEDED) { code }}

void dl_init(EFI_FILE_HANDLE workdir){
	dl_workdir = workdir;
	ht_init(&global_exports, sizeof(symbol_t), 16);
	ht_init(&loaded_libs, sizeof(libid_t), 4);
}

static void parse_exports(hashtab_t *const ctx, libid_t *lib){
	for (size_t i = 0; i < lib->symcnt; i++){
		if (lib->dynsym[i].st_shndx != STN_UNDEF && lib->dynsym[i].st_value != 0 && ELF64_ST_BIND(lib->dynsym[i].st_info) != STB_LOCAL){
			if (!ht_read(ctx, &lib->dynstr[lib->dynsym[i].st_name])){
				symbol_t sym = {
					.addr = (uint64_t)(lib->image + lib->dynsym[i].st_value)
				};
				ht_insert(ctx, &lib->dynstr[lib->dynsym[i].st_name], &sym);
			}
		}
	}
}

static libid_weakptr get_file(hashtab_t *const ctx, char* path){
	//checking if library loaded already
	libid_weakptr ret = ht_read(&loaded_libs, path);
	if (ret) return ret;
	libid_t nl = {0}; //new library

	//convert path to UTF-16LE
	CHAR16* pool = makechar16(path);
	if (!pool) goto fail;

	nl.imagesize = ReadFile(dl_workdir, pool, &nl.image, 0);
	//Print(L"image: %p\n", nl.image);

	if (!nl.image || check_elf(nl.image)) goto fail;

	Elf64_Shdr *bss = get_section_by_name(nl.image, ".bss");
	size_t bss_size = bss ? bss->sh_size : 0;
	if (bss_size){
		void *image_with_bss;
		if (ALLOC_KPAGES(&image_with_bss, TO_PAGES(nl.imagesize + bss_size)) == EFI_SUCCESS){
			CopyMem(image_with_bss, nl.image, nl.imagesize);
			ZeroMem(image_with_bss + bss->sh_offset, bss_size);
			FileDestructor(nl.image, nl.imagesize);
			nl.image = image_with_bss;
			nl.imagesize += bss_size;
		} else {
			goto fail;
		}
	}

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

	nl.tls_area = NULL;
	nl.tls_size = 0;

	Elf64_Phdr* percpu = get_segment_by_type(nl.image, PT_TLS);
	if (percpu){
		nl.tls_size = percpu->p_memsz;
		if (EFI_ERROR(ALLOC_KPAGES(&nl.tls_area, nl.tls_size))) goto fail;
		ZeroMem(nl.tls_area + percpu->p_filesz, percpu->p_memsz - percpu->p_filesz); // for .tbss
		CopyMem(nl.tls_area, nl.image + percpu->p_offset, percpu->p_filesz); // copy .tdata
	}

	foreach_needed(nl.dynamic, 
		nl.neededcnt++;
	)

	nl.soname = NULL;
	for (Elf64_Dyn *c = nl.dynamic; c->d_tag != DT_NULL; c++) {
		if (c->d_tag == DT_SONAME) {
			nl.soname = &nl.dynstr[c->d_un.d_val];
			break;
		}
	}

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
	if (lib->neededcnt == SIZE_MAX) return; //already resolved, stop recursion
	Print(L"R rela: %p needed %llu\n", lib->rela, lib->neededcnt);
	
	for (size_t i = 0; i < lib->relacnt; i++){
		void* got_entry = lib->image + lib->rela[i].r_offset;
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
				// if (!strcmpa(symname,"_DTV_") && ELF64_ST_BIND(lib->dynsym[ELF64_R_SYM(lib->rela[i].r_info)].st_info) == STB_WEAK) break;

				symbol_t *target = ht_read(ctx, symname);
				if (!target && ctx != &global_exports) target = ht_read(&global_exports, symname);
				if (!target && ELF64_ST_BIND(lib->dynsym[ELF64_R_SYM(lib->rela[i].r_info)].st_info) == STB_GLOBAL){
					CHAR16* err_name = makechar16(symname);
					Print(L"Unable to found symbol: %s\n", err_name);
					Fatality(LDR_ELF, 24);
				}
				*(uint64_t*)got_entry = (target ? (uint64_t)CONV_PTR(target->addr) : 0) + lib->rela[i].r_addend;

				break;
			case R_X86_64_NONE:
				//Print(L"NONE\n");
				break;
			case R_X86_64_DTPMOD64:
				Print(L"R_X86_64_DTPMOD64@%p <- %p\n", got_entry, CONV_PTR(lib->image + 0x1000));
				*(uint64_t*)got_entry = (uint64_t)CONV_PTR(lib->image + 0x1000);
				break;
			default:
				Fatality(LDR_ELF, ELF64_R_TYPE(lib->rela[i].r_info));
		}
	}
	lib->neededcnt = SIZE_MAX; //resolved

	foreach_needed(lib->dynamic,
		// no more insertions to loaded_libs
		resolve(ctx, ht_read(&loaded_libs, &lib->dynstr[c->d_un.d_val]));
	)

	if (lib->ctx && ctx != &global_exports){
		ht_destroy(ctx);
		FreePool(ctx);
	}
}

libid_weakptr dl_process(char *path, const BOOLEAN global_f){
	hashtab_t *const ctx = global_f ? &global_exports : AllocatePool(sizeof(hashtab_t));

	// init ctx if local
	if (!global_f) ht_init(ctx, sizeof(symbol_t), 8);

	libid_weakptr module = get_file(ctx, path);

	if (!module) Fatality(LDR_MEM, 2);

	setup_namespace(ctx, module);

	module = get_file(ctx, path);
	module->ctx = ctx;

	return module;
}

void dl_fini(loaded_file **files, size_t *files_cnt){
	*files_cnt = 0;
	if (ALLOC_KPAGES(files, TO_PAGES(loaded_libs.entries*sizeof(loaded_file))) == EFI_SUCCESS){
		for (size_t i = 0; i < loaded_libs.capacity; i++){
			ht_entry_t* ent = ht_index(&loaded_libs, i);
			if (ent->key){
				libid_t *lib = (libid_t*)((uint8_t*)ent + sizeof(ht_entry_t));
				if (lib->ctx) resolve(lib->ctx, lib);

				loaded_file *file = *files + (*files_cnt)++;
				file->name = lib->soname ? CONV_PTR(lib->soname) : NULL;
				file->data = CONV_PTR(lib->image);
				file->size = lib->imagesize;

				if (lib->tls_area){
					file->tls_area = CONV_PTR(lib->tls_area);
					file->tls_size = lib->tls_size;
					*(uint64_t*)(lib->image + 0x0ff8) = (uint64_t)file->tls_area;
				} else {
					file->tls_area = NULL;
				}
			}
		}
		ht_destroy(&global_exports);
	} else {
		Fatality(LDR_MEM, 44);
	}

	ht_destroy(&loaded_libs);
}