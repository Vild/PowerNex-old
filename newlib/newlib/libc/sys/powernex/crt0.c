#include <fcntl.h>

extern void exit(int exitcode);
extern int main(int argv, char ** argv);

void _start() {
	return exit(main(0, NULL));
}
