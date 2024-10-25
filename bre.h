#ifndef BRE_H
#define BRE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t b_per_sym;
	uint8_t b_per_run;
	uint16_t atype;
	int64_t asize;
	uint16_t mtype;
	int64_t n_rec;
	int64_t l_aux;
	uint8_t *aux;
} bre_hdr_t;

typedef struct {
	bre_hdr_t hdr;
	int32_t is_write;
	// modified during reading/writing
	void *fp;
	int64_t c;
	int64_t l;
	int64_t n_rec;
} bre_file_t;

typedef enum { BRE_AT_UNKNOWN=0, BRE_AT_ASCII, BRE_AT_DNA6, BRE_AT_DNA16 } bre_atype_t;

bre_file_t *bre_open_read(const char *fn);
int64_t bre_read(bre_file_t *f, int64_t *c);
bre_file_t *bre_open_write(const char *fn, const bre_hdr_t *hdr);
int bre_write(bre_file_t *f, int64_t c, int64_t l);
void bre_close(bre_file_t *f);
int bre_hdr_init(bre_hdr_t *h, bre_atype_t at, int32_t b_per_run);

#ifdef __cplusplus
}
#endif

#endif
