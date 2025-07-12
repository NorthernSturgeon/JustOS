#ifndef __CONSOLE_H__
#define __CONSOLE_H__
/*
//deprecated

#define BG_BLACK 0x00
#define BG_BLUE 0x10
#define BG_GREEN 0x20
#define BG_AQUA 0x30
#define BG_RED 0x40
#define BG_PURPLE 0x50
#define BG_YELLOW 0x60
#define BG_WHITE 0x70
#define BG_GRAY 0x80
#define BG_LBLUE 0x90
#define BG_LGREEN 0xA0
#define BG_LAQUA 0xB0
#define BG_LRED 0xC0
#define BG_LPURPLE 0xD0
#define BG_LYELLOW 0xE0
#define BG_LWHITE 0xF0

#define FT_BLACK 0x00
#define FT_BLUE 0x01
#define FT_GREEN 0x02
#define FT_AQUA 0x03
#define FT_RED 0x04
#define FT_PURPLE 0x05
#define FT_YELLOW 0x06
#define FT_WHITE 0x07
#define FT_GRAY 0x08
#define FT_LBLUE 0x09
#define FT_LGREEN 0x0A
#define FT_LAQUA 0x0B
#define FT_LRED 0x0C
#define FT_LPURPLE 0x0D
#define FT_LYELLOW 0x0E
#define FT_LWHITE 0x0F

#define COLOR(BG, FT) (BG|FT)
*/

//extern void scroll(uint8_t count);

extern void set_color(uint32_t fc, uint32_t bc);
extern void print(char *string);
extern void printf(char *str, ...);

#endif