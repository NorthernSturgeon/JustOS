#include "lib/types.h"
//#include "lib/memory.h"
#include <stdarg.h>
#include "lib/font.h"
#include "video.h"
#include "lib/console.h"
#include "register.h"

static uint8_t column_width = 60;

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

void _puts(const char *str, char c){
	struct screen_resolution sr = get_res();
	uint16_t w = sr.width;
	uint16_t h = sr.height;

	for (; *str != c; str++){
		if (*str == '\r') {x = basex; continue;}
		if (*str == '\n') {x = basex; y += font_full_height;continue;}
		if (x >= w-font_width) {x = basex; y += font_full_height;}
		if (y >= h-font_full_height) {y = basey; basex += column_width*font_full_width; x = basex;}
		if (basex >= w-font_full_width*column_width) {basex = 0; scrool_flag = true; x = basex;}

		fill_rect(backcolor,x,y,font_full_width,font_full_height);
		draw_by_font_bitmap(get_symbol_by_id(*str), forecolor, x, y);
		if (scrool_flag){
			fill_rect(forecolor, basex, y+font_height, column_width*font_full_width, 1);
			if (y >= font_full_height) fill_rect(backcolor, basex, y+font_height-font_full_height, column_width*font_full_width, 1);
		}
		x += font_full_width;
	}
}

#define puts(s) _puts(s, 0)

void printf(const char *str, ...){
	uint64_t u_buffer;
	unsigned char p_buffer[8] = {0};
	va_list data;
	va_start(data, str);
	const char *str0 = str;
	for (;*str;str++){
		if (*str == '%') {
			_puts(str0, '%');
			str0 = str + 2;
			char out_buffer[21] = {0};
			switch (*++str){
			case '%':
				out_buffer[0] = '%';
				break;
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
				puts(va_arg(data, char*));
				continue;
			case 'd':
				int64_t d_buffer = va_arg(data, int64_t);
				uint8_t i = 0;
				if (d_buffer < 0){
					out_buffer[i++] = '-';
					u_buffer = -d_buffer;
				}
				goto i2a;
			case 'u':
				u_buffer = va_arg(data, uint64_t);
				i = 0;
				i2a:
				if (!u_buffer) {out_buffer[0] = '0'; break;}
				while (u_buffer > 0){
					out_buffer[i] = (u_buffer%10)+0x30;
					u_buffer /= 10;
					i++;
				}
				char *out = (char*)((uint64_t)&out_buffer+(uint64_t)i);
				*out-- = 0;
				char *start = (char*)&out_buffer;
				for (;start < out; start++, out--){
					char temp=*start;
					*start=*out;
					*out=temp;
				}
				break;
			}
			puts(out_buffer);
		}
	}
	puts(str0);
	va_end(data);
}
