#include <efi/efi.h>
#include <efi/efilib.h>

#include "json.h"

static CHAR8 volatile *buffer = NULL;
static UINT16 lenght = 0;

static CHAR8 digits = "-0123456789";

UINT8 SetBuffer(CHAR8 *buf, UINT16 len){
	if (buf[0] == '{' && buf[len-1] == '}'){
		buffer = buf;
		lenght = len-1;
		return 0;
	}
	return 1;
}

static inline UINT8 cmp(CHAR8 *src1, CHAR8 *src2, UINT8 n){
	for (UINT8 i=0;src1[i]==src2[i]&&i<n;i++);
	return !(i-n);
}

JsonValue_t Search(CHAR8 *name){
	JsonValue_t ret;
	UINT8 offset2;
	struct {
		uint8_t inStr:1;
		uint8_t inVal:1;
		uint8_t founded:1;
	} flags = {0;0;0};
	for (UINT16 offset=1;offset<lenght;offset++){
		if (buffer[offset] == '"'){
			flags.inStr = TRUE;
			continue
		}
		if (!flags.inStr) {
			if (buffer[offset] == ':') flags.inVal = TRUE;
			else if (buffer[offset] == ',') flags.inVal = FALSE;
			continue;
			}
		if (!flags.founded && flags.inStr && !flags.inVal){
			founded = cmp(&buffer[offset], name, strlena(name));
		}
		if (flags.founded && flags.inVal){
			ret.offset = offset;
			if (flags.inStr){
				ret.type = TYPE_STR;
				for (;buffer[offset+offset2]!='"';offset2++);
				ret.size = offset2;
			} else {
				for (offset2=0;offset2<11;offset2++){
					if (buffer[offset] == digits[offset2]){
						ret.type = TYPE_INT;
						ret.size = 8;
						break;
					}
				}
				if (!ret.type){
					size = 1;
					if (buffer[offset] == 'n') ret.type = TYPE_NULL;
					else ret.type = TYPE_BOOL;
				}
			}
			return ret;
		}
	}
	return ret;
}

UINT8 GetValue(VOID *dest, JsonValue_t val_type){
	if (!val_type.size) return 1;
	ZeroMem(dest, val_type.size);
	switch (val_type.type){
		case TYPE_NULL: return 2;
		case TYPE_BOOL:
			if (buffer[val_type.offset] == 't') *(BOOL*)dest = TRUE;
			break;
		case TYPE_STR:
			CopyMem(dest, &buffer[val_type.offset], val_type.size);
			((CHAR8*)dest)[val_type.size] = 0;
			break;
		case TYPE_INT:
			INT64 ret;
			UINT64 factor = 1;
			UINT8 sign, cur_pos;
			if (buffer[val_type.offset] == '-') sign++;
			for (;buffer[val_type.offset+cur_pos+sign]!=',';cur_pos++);
			for (--cur_pos;cur_pos!=0;cur_pos++){
				ret += (buffer[val_type.offset+cur_pos]-48)*factor;
				factor *= 10;
			}
			if (sign) ret = -ret;
			*(INT64*)dest = ret;
			break;
		default: return 3;
	}
	return 0;
}