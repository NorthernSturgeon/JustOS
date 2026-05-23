#include "lib/types.h"
#include "lib/font.h"
#include "video.h"

typedef enum {
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

static uint32_t volatile *vram = NULL;
static uint16_t width, height, format, scanline;

#define WHERE uint32_t where = width*y+x
//#define SET_PIXEL(dest,pos,r,g,b) dest[pos] = r; dest[pos+1] = g; dest[pos+2] = b

static void color_correct(uint32_t *color){
	if (format != PixelBlueGreenRedReserved8BitPerColor) return;
	uint8_t *colorarray = (uint8_t*)color;
	uint8_t temp = colorarray[0];
	colorarray[0] = colorarray[2];
	colorarray[2] = temp;
}

void init_video(void* vram_ptr, uint16_t w, uint16_t h, uint16_t ppl, uint16_t f){
	vram = vram_ptr;
	width = w;
	height = h;
	format = f;
	scanline = ppl;
}

struct screen_resolution get_res(){
	struct screen_resolution sr;
	sr.width = width;
	sr.height = height;
	return sr;
}

void draw_pixel(uint32_t color, uint16_t x, uint16_t y){
	color_correct(&color);
	vram[scanline*y+x] = color;
}

void fill_rect(uint32_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	color_correct(&color);
	//uint32_t volatile *where = vram+4*(width*y+x);
	uint16_t cx, cy; // cx - current x, cy - current y
	for (cy = 0; cy < h; cy++){
		for (cx = 0; cx < w; cx++){
			//SET_PIXEL(buffer,where+cx,r,g,b);
			vram[scanline*(cy+y)+x+cx] = color;
		}
		//where += (width*y)*4;
	}
}

void draw_by_font_bitmap(uint8_t bitmap[] ,uint32_t color, uint16_t x, uint16_t y){
	// cx - current x, cy - current y
	color_correct(&color);
	for (uint16_t cy = 0; cy < font_height; cy++){
		for (uint16_t cx = 0; cx <= font_width; cx++){
			if (bitmap[cy] & (1 << (7-cx))) vram[scanline*(cy+y)+x+cx] = color;
		}
	}
	
	//draw_pixel(color, x+3, y+3);
}
