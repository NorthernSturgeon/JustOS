#include "lib/types.h"
#include "lib/font_table.h"

uint8_t *get_symbol_by_id(uint8_t* *sym_tab, uint8_t sym_id){
	for (size_t i = 0; i < font_offset_table_size; i++){
		if ((sym_id >= font_offset_table[i][0]) && (sym_id <= font_offset_table[i][1]))
			return sym_tab[sym_id-font_offset_table[i][2]];
	}
	return undefined;
}