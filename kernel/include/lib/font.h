#ifndef __FONT_H__
#define __FONT_H__

typedef unsigned char uint8_t;

extern uint8_t* font_symbol_tab[];
extern const size_t font_symbol_tab_size;

extern const uint8_t font_width;
extern const uint8_t font_height;
extern const uint8_t font_full_width;
extern const uint8_t font_full_height;

extern uint8_t *get_symbol_by_id(uint8_t *symbol_tab[], uint8_t sym_id);

#endif