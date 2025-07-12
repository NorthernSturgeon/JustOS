#ifndef __VIDEO_H__
#define __VIDEO_H__

extern uint32_t get_res();
extern void init_video(void* vram_ptr, uint16_t w, uint16_t h);
extern void draw_pixel(uint32_t color, uint16_t x, uint16_t y);
extern void fill_rect(uint32_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
extern void draw_by_font_bitmap(uint8_t bitmap[], uint32_t color, uint16_t x, uint16_t y);

#endif