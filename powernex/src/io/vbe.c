#include <powernex/io/vbe.h>
#include <powernex/cpu/task.h>
#include <powernex/string.h>
#include <powernex/mem/heap.h>
#include <powernex/io/port.h>
#include <powernex/io/textmode.h>
#include <powernex/text/font.h>
#include <powernex/text/logo.h>
#include <powernex/text/cool.h>
#include <powernex/io/textmode.h>
#include <powernex/math.h>

extern const struct bitmap_font font;

static int screenRefresh(void * arg);
static void paintBackground();
static void createLookupTable();

static uint16_t width, height;

static vbe_modeinfo_t * modeInfo;
static uint8_t * screenBuffer;
static uint32_t screenBufferSize;
static task_t * screenRefreshTask;

static const uint8_t * charLookup[UINT16_MAX + 1];

static uint32_t stack[0x1000];

void vbe_init(multiboot_info_t * multiboot) {
	modeInfo = (vbe_modeinfo_t *)multiboot->vbe_mode_info;

	width = modeInfo->Xres;
	height = modeInfo->Yres;
	screenBufferSize = width * height * (modeInfo->bpp / 8);
	screenBuffer = kmalloc(screenBufferSize);
	vbe_putRect(0, 0, width, height, &(vbe_color_t){.a = 255, .r = 0x23, .g = 0x23, .b = 0x23});
	for (uint32_t x = 0; x < cool_width*4; x++)
		for (uint32_t y = 0; y < cool_height*4; y++)
			vbe_putPixel(x+0, y+0, &(vbe_color_t){.a = 255,
						.r = cool_data[(x/4+(y/4*cool_width))*3+0],
						.g = cool_data[(x/4+(y/4*cool_width))*3+1],
						.b = cool_data[(x/4+(y/4*cool_width))*3+2]});
	memcpy((void *)modeInfo->physbase, screenBuffer, screenBufferSize);

	textmode_resize(width/font.Width, height/font.Height);

	createLookupTable();

	screenRefreshTask = task_create(&screenRefresh, NULL, stack + (0x1000/sizeof(uint32_t)));
	task_start(screenRefreshTask);
}

static inline const uint8_t * getFontPos(uint16_t c) {
	const uint16_t * arr = font.Index;
	for (uint16_t i = 0; i < font.Chars; i++) {
		if (arr[i] == c)
			return &font.Bitmap[i * font.Height];
		else if (arr[i] > c) //Because the list is sorted by size
			break;
	}
	return &font.Bitmap[0];
}

static void createLookupTable() {
	for (uint32_t i = 0; i <= UINT16_MAX; i++)
		charLookup[i] = getFontPos((uint16_t)i);
}

static int screenRefresh(UNUSED void * arg) {
	while(true) {
		__asm__ volatile("cli");
		paintBackground();
		memcpy((void *)modeInfo->physbase, screenBuffer, screenBufferSize);
		__asm__ volatile("sti");
		task_next();
	}
	return 0;
}

void vbe_putPixel(int x, int y, vbe_color_t * color) {
	if (x < 0 || x > width ||
			y < 0 || y > height ||
		  !color->a)
		return;
	if (x)
		x = (x * (modeInfo->bpp>>3));
	if (y)
		y = (y * modeInfo->pitch);

	register uint8_t * pixel = &screenBuffer[x+y];

	uint8_t tmp;
	tmp = pixel[0];
	if (color->a < 255)
		tmp = (uint8_t)((tmp*(255.0-color->a)/255.0) +
										(color->b * (255.0/(float)color->a)));
	else
		tmp = color->b;
	pixel[0] = tmp;

	tmp = pixel[1];
	if (color->a < 255)
		tmp = (uint8_t)((tmp*(255.0-color->a)/255.0) +
										(color->g * (255.0/(float)color->a)));
	else
		tmp = color->g;
	pixel[1] = tmp;

	tmp = pixel[2];
	if (color->a < 255)
		tmp = (uint8_t)((tmp*(255.0-color->a)/255.0) +
										(color->r * (255.0/(float)color->a)));
	else
		tmp = color->r;
	pixel[2] = tmp;
}

void vbe_putRect(int x, int y, int width, int height,  vbe_color_t * color) {
	for (int xx = x; xx < x+width; xx++)
		for (int yy = y; yy < y+height; yy++)
			vbe_putPixel(xx, yy, color);
}

void vbe_putLine(int x0, int y0, int x1, int y1,  vbe_color_t * color) {
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
			vbe_putPixel(y, x, color);
		else
			vbe_putPixel(x, y, color);
		
		if ((e + dy) << 1 < dx)
			e += dy;
		else {
			y += inc;
			e += dy - dx;
		}
	}
}

