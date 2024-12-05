#include <stdio.h>
#include <assert.h>
#include "bre.h"

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "Usage: dna6print <in.bre>\n");
		return 1;
	}
	bre_file_t *f = bre_open_read(argv[1]);
	assert(f);
	const bre_hdr_t *h = bre_get_hdr(f);
	assert(h->atype == BRE_AT_DNA6);
	int64_t i, c, l;
	while ((l = bre_read(f, &c)) > 0) {
		assert(c >= 0 && c < 6);
		for (i = 0; i < l; ++i)
			fputc("\nACGTX"[c], stdout);
	}
	bre_close(f);
	return 0;
}
