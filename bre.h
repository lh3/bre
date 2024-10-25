#ifndef BRE_H
#define BRE_H

#include <stdint.h>

#ifndef __cplusplus
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
	int64_t c;
	int64_t l;
	int64_t n_rec;
} bre_itr_t;

typedef struct {
	bre_hdr_t hdr;
	bre_itr_t itr;
	void *fp;
	int32_t is_write;
} bre_file_t;

bre_file_t *bre_open(const char *fn, int32_t is_write);
void bre_close(bre_file_t *f);
int64_t bre_read(bre_file_t *f, int64_t *c);
int bre_write(bre_file_t *f, int64_t c, int64_t l);

#ifndef __cplusplus
}
#endif

#endif
