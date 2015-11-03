#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/* Windows 3.x bitmap file header */
typedef struct {
    char         filetype[2];   /* magic - always 'B' 'M' */
    unsigned int filesize;
    short        reserved1;
    short        reserved2;
    unsigned int dataoffset;    /* offset in bytes to actual bitmap data */
} __attribute__((packed)) file_header;

/* Windows 3.x bitmap full header, including file header */
typedef struct {
   file_header  fileheader;
    unsigned int headersize;
    int          width;
    int          height;
    short        planes;
    short        bitsperpixel;  /* we only support the value 24 here */
    unsigned int compression;   /* we do not support compression */
    unsigned int bitmapsize;
    int          horizontalres;
    int          verticalres;
    unsigned int numcolors;
    unsigned int importantcolors;
} __attribute__((packed)) bitmap_header;

typedef struct __attribute__((__packed__)) {
    unsigned char  b;
    unsigned char  g;
    unsigned char  r;
} __attribute__((packed)) data;

int main(int argc, char ** argv) {
  if (argc < 4) {
		printf("%s: <IN> <OUT> <NAMESPACE>\n", argv[0]);
		return 0;
	}
		
	FILE * in = fopen(argv[1], "rb");
	if (!in) {
		fprintf(stderr, "ERROR: Input file '%s' does not exist!\nErrno: %d (%s)\n", argv[1], errno, strerror(errno));
		return -1;
	}
	char * namespace = argv[3];
	bitmap_header bitmap;
	int tmp __attribute__((unused)) = fread(&bitmap, sizeof(bitmap_header), 1, in);
	if (bitmap.fileheader.filetype[0] != 'B' || bitmap.fileheader.filetype[1] != 'M') {
		fprintf(stderr, "ERROR: Not a BMP file!\n");
		fclose(in);
		return -1;
	}
	int width = bitmap.width;
	int height = bitmap.height;
	
	if (width * height > 1*1024*1024) {
		fprintf(stderr, "ERROR: Current size is %d. Max size is %d! Diff: %d\n", width*height, 1*1024*1024, width*height-1*1024*1024);
		fclose(in);
		return -1;
	}

	
	FILE * out = fopen(argv[2], "wb");
	if (!out) {
		fprintf(stderr, "ERROR: Output file '%s' can't be opened!\nErrno: %d (%s)\n", argv[2], errno, strerror(errno));
		fclose(in);
		return -1;
	}
	fprintf(out, "#ifndef %s_H_\n#define %s_H_\n\n", namespace, namespace);
	fprintf(out, "const unsigned int %s_width = %d;\n", namespace, width);
	fprintf(out, "const unsigned int %s_height = %d;\n", namespace, height);
	fprintf(out, "const unsigned char %s_data[%d*%d*3] = {", namespace, width, height);
	
	fseek(in, bitmap.fileheader.dataoffset, SEEK_SET);

	int pad = (width % 4);
	
	data * data = calloc(sizeof(data), width*height);
	for (int y = height-1; y >= 0; y--) {
		for (int x = 0; x < width; x++) {
			data[(y*width+x)].b = fgetc(in);
			data[(y*width+x)].g = fgetc(in);
			data[(y*width+x)].r = fgetc(in);
		}
		if (pad)
			for (int i = 0; i < 4-pad; i++)
			     (void)getc(in);
	}
	
	for (int i = 0; i < width*height; i++) {
		if (i % width == 0)
				fputs("\n\t", out);
		fprintf(out, "0x%X, 0x%X, 0x%X, ", data[i].r, data[i].g, data[i].b);
	}
	fprintf(out, "\n};\n\n#endif\n");
	free(data);
	
	fclose(out);
	fclose(in);
	return 0;
}
