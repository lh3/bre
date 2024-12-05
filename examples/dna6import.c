#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bre.h"

#define BUF_SIZE 1024

const uint8_t nt6_table[128] = {
    0, 1, 2, 3,  4, 5, 5, 5,  5, 5, 0, 5,  5, 5, 5, 5, // "\n" (0x0a) is mapped to 0
    5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  0, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5, // $ mapped to 0
    5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 1, 5, 2,  5, 5, 5, 3,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  4, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 1, 5, 2,  5, 5, 5, 3,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  4, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5
};

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "Usage: dna6import <in.txt> > out.bre\n");
		return 1;
	}

	uint8_t buf[BUF_SIZE];
	FILE *fp;
	bre_file_t *f;
	bre_hdr_t hdr;
	int64_t i, k;

	fp = strcmp(argv[1], "-") == 0? stdin : fopen(argv[1], "rb");
	assert(fp);
	bre_hdr_init(&hdr, BRE_AT_DNA6, 1, 2);
	f = bre_open_write(0, &hdr); // to stdout
	while ((k = fread(buf, 1, BUF_SIZE, fp)) > 0)
		for (i = 0; i < k; ++i)
			bre_write(f, buf[i] < 128? nt6_table[buf[i]] : 5, 1);
	bre_close(f);
	fclose(fp);
	return 0;
}
