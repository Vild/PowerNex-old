#ifndef VBE_H_
#define VBE_H_

#include <powernex/powernex.h>
#include <powernex/multiboot.h>

typedef struct vbe_modeinfo {
  uint16_t attributes;
  uint8_t winA,winB;
  uint16_t granularity;
  uint16_t winsize;
  uint16_t segmentA, segmentB;
  uint32_t realFctPtr;
  uint16_t pitch; // bytes per scanline
 
  uint16_t Xres, Yres;
  uint8_t Wchar, Ychar, planes, bpp, banks;
  uint8_t memory_model, bank_size, image_pages;
  uint8_t reserved0;
 
  uint8_t red_mask, red_position;
  uint8_t green_mask, green_position;
  uint8_t blue_mask, blue_position;
  uint8_t rsv_mask, rsv_position;
  uint8_t directcolor_attributes;
 
  uint32_t physbase;  // your LFB (Linear Framebuffer) address ;)
  uint32_t reserved1;
  uint16_t reserved2;
} __attribute__((packed)) vbe_modeinfo_t;

void vbe_init(multiboot_info_t * multiboot);
void vbe_putPixel(int x, int y, int rgb);
void vbe_putRect(int x, int y, int width, int height, int rgb);
void vbe_putLine(int x1, int y1, int x2, int y2, int rgb);
void vbe_putCircle(int x0, int y0, int radius, int rgb);

void vbe_drawChar(int x, int y, uint16_t c, int rgb);
void vbe_print(char * str, int x, int y, int rgb);

#endif
