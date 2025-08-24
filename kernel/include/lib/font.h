#ifndef __FONT_H__
#define __FONT_H__

typedef unsigned char uint8_t;

extern volatile void* font_symbol_table[];
extern size_t font_symbol_table_size;

extern uint16_t font_width;
extern uint16_t font_height;
extern uint16_t font_full_width;
extern uint16_t font_full_height;

void init_kernel_font(void* kernel);
extern uint8_t* get_symbol_by_id(uint8_t sym_id);

#endif