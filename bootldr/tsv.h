#ifndef __BL_TSV_H__
#define __BL_TSV_H__

extern CHAR8* tsv_search(CHAR8* base, char* key, UINTN idx);
extern INTN tsv_parseint(CHAR8* base);
extern CHAR16* tsv_parsestr(CHAR8* base);

#endif