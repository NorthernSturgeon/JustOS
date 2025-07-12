#ifndef __BL_JSON_H__
#define __BL_JSON_H__

#define TYPE_NULL 0
#define TYPE_BOOL 1
#define TYPE_INT 2
#define TYPE_STR 3

typedef struct __attribute__((__packed__)){
	UINT16 offset;
	UINT8 type;
	UINT8 size;
} JsonValue_t;

extern UINT8 SetBuffer(CHAR8 *buf, UINT16 len);
extern JsonValue_t Search(CHAR8 *name);
extern UINT8 GetValue(VOID *dest, JsonValue_t value_type);

#endif