#include <powernex/io/vbe.h>
#include <powernex/cpu/task.h>
#include <powernex/string.h>
#include <powernex/mem/heap.h>
#include <powernex/io/port.h>
#include <powernex/io/textmode.h>
#include <powernex/text/font.h>
#include <powernex/text/logo.h>
#include <powernex/text/cool.h>

#include <powernex/math.h>

extern const struct bitmap_font font;

static int screenRefresh(void * arg);
static void paintBackground();

static vbe_modeinfo_t * modeInfo;
static uint8_t * screenBuffer;
static uint32_t screenBufferSize;
static task_t * screenRefreshTask;

uint16_t width, height;

static uint32_t stack[0x1000];

void vbe_init(multiboot_info_t * multiboot) {
	modeInfo = (vbe_modeinfo_t *)multiboot->vbe_mode_info;

	width = modeInfo->Xres;
	height = modeInfo->Yres;
	screenBufferSize = width * height * (modeInfo->bpp / 8);
	screenBuffer = kmalloc(screenBufferSize);

	screenRefreshTask = task_create(&screenRefresh, NULL, stack + (0x1000/sizeof(uint32_t)));
	task_start(screenRefreshTask);

	kprintf("physBase: 0x%X, width: %d, height: %d\n", modeInfo->physbase, width, height);

	paintBackground();
	memcpy((void *)modeInfo->physbase, screenBuffer, screenBufferSize);
}

static int screenRefresh(UNUSED void * arg) {
	while(true) {
		paintBackground();
		while ((inb(0x3DA) & 0x08))
			task_next();
		while (!(inb(0x3DA) & 0x08))
			task_next();
		memcpy((void *)modeInfo->physbase, screenBuffer, screenBufferSize);
		task_next();
	}
	return 0;
}

void vbe_putPixel(int x, int y, int rgb) {
//do not write memory outside the screen buffer, check parameters against the VBE mode info
	if (x < 0 || x > width || y < 0 || y > height)
		return;
	if (x)
		x = (x * (modeInfo->bpp>>3));
	if (y)
		y = (y * modeInfo->pitch);
	register uint8_t * cTemp;
	cTemp = &screenBuffer[x+y];
	cTemp[0] = rgb & 0xff;
	cTemp[1] = (rgb>>8) & 0xff;
	cTemp[2] = (rgb>>16) & 0xff;
}

void vbe_putRect(int x, int y, int width, int height, int rgb) {
	for (int x_ = x; x_ < x+width; x_++)
		for (int y_ = y; y_ < y+height; y_++)
			vbe_putPixel(x_, y_, rgb);
}

void vbe_putLine(int x0, int y0, int x1, int y1, int rgb) {
	//Bresenham's line algorithm
	int steep = abs(y1 - y0) > abs(x1 - x0);
	int inc = -1;

	if (steep) {
		int tmp = x0;
		x0 = y0;
		y0 = tmp;

		tmp = x1;
		x1 = y1;
		y1 = tmp;
	}

	if (x0 > x1) {
		int tmp = x0;
		x0 = x1;
		x1 = tmp;

		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	if (y0 < y1)
		inc = 1;

	int dx = abs(x0 - x1);
	int dy = abs(y1 - y0);
	int e = 0;
	int	y = y0;
	int x = x0;

	for (; x <= x1; x++) {
		if (steep)
			vbe_putPixel(y, x, rgb);
		else
			vbe_putPixel(x, y, rgb);
		
		if ((e + dy) << 1 < dx)
			e += dy;
		else {
			y += inc;
			e += dy - dx;
		}
	}
}

void vbe_putCircle(int x0, int y0, int radius, int rgb) {
  //Midpoint circle algorithm
  int x = radius;
  int y = 0;
  int radiusError = 1-x;
 
  while(x >= y) {
    vbe_putPixel( x + x0,  y + y0, rgb);
    vbe_putPixel( y + x0,  x + y0, rgb);
    vbe_putPixel(-x + x0,  y + y0, rgb);
    vbe_putPixel(-y + x0,  x + y0, rgb);
    vbe_putPixel(-x + x0, -y + y0, rgb);
    vbe_putPixel(-y + x0, -x + y0, rgb);
    vbe_putPixel( x + x0, -y + y0, rgb);
    vbe_putPixel( y + x0, -x + y0, rgb);
    y++;
    if (radiusError < 0)
      radiusError += 2 * y + 1;
    else {
      x--;
      radiusError += 2 * (y - x) + 1;
    }
  }
}

void vbe_print(char * str, int x, int y, int rgb) {
	for (int i = 0; i < strlen(str); i++)
		vbe_drawChar(x + font.Width*i, y, str[i], rgb);
}

static void paintBackground() {
	vbe_putRect(0, 0, width, height, 0x000000);
	vbe_print("Hello World!, Hello Viewers :D", 10, 10, 0x00FFFFFF);


	vbe_putCircle(200, 200, 100, 0x00FF00);
	vbe_putCircle(200, 200, 90, 0x00FF00);
	

	for (uint32_t x = 0; x < logo_width*2; x++)
		for (uint32_t y = 0; y < logo_height*2; y++)
			vbe_putPixel(x+300, y+75, (
										 (logo_data[(x/2+(y/2*logo_width))*3+0] << 16) |
										 (logo_data[(x/2+(y/2*logo_width))*3+1] << 8)  |
										 (logo_data[(x/2+(y/2*logo_width))*3+2] << 0)));

	
	for (uint32_t x = 0; x < cool_width*2; x++)
		for (uint32_t y = 0; y < cool_height*2; y++)
			vbe_putPixel(x+300, y+200, (
										 (cool_data[(x/2+(y/2*cool_width))*3+0] << 16) |
										 (cool_data[(x/2+(y/2*cool_width))*3+1] << 8)  |
										 (cool_data[(x/2+(y/2*cool_width))*3+2] << 0)));
}

static const uint8_t * getFontPos(uint16_t c) {
	const uint16_t * arr = font.Index;
	for (uint16_t i = 0; i < font.Chars; i++) {
		if (arr[i] == c)
			return &font.Bitmap[i * font.Height];
		else if (arr[i] > c) //Because the list is sorted by size
			break;
	}
	return &font.Bitmap[0];
}

void vbe_drawChar(int x_, int y_, uint16_t c, int rgb) {
	const uint8_t * lines = getFontPos(c);
	for (int y = 0; y < font.Height; y++) {
		uint8_t line = lines[y];
		for (int x = 0; x < font.Width; x++) {
			if (line & (1 << x))
				vbe_putPixel(x_ + (font.Width - x), y_ + y, rgb);
		}
	}
}
