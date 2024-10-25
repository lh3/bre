#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "bre.h"

#define Calloc(type, cnt)       ((type*)calloc((cnt), sizeof(type)))
#define Realloc(type, ptr, cnt) ((type*)realloc((ptr), (cnt) * sizeof(type)))

#define BRE_HDR_SIZE 34

/**********
 * Writer *
 **********/

static int bre_hdr_write(FILE *fp, const bre_hdr_t *hdr)
{
	size_t sz = 0;
	sz += fwrite("BRE\1", 1, 4, fp);
	sz += fwrite(&hdr->b_per_sym, 1, 1, fp);
	sz += fwrite(&hdr->b_per_run, 1, 1, fp);
	sz += fwrite(&hdr->atype, 2, 1, fp) * 2;
	sz += fwrite(&hdr->asize, 8, 1, fp) * 8;
	sz += fwrite(&hdr->mtype, 2, 1, fp) * 2;
	sz += fwrite(&hdr->n_rec, 8, 1, fp) * 8;
	sz += fwrite(&hdr->l_aux, 8, 1, fp) * 8;
	if (sz != BRE_HDR_SIZE) return -1;
	if (hdr->l_aux > 0) {
		assert(hdr->aux);
		sz = fwrite(hdr->aux, 1, hdr->l_aux, fp); // TODO: test return code
	}
	return 0;
}

static inline void bre_write_int(FILE *fp, uint8_t n, uint64_t x)
{
	uint8_t i, k, shift = 0;
	for (i = k = 0; i < n; ++i, shift += 8) {
		uint8_t y = (x >> shift) & 0xff;
		k += fwrite(&y, 1, 1, fp);
	}
}

int bre_write(bre_file_t *f, int64_t c, int64_t l)
{
	FILE *fp = (FILE*)f->fp;
	if (!f->is_write) return -1;
	if (f->c == c) {
		f->l += l;
	} else {
		int64_t rest = f->l, max = (1LL<<f->hdr.b_per_run) - 1;
		while (rest > 0) {
			int64_t len = rest <= max? rest : max;
			bre_write_int(fp, f->hdr.b_per_sym, f->c);
			bre_write_int(fp, f->hdr.b_per_run, len);
			f->n_rec++;
			rest -= len;
		}
		f->c = c, f->l = l;
	}
	return 0;
}

/**********
 * Reader *
 **********/

static int bre_hdr_read(FILE *fp, bre_hdr_t *hdr)
{
	size_t sz = 0;
	char magic[4];
	sz += fread(magic, 1, 4, fp);
	memset(hdr, 0, sizeof(*hdr));
	if (sz < 4 && strncmp(magic, "BRE\1", 4) != 0) // wrong magic
		return -1;
	sz += fread(&hdr->b_per_sym, 1, 1, fp);
	sz += fread(&hdr->b_per_run, 1, 1, fp);
	sz += fread(&hdr->atype, 2, 1, fp) * 2;
	sz += fread(&hdr->asize, 8, 1, fp) * 8;
	sz += fread(&hdr->mtype, 2, 1, fp) * 2;
	sz += fread(&hdr->n_rec, 8, 1, fp) * 8;
	sz += fread(&hdr->l_aux, 8, 1, fp) * 8;
	if (sz != BRE_HDR_SIZE) return -2;
	if (hdr->l_aux > 0) {
		hdr->aux = Calloc(uint8_t, hdr->l_aux);
		sz = fread(hdr->aux, 1, hdr->l_aux, fp); // TODO: test return code
	}
	return 0;
}

static inline int64_t bre_read_int(FILE *fp, uint8_t n) // NB: this doesn't handle signs properly except for 64-bit integers
{
	uint8_t i, y, k, shift = 0;
	uint64_t x = 0;
	assert(n <= 8);
	for (i = k = 0; i < n; ++i, shift += 8) {
		k += fread(&y, 1, 1, fp);
		x |= (uint64_t)y << shift;
	}
	return (int64_t)x;
}

int64_t bre_read(bre_file_t *f, int64_t *b)
{
	FILE *fp = (FILE*)f->fp;
	int64_t ret = -1;
	if (f->is_write) return -1;
	while (1) {
		if (feof(fp) && (f->n_rec < 0 || f->n_rec < f->hdr.n_rec)) {
			int64_t c, l;
			c = bre_read_int(fp, f->hdr.b_per_sym);
			l = bre_read_int(fp, f->hdr.b_per_run);
			f->n_rec++;
			if (c == f->c) {
				f->l += l;
			} else {
				*b = f->c, ret = f->l;
				f->c = c, f->l = l;
				break;
			}
		} else {
			*b = f->c, ret = f->l;
			f->c = -1, f->l = 0;
			break;
		}
	}
	return ret;
}

/**************
 * Open/close *
 **************/

bre_file_t *bre_open(const char *fn, int32_t is_write)
{
	FILE *fp;
	int rc;
	bre_file_t *f;
	if (!is_write) {
		fp = fn && strcmp(fn, "-")? fopen(fn, "rb") : stdout;
	} else {
		fp = fn && strcmp(fn, "-")? fopen(fn, "wb") : stdin;
	}
	if (fp == 0) return 0; // fail to open the file
	f = Calloc(bre_file_t, sizeof(*f));
	f->c = -1;
	f->is_write = is_write;
	f->fp = fp;
	if (!is_write) {
		rc = bre_hdr_read(fp, &f->hdr);
	} else {
		rc = bre_hdr_write(fp, &f->hdr);
	}
	if (rc < 0) {
		if (fp != stdin && fp != stdout) fclose(fp);
		free(f);
		return 0;
	}
	return f;
}

void bre_close(bre_file_t *f)
{
	if (f == 0) return;
	if (f->is_write) bre_write(f, -1, 0);
	free(f->hdr.aux);
	free(f);
}
