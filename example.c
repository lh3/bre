#include <string.h>
#include <stdio.h>
#include "bre.h"

int main(int argc, char *argv[])
{
	const char *fn = "test.bre";
	bre_file_t *f;
	bre_hdr_t hdr;
	int64_t c, l;

	// writing
	bre_hdr_init(&hdr, BRE_AT_DNA6, 2);
	f = bre_open_write(fn, &hdr);
	bre_write(f, 4, 10);
	bre_write(f, 4, 100); // this will be combined with the previous run
	bre_write(f, 3, 70000); // this run will be split as it is longer than 65535
	bre_close(f);

	// reading
	f = bre_open_read(fn);
	while ((l = bre_read(f, &c)) > 0) // this will read two runs
		printf("%ld:%ld\n", (long)c, (long)l);
	bre_close(f);
	return 0;
}
