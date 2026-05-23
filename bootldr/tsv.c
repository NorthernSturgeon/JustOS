#include <efi/efi.h>
#include <efi/efilib.h>

CHAR8* tsv_search(CHAR8* base, char* key, UINTN idx){
	while (strncmpa(base, key, strlena(key)) != 0){
		while (*base != '\n') base++;
		if (*(++base) == '\n') return NULL;
	}
	UINTN ctr = 0;
	while (ctr < idx){
		if (*(base++) == '\t') ctr++;
		if (*base == '\n') return NULL;
	}
	return base;
}

INTN tsv_parseint(CHAR8* base){
	if (!base) {
		Print(L"tsv_parseint: base=NULL!!!\n");
		for(;;);
	}
	INTN ret = 0;
	while (*base != '\t' && *base != '\n') base++;
	base--;
	UINTN factor = 1;
	while (*base != '\t'&& *base != '-'){
		ret += (*base - 0x30)*factor;
		factor *= 10;
		base--;
	}
	if (*base == '-') ret = -ret;
	return ret;
}

CHAR16* tsv_parsestr(CHAR8* base){
	if (!base) return NULL;
	UINTN len = 1;
	while (*base != '\t' && *base != '\n'){
		base++;
		len++;
	}
	CHAR16* pool = AllocateZeroPool(len*2);
	if (pool) {
		base -= --len;
		for (UINTN idx = 0; idx < len; idx++) pool[idx] = base[idx];
	}
	return pool;
}