void vbe_putCircle(int x0, int y0, int radius, vbe_color_t * color) {
  //Midpoint circle algorithm
  int x = radius;
  int y = 0;
  int radiusError = 1 - x;
 
  while(x >= y) {
    vbe_putPixel( x + x0,  y + y0, color);
    vbe_putPixel( y + x0,  x + y0, color);
    vbe_putPixel(-x + x0,  y + y0, color);
    vbe_putPixel(-y + x0,  x + y0, color);
    vbe_putPixel(-x + x0, -y + y0, color);
    vbe_putPixel(-y + x0, -x + y0, color);
    vbe_putPixel( x + x0, -y + y0, color);
    vbe_putPixel( y + x0, -x + y0, color);
    y++;
    if (radiusError < 0)
      radiusError += 2 * y + 1;
    else {
      x--;
      radiusError += 2 * (y - x) + 1;
    }
  }
}

void vbe_print(char * str, int x, int y, vbe_color_t * fgcolor, vbe_color_t * bgcolor) {
	for (int i = 0; i < strlen(str); i++)
		vbe_drawChar(x + font.Width*i, y, str[i], fgcolor, bgcolor);
}

void fromTBColor(uint8_t color, vbe_color_t * fgcolor, vbe_color_t * bgcolor) {
	uint8_t fg = color & 0xF;
	uint8_t bg = (color >> 4) & 0xF;
	switch (fg) {
	case COLOR_BLACK:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0x00, .b = 0x00};
		break;
	case COLOR_BLUE:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0x00, .b = 0xC0};
		break;
	case COLOR_GREEN:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xC0, .b = 0x00};
		break;
	case COLOR_CYAN:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xC0, .b = 0xC0};
		break;
	case COLOR_RED:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0x00, .b = 0x00};
		break;
	case COLOR_MAGENTA:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0x00, .b = 0xC0};
		break;
	case COLOR_BROWN:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0x80, .b = 0x00};
		break;
	case COLOR_LIGHT_GREY:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0xC0, .b = 0xC0};
		break;
	case COLOR_DARK_GREY:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x80, .g = 0x80, .b = 0x80};
		break;
	case COLOR_LIGHT_BLUE:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0x00, .b = 0xFF};
		break;
	case COLOR_LIGHT_GREEN:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xFF, .b = 0x00};
		break;
	case COLOR_LIGHT_CYAN:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xF, .b = 0xFF};
		break;
	case COLOR_LIGHT_RED:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0x00, .b = 0x00};
		break;
	case COLOR_LIGHT_MAGENTA:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0x00, .b = 0xFF};
		break;
	case COLOR_YELLOW:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0xFF, .b = 0x00};
		break;
	default:
	case COLOR_WHITE:
		*fgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0xFF, .b = 0xFF};
		break;
	}

	switch (bg) {
	case COLOR_BLACK:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0x00, .b = 0x00};
		break;
	case COLOR_BLUE:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0x00, .b = 0xC0};
		break;
	case COLOR_GREEN:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xC0, .b = 0x00};
		break;
	case COLOR_CYAN:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xC0, .b = 0xC0};
		break;
	case COLOR_RED:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0x00, .b = 0x00};
		break;
	case COLOR_MAGENTA:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0x00, .b = 0xC0};
		break;
	case COLOR_BROWN:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0x80, .b = 0x00};
		break;
	case COLOR_LIGHT_GREY:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xC0, .g = 0xC0, .b = 0xC0};
		break;
	case COLOR_DARK_GREY:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x80, .g = 0x80, .b = 0x80};
		break;
	case COLOR_LIGHT_BLUE:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0x00, .b = 0xFF};
		break;
	case COLOR_LIGHT_GREEN:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xFF, .b = 0x00};
		break;
	case COLOR_LIGHT_CYAN:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0x00, .g = 0xF, .b = 0xFF};
		break;
	case COLOR_LIGHT_RED:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0x00, .b = 0x00};
		break;
	case COLOR_LIGHT_MAGENTA:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0x00, .b = 0xFF};
		break;
	case COLOR_YELLOW:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0xFF, .b = 0x00};
		break;
	default:
	case COLOR_WHITE:
		*bgcolor = (vbe_color_t){.a = 0xFF, .r = 0xFF, .g = 0xFF, .b = 0xFF};
		break;
	}
}

static void paintBackground() {
	vbe_putRect(0, 0, width, height, &(vbe_color_t){.a = 255, .r = 0, .g = 0, .b = 0});

	vbe_color_t fg, bg;

	for (int x = 0; x < currentTTY->width; x++) {
		for (int y = 0; y < currentTTY->height; y++) {
			int pos = (y*currentTTY->width)+x;
			char text = currentTTY->text[pos];
			uint8_t color = currentTTY->color[pos];
			if (!text)
				break;
			fromTBColor(color, &fg, &bg);
			vbe_drawChar(font.Width*x, font.Height*y, text, &fg, &bg);
		}
	}
}


void vbe_drawChar(int x_, int y_, uint16_t c, vbe_color_t * fgcolor, vbe_color_t * bgcolor) {
	vbe_putRect(x_, y_, font.Width, font.Height, bgcolor);
	const uint8_t * lines = charLookup[c];
	for (int y = 0; y < font.Height; y++) {
		uint8_t line = lines[y];
		for (int x = 0; x < font.Width; x++) {
			if (line & (1 << x))
				vbe_putPixel(x_ + (font.Width - x), y_ + y, fgcolor);
		}
	}
}
