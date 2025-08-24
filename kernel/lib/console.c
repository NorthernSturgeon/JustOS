#include "lib/types.h"
//#include "lib/memory.h"
#include <stdarg.h>
#include "lib/font.h"
#include "video.h"
#include "lib/console.h"
#include "register.h"

static uint8_t column_width = 40;

static uint16_t basex = 0;
static uint16_t basey = 0;
static uint16_t x = 0;
static uint16_t y = 0;

static uint8_t scrool_flag = false;

static uint32_t forecolor;
static uint32_t backcolor;

void set_color(uint32_t fc, uint32_t bc){
	forecolor = fc;
	backcolor = bc;
}

void set_column_width(uint8_t cw){
	if (cw > 0 && cw < get_console_info().width-1) column_width = cw;
}

struct console_info get_console_info(void){
	struct screen_resolution sr = get_res();
	struct console_info cs;
	cs.width = sr.width/font_full_width;
	cs.height = sr.height/font_full_height;
	cs.column_width = column_width;
	return cs;
}

void printf(char *str, ...){
	struct screen_resolution sr = get_res();
	uint16_t w = sr.width;
	uint16_t h = sr.height;
	uint64_t u_buffer;
	unsigned char p_buffer[8] = {0}; // :) char is signed (linux)
	va_list data;
	va_start(data, str);
	for (;*str;str++){
		if (*str == '\r') {x = basex; continue;}
		if (*str == '\n') {x = basex; y += font_full_height;continue;}
		if (x >= w-font_width) {x = basex; y += font_full_height;}
		if (y >= h-font_full_height) {y = basey; basex += column_width*font_full_width; x = basex;}
		if (basex >= w-font_full_width*column_width) {basex = 0; scrool_flag = true; x = basex;}
		if (*str == '%') {
			//write_creg(r15, &out_buffer);
			char out_buffer[17] = {0};
			switch (*++str){
			case '\0':
				return;
			case 'p':
				*((uint64_t*)&p_buffer) = va_arg(data, uint64_t);
				for (uint8_t i = 0; i < 8; i++){
					out_buffer[15-2*i] = (p_buffer[i]&15)+0x30;
					if (out_buffer[15-2*i] >= 0x3A) out_buffer[15-2*i] += 7;
					out_buffer[14-2*i] = (p_buffer[i]>>4)+0x30;
					if (out_buffer[14-2*i] >= 0x3A) out_buffer[14-2*i] += 7;
				}
				break;
			case 's':
				printf(va_arg(data, char*));
				continue;
			case 'u':
				u_buffer = va_arg(data, uint64_t);
				if (!u_buffer) {out_buffer[0] = '0'; break;}
				uint8_t i = 0;
				while (u_buffer > 0){
					out_buffer[i] = (u_buffer%10)+0x30;
					u_buffer /= 10;
					i++;
				}
				char *out = (char*)((uint64_t)&out_buffer+(uint64_t)i);
				*out-- = 0;
				char *start = (char*)&out_buffer;
				char temp;
				while (start < out){
					temp=*start;
					*start=*out;
					*out=temp;
					start++;
					out--;
				}
			}
			printf(out_buffer);
			continue;
		}
		//fill_rect(backcolor,x,y,font_full_width,font_full_height);
		fill_rect(backcolor,x,y,font_full_width,font_full_height);
		draw_by_font_bitmap(get_symbol_by_id(*str), forecolor, x, y);
		if (scrool_flag){
			fill_rect(forecolor, basex, y+font_height, column_width*font_full_width, 1);
			if (y >= font_full_height) fill_rect(backcolor, basex, y+font_height-font_full_height, column_width*font_full_width, 1);
		}
		x += font_full_width;
	}
	va_end(data);
}
