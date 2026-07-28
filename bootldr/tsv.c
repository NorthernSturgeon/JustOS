#include <efi/efi.h>
#include <efi/efilib.h>

/*
TODO: documentation
*/
CHAR8* tsv_init(CHAR8* file, UINTN filesz){
	UINTN delta = 1;
	//Stage 1: calculation (optimized loop)
	for (UINTN i = 0; i < filesz; i++){
		if (file[i] == '\t') file[i] = '\0';
		else if (file[i] == '\n'){
			if (i < filesz-1 && file[i+1] == '\n') {
				file[i] = '\r'; //goto r-case
				goto r;
			}
			else delta++;
		} else if (file[i] == '\r') r: delta--;//r-case
	}
	if (file[filesz-1] != '\n') delta++;

	//Intermediate: memory allocation
	filesz += delta;
	CHAR8 *base = AllocatePool(filesz);
	if (!base) return NULL;

	//Stage 2: copy
	for (UINTN b = 0, f = 0; b < filesz && f < filesz-delta; b++, f++){
		while (file[f] == '\r') f++;
		if (file[f] == '\n') base[b++] = '\0';
		base[b] = file[f];
	}
	base[filesz-2] = '\n';
	base[filesz-1] = '\r';
	return base;
}

CHAR8* tsv_search(CHAR8* base, char* key, UINTN idx){
	//search line
	while (strcmpa(base, key)){
		while (*base != '\n') base++;
		//EOF: \n\r
		if (*(++base) == '\r') return NULL;
	}
	//search column
	UINTN ctr = 0;
	while (ctr < idx){
		if (*(base++) == '\0') ctr++;
		if (*base == '\n') return NULL;
	}
	return base;
}

UINTN tsv_listlen(CHAR8* base){
	if (!base){
		Print(L"tsv_listlen: base=NULL!!!\n");
		for(;;);
	}
	UINTN ret = 0;
	for (; *base != '\n'; base++){
		if (*base == '\0') ret++;
	}
	return ret;
}

CHAR8* tsv_next(CHAR8* base){
	if (!base){
		Print(L"tsv_listlen: base=NULL!!!\n");
		for(;;);
	}
	for (; *base != '\0'; base++){
		if (*base == '\n') return NULL;
	}
	return ++base;
}

INTN tsv_parseint(CHAR8* base){
	if (!base){
		Print(L"tsv_parseint: base=NULL!!!\n");
		for(;;);
	}
	INTN ret = 0;
	while (*base) base++;
	base--;
	UINTN factor = 1;
	while (*base != '\0' && *base != '-'){
		ret += (*base - 0x30)*factor;
		factor *= 10;
		base--;
	}
	if (*base == '-') ret = -ret;
	return ret;
}

CHAR16* makechar16(CHAR8* base){
	if (!base) return NULL;
	UINTN len = 1 + strlena(base);
	CHAR16* pool = AllocatePool(len*2);
	if (pool) {
		pool[len] = 0;
		for (UINTN idx = 0; idx < len; idx++) pool[idx] = base[idx];
	}
	return pool;
}
