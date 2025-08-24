#ifndef __VIDEO_H__
#define __VIDEO_H__

struct screen_resolution{
	uint16_t width;
	uint16_t height;
};

extern struct screen_resolution get_res();
extern void init_video(void* vram_ptr, uint16_t w, uint16_t h);
extern void draw_pixel(uint32_t color, uint16_t x, uint16_t y);
extern void fill_rect(uint32_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
extern void draw_by_font_bitmap(uint8_t bitmap[], uint32_t color, uint16_t x, uint16_t y);

#endif