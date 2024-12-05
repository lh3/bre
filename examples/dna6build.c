#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bre.h"

const uint8_t rb3_nt6_table[128] = {
    0, 1, 2, 3,  4, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 1, 5, 2,  5, 5, 5, 3,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  4, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 1, 5, 2,  5, 5, 5, 3,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  4, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5
};

int str2bwt(char *s, int l) // this function works with ASCII strings but we use it for DNA6 only here
{
	int i, c = 0, k = 0;
	for(i = l - 1; i >= 0; --i) {
		int j, r = 0, a = s[i];
		memmove(&s[i], &s[i + 1], k);
		s[i + k] = a;
		for(j = i; j < i + k; ++j) r += (s[j] <= a);
		for(; j < l; ++j) r += (s[j] < a);
		k = r + 1, c = a;
	}
	return k; // sentinel
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "Usage: dna6build <in.txt> > out.bre\n");
		return 1;
	}

	FILE *fp;
	char *line = 0;
	size_t max = 0;
	ssize_t len;
	bre_file_t *f;
	bre_hdr_t hdr;

	fp = strcmp(argv[1], "-") == 0? stdin : fopen(argv[1], "rb");
	assert(fp);
	bre_hdr_init(&hdr, BRE_AT_DNA6, 1, 2);
	f = bre_open_write(0, &hdr); // to stdout
	if ((len = getline(&line, &max, fp)) > 0) { // only parse the first line
		int i, k;
		--len; // skip the trailing '\0'
		for (i = 0; i < len; ++i)
			line[i] = (uint8_t)line[i] < 128? rb3_nt6_table[(uint8_t)line[i]] : 5;
		k = str2bwt(line, len);
		for (i = 0; i < k; ++i) bre_write(f, line[i], 1);
		bre_write(f, 0, 1); // write the sentinel
		for (; i < len; ++i)    bre_write(f, line[i], 1);
	}
	bre_close(f);
	fclose(fp);
	return 0;
}
