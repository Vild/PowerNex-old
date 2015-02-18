#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <powernex/fs/initrd.h>
#include <dirent.h>
#include <getopt.h>
#include <stdlib.h>

char * output = "initrd.img";
char * input = "initrd";
struct {
	char name[128];
	char path[128];
} input_files[64];
int file_index = 0;

static __attribute__((noreturn)) void showHelp() {
	printf("mkinitrd [-h] [-o initrd.img] [-i initrd]");
	printf("\t -h / --help          - Shows this help");
	printf("\t -o / --output <FILE> - The output file for the initrd, default 'initrd.img'");
	printf("\t -i / --input  <DIR>  - The input directory that will be used for the initrd, default: 'initrd'");
	exit(0);
}

static void handleArgs(int argc, char ** argv) {
	int c;
	static struct option long_options[] = {
		{"output", 1, 0, 'o'},
		{"input", 1, 0, 'i'},
		{"help", 0, 0, 'h'},
		{NULL, 0, NULL, 0}
	};
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "o:i:h", long_options, &option_index)) != -1) {
		switch (c) {
		case 'o':
			output = optarg;
			break;
		case 'i':
			input = optarg;
			break;
		case 'h':
			showHelp();
			break;
		default:
			printf("?? getopt returned character code 0%o ??\n", c);
			exit(0);
		}
	}
	if (optind < argc)
		showHelp();
}



static int one (UNUSED const struct dirent *unused) {
  return 1;
}

static void findFiles(char * dir, char * dirname) {
	char buf[128];
  struct dirent **eps;
  int n;

  n = scandir(dir, &eps, one, alphasort);
  if (n >= 0) {
		for (int i = 0; i < n; ++i) {
			if (!strcmp(eps[i]->d_name, ".") || !strcmp(eps[i]->d_name, ".."))
				continue;
			snprintf(buf, 128, "%s%s", dirname, eps[i]->d_name);
			if (eps[i]->d_type == DT_REG) {
				strncpy(input_files[file_index].name, eps[i]->d_name, 128);

				snprintf(input_files[file_index].path, 128, "%s/%s", input, buf);
				file_index++;
			} else if (eps[i]->d_type == DT_DIR) {
				printf("D: %s SKIPPING!\n", buf);
				//handleDir(dir, esp[i]->d_name);
			}
		}
  } else
    perror("Couldn't open the directory");
}

int main(int argc, char ** argv) {
	handleArgs(argc, argv);

	if (input == NULL || output == NULL)
		showHelp();

	findFiles(input, "");
	
	initrd_header_t header;
	header.count = file_index;

	initrd_fileHeader_t fileHeader[64];
	memset(fileHeader, 0, sizeof(initrd_fileHeader_t)*64);

	uint32_t off = sizeof(initrd_header_t) + sizeof(initrd_fileHeader_t) * file_index;
	
	for (int i = 0; i < file_index; i++) {
		printf("Writing file: %s->%s at 0x%x\n", input_files[i].path, input_files[i].name, off);
		
		fileHeader[i].magic = 0xBF;
		strncpy(fileHeader[i].name, input_files[i].name, 128);
		fileHeader[i].offset = off;
		FILE * tfp = fopen(input_files[i].path, "rb");
		fseek(tfp, 0, SEEK_END);
		fileHeader[i].length = ftell(tfp);
		off += fileHeader[i].length;
		fclose(tfp);
	}

	FILE * fp = fopen(output, "wb");

	fwrite(&header, sizeof(initrd_header_t), 1, fp);
	fwrite(&fileHeader, sizeof(initrd_fileHeader_t), file_index, fp);

	for (int i = 0; i < file_index; i++) {
		FILE * tfp = fopen(input_files[i].path, "rb");
		uint8_t * buf = malloc(fileHeader[i].length);

		fread(buf, 1, fileHeader[i].length, tfp);
		fwrite(buf, 1, fileHeader[i].length, fp);
		fclose(tfp);
		
		free(buf);
	}	

	fclose(fp);
	
	return 0;
}
