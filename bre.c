#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "bre.h"

#define Calloc(type, cnt)       ((type*)calloc((cnt), sizeof(type)))
#define Realloc(type, ptr, cnt) ((type*)realloc((ptr), (cnt) * sizeof(type)))

#define BRE_HDR_SIZE 24

struct bre_file_s {
	// these members are not modified by bre_read/bre_write()
	bre_hdr_t hdr;     // BRE header
	int32_t is_write;  // if the file is opened for write
	// modified by bre_read/bre_write()
	FILE *fp;          // actual file handler
	int64_t c;         // buffered symbol
	int64_t l;         // buffered run length
	int64_t n_rec;     // number of records read or written so far
};

/**********
 * Writer *
 **********/

static int bre_hdr_write(FILE *fp, const bre_hdr_t *hdr)
{
	size_t sz = 0;
	sz += fwrite("BRE\1", 1, 4, fp);
	sz += fwrite(&hdr->b_per_sym, 1, 1, fp);
	sz += fwrite(&hdr->b_per_run, 1, 1, fp);
	sz += fwrite(&hdr->atype, 1, 1, fp);
	sz += fwrite(&hdr->mtype, 1, 1, fp);
	sz += fwrite(&hdr->asize, 8, 1, fp) * 8;
	sz += fwrite(&hdr->l_aux, 8, 1, fp) * 8;
	if (sz != BRE_HDR_SIZE) return -1;
	if (hdr->l_aux > 0) {
		assert(hdr->aux);
		sz = fwrite(hdr->aux, 1, hdr->l_aux, fp); // TODO: test return code
	}
	return 0;
}

static inline void bre_write_uint(FILE *fp, uint8_t n, uint64_t x)
{
	uint8_t i, k, shift = 0;
	for (i = k = 0; i < n; ++i, shift += 8) {
		uint8_t y = (x >> shift) & 0xff;
		k += fwrite(&y, 1, 1, fp);
	}
}

int bre_write(bre_file_t *f, int64_t c, int64_t l)
{
	if (f == 0 || !f->is_write) return -1;
	if (f->c == c) {
		f->l += l;
	} else {
		int64_t rest = f->l, max = (1LL<<f->hdr.b_per_run*8) - 1;
		while (rest > 0) {
			int64_t len = rest <= max? rest : max;
			bre_write_uint(f->fp, f->hdr.b_per_sym, f->c);
			bre_write_uint(f->fp, f->hdr.b_per_run, len);
			f->n_rec++;
			rest -= len;
		}
		f->c = c, f->l = l;
	}
	return 0;
}

bre_file_t *bre_open_write(const char *fn, const bre_hdr_t *hdr)
{
	FILE *fp;
	bre_file_t *f;
	fp = fn && strcmp(fn, "-")? fopen(fn, "wb") : stdout;
	if (fp == 0) return 0; // fail to open the file
	f = Calloc(bre_file_t, sizeof(*f));
	f->c = -1, f->fp = fp, f->is_write = 1;
	memcpy(&f->hdr, hdr, sizeof(*hdr));
	if (bre_hdr_write(fp, &f->hdr) < 0) {
		if (fp != stdout) fclose(fp);
		free(f);
		return 0;
	}
	return f;
}

/**********
 * Reader *
 **********/

static int bre_hdr_read(FILE *fp, bre_hdr_t *hdr)
{
	size_t sz;
	char magic[4];
	memset(hdr, 0, sizeof(*hdr));
	sz = fread(magic, 1, 4, fp);
	if (sz < 4 && strncmp(magic, "BRE\1", 4) != 0) // wrong magic
		return -1;
	sz += fread(&hdr->b_per_sym, 1, 1, fp);
	sz += fread(&hdr->b_per_run, 1, 1, fp);
	sz += fread(&hdr->atype, 1, 1, fp);
	sz += fread(&hdr->mtype, 1, 1, fp);
	sz += fread(&hdr->asize, 8, 1, fp) * 8;
	sz += fread(&hdr->l_aux, 8, 1, fp) * 8;
	if (sz != BRE_HDR_SIZE) return -2;
	if (hdr->l_aux > 0) {
		hdr->aux = Calloc(uint8_t, hdr->l_aux);
		sz = fread(hdr->aux, 1, hdr->l_aux, fp); // TODO: test return code
	}
	return 0;
}

static inline int64_t bre_read_uint(FILE *fp, uint8_t n) // NB: read up to 63 bits
{
	uint8_t i, y, k, shift = 0;
	uint64_t x = 0;
	assert(n <= 8);
	for (i = k = 0; i < n; ++i, shift += 8) {
		k += fread(&y, 1, 1, fp);
		x |= (uint64_t)y << shift;
	}
	return k == n? x : -1;
}

int64_t bre_read(bre_file_t *f, int64_t *b)
{
	int64_t ret = -1;
	if (f == 0 && f->is_write) return -1;
	while (1) {
		if (!feof(f->fp)) {
			int64_t c, l;
			c = bre_read_uint(f->fp, f->hdr.b_per_sym);
			if (c < 0) goto end_read;
			l = bre_read_uint(f->fp, f->hdr.b_per_run);
			if (c == 0 && l == 0) goto end_read;
			f->n_rec++;
			if (f->c < 0) {
				f->c = c, f->l = l;
			} else if (c == f->c) {
				f->l += l;
			} else {
				*b = f->c, ret = f->l;
				f->c = c, f->l = l;
				break;
			}
		} else {
end_read:	*b = f->c, ret = f->l;
			f->c = -1, f->l = 0;
			break;
		}
	}
	return ret;
}

bre_file_t *bre_open_read(const char *fn)
{
	FILE *fp;
	bre_file_t *f;
	fp = fn && strcmp(fn, "-")? fopen(fn, "rb") : stdin;
	if (fp == 0) return 0; // fail to open the file
	f = Calloc(bre_file_t, sizeof(*f));
	f->c = -1, f->fp = fp, f->is_write = 0;
	if (bre_hdr_read(fp, &f->hdr) < 0) {
		if (fp != stdin) fclose(fp);
		free(f);
		return 0;
	}
	return f;
}

/*****************
 * Miscellaneous *
 *****************/

void bre_close(bre_file_t *f)
{
	if (f == 0) return;
	if (f->is_write) {
		bre_write(f, -1, 0);
		bre_write_uint(f->fp, f->hdr.b_per_sym, 0);
		bre_write_uint(f->fp, f->hdr.b_per_run, 0);
	}
	if (f->hdr.l_aux > 0) free(f->hdr.aux);
	fclose(f->fp);
	free(f);
}

int bre_hdr_init(bre_hdr_t *h, bre_atype_t at, int32_t b_per_run)
{
	memset(h, 0, sizeof(*h));
	h->b_per_sym = 1, h->b_per_run = b_per_run, h->atype = at, h->asize = 256;
	if (at == BRE_AT_ASCII) h->asize = 128;
	else if (at == BRE_AT_DNA6) h->asize = 6;
	else if (at == BRE_AT_DNA16) h->asize = 16;
	return 0;
}
