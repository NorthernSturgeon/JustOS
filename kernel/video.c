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

static uint32_t *vram = NULL;
static uint16_t width, height, format, scanline;

#define WHERE uint32_t where = width*y+x
//#define SET_PIXEL(dest,pos,r,g,b) dest[pos] = r; dest[pos+1] = g; dest[pos+2] = b

static void color_correct(uint32_t *color){
	if (format != PixelBlueGreenRedReserved8BitPerColor) return;
	*color = ((*color & 0x000000FFu) << 16) | ((*color & 0x00FF0000u) >> 16) | (*color & 0xFF00FF00u);
}

void init_video(void* vram_ptr, uint16_t w, uint16_t h, uint16_t ppl, uint16_t f){
	vram = vram_ptr;
	width = w;
	height = h;
	format = f;
	scanline = ppl;
}

struct screen_resolution __attribute__((visibility("default"))) get_res(){
	struct screen_resolution sr;
	sr.width = width;
	sr.height = height;
	return sr;
}

void __attribute__((visibility("default"))) draw_pixel(uint32_t color, uint16_t x, uint16_t y){
	color_correct(&color);
	vram[scanline*y+x] = color;
}

void __attribute__((visibility("default"))) fill_rect(uint32_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	color_correct(&color);
    uint32_t *where = vram + (uint32_t)scanline * y + x;
    for (uint16_t cy = 0; cy < h; cy++) {
        for (uint16_t cx = 0; cx < w; cx++) {
            where[cx] = color;
        }
        where += scanline;
    }
}

void __attribute__((visibility("default"))) draw_by_font_bitmap(uint8_t bitmap[] ,uint32_t color, uint16_t x, uint16_t y){
	// cx - current x, cy - current y
	color_correct(&color);
    uint32_t *where = vram + (uint32_t)scanline * y + x;
	for (uint16_t cy = 0; cy < font_height; cy++){
		for (uint16_t cx = 0; cx <= font_width; cx++){
			if (bitmap[cy] & (1 << (7-cx))) where[cx] = color;
		}
		where += scanline;
	}
	
	//draw_pixel(color, x+3, y+3);
}
