#ifndef __BL_TSV_H__
#define __BL_TSV_H__

extern CHAR8* tsv_init(CHAR8* file, UINTN filesz);
extern CHAR8* tsv_search(CHAR8* base, char* key, UINTN idx);
extern INTN tsv_parseint(CHAR8* base);
extern CHAR16* makechar16(CHAR8* base);

#endif