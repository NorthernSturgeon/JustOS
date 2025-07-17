#include "lib/types.h"
#include "lib/font.h"
//#include "lib/mem.h"

static uint32_t volatile *vram = NULL;
static uint16_t width, height;

#define WHERE uint32_t where = width*y+x
//#define SET_PIXEL(dest,pos,r,g,b) dest[pos] = r; dest[pos+1] = g; dest[pos+2] = b

void init_video(void* vram_ptr, uint16_t w, uint16_t h){
	vram = vram_ptr;
	width = w;
	height = h;
	//format = format;
}

uint32_t get_res(){
	return (((uint32_t)width)<<16)+height;
}

void draw_pixel(uint32_t color, uint16_t x, uint16_t y){
	vram[width*y+x] = color;
}

void fill_rect(uint32_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	//uint32_t volatile *where = vram+4*(width*y+x);
	uint16_t cx, cy; // cx - current x, cy - current y
	for (cy = 0; cy < h; cy++){
		for (cx = 0; cx < w; cx++){
			//SET_PIXEL(buffer,where+cx,r,g,b);
			vram[width*(cy+y)+x+cx] = color;
		}
		//where += (width*y)*4;
	}
}

void draw_by_font_bitmap(uint8_t bitmap[], uint32_t color, uint16_t x, uint16_t y){
	WHERE;
	uint16_t cx, cy; // cx - current x, cy - current y
	for (cy = 0; cy <= font_height; cy++){
		for (cx = 0; cx <= font_width; cx++){
			//SET_PIXEL(buffer,where+cx,r,g,b);
			if (bitmap[cy] & (1 << (7-cx))) vram[where+cx] = color;
		}
		where += width;
	}
}
