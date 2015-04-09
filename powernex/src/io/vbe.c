#include <powernex/io/vbe.h>
#include <powernex/cpu/task.h>
#include <powernex/string.h>
#include <powernex/mem/heap.h>
#include <powernex/io/port.h>
#include <powernex/io/textmode.h>
#include <powernex/text/font.h>
#include <powernex/math.h>

extern const struct bitmap_font font;

static void putPixel(int x, int y, int rgb);
static void putRect(int x, int y, int width, int height, int rgb);
static void putLine(int x1, int y1, int x2, int y2, int rgb);
static void putCircle(int x0, int y0, int radius, int rgb);

static int screenRefresh(void * arg);
static void paintBackground();
static void drawChar(int x, int y, uint16_t c, int rgb);

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

static void putPixel(int x, int y, int rgb) {
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

static void putRect(int x, int y, int width, int height, int rgb) {
	for (int x_ = x; x_ < x+width; x_++)
		for (int y_ = y; y_ < y+height; y_++)
			putPixel(x_, y_, rgb);
}

static void putLine(int x0, int y0, int x1, int y1, int rgb) {
	//Bresenham's line algorithm
	int deltax = x1 - x0;
	int deltay = y1 - y0;
	float error = 0;
	float deltaerr = abs((float)deltay / (float)deltax);
	int y = y0;
	int xd = (deltax < 0) ? -1 : 1;
	int yd = (deltay < 0) ? -1 : 1;
	int x = x0;
	while ((deltax < 0 && x > x1) || (deltax > 0 && x < x1)) {
		putPixel(x, y, rgb);
		error += deltaerr;
		while (error >= 0.5) {
			putPixel(x, y, rgb);
			y += yd;
			error -= 1.0;
			x += xd;
		}
	}
}

static void putCircle(int x0, int y0, int radius, int rgb) {
  int x = radius;
  int y = 0;
  int radiusError = 1-x;
 
  while(x >= y) {
    putPixel( x + x0,  y + y0, rgb);
    putPixel( y + x0,  x + y0, rgb);
    putPixel(-x + x0,  y + y0, rgb);
    putPixel(-y + x0,  x + y0, rgb);
    putPixel(-x + x0, -y + y0, rgb);
    putPixel(-y + x0, -x + y0, rgb);
    putPixel( x + x0, -y + y0, rgb);
    putPixel( y + x0, -x + y0, rgb);
    y++;
    if (radiusError < 0)
      radiusError += 2 * y + 1;
    else {
      x--;
      radiusError += 2 * (y - x) + 1;
    }
  }
}

static void paintBackground() {
	putRect(0, 0, width, height, 0x000000);
	int c = 0;
	for (uint16_t i = 'A'; i < 'Z'+1; i++, c++)
		drawChar(2+(font.Width+2)*c, 10, i, 0xFF00FF);

	putLine(150-50, 150-50, 150+50, 150+50, 0xFF0000);
	putLine(150-50, 150+50, 150+50, 150-50, 0x00FF00);
	putLine(250+50, 150-50, 250-50, 150+50, 0x0000FF);
	putLine(250+50, 150+50, 250-50, 150-50, 0xFFFF00);

	putCircle(400, 150, 25, 0xFF00FF);
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

static void drawChar(int x_, int y_, uint16_t c, int rgb) {
	const uint8_t * lines = getFontPos(c);
	for (int y = 0; y < font.Height; y++) {
		uint8_t line = lines[y];
		for (int x = 0; x < font.Width; x++) {
			if (line & (1 << x))
				putPixel(x_ + (font.Width - x), y_ + y, rgb);
		}
	}
}
