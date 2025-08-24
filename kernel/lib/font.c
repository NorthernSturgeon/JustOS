#include "lib/font_table.h"

void init_kernel_font(void* kernel){
	for (size_t i = 0; i < font_symbol_table_size; i++){
		font_symbol_table[i] += (uint64_t)kernel + 4096;
	}
}

uint8_t* get_symbol_by_id(uint8_t sym_id){
	for (size_t i = 0; i < font_offset_table_size; i++){
		if ((sym_id >= font_offset_table[i][0]) && (sym_id <= font_offset_table[i][1]))
			return (uint8_t*)font_symbol_table[sym_id-font_offset_table[i][2]];
	}
	return (uint8_t*)&undefined;
}