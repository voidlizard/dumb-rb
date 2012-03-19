#ifndef __voidlizard_ringbuf_h
#define __voidlizard_ringbuf_h

#include "ringbuf_setup.h"
#include <stdint.h>

#define RINGBUF_AUTOCOMMIT 1

typedef struct ring_buffer_t_ {
	uint8_t flags;
    uint8_t *bs;
    uint8_t *be;
    uint8_t *rp;
    uint8_t *wp;
    size_t  written;
	uint8_t *twp;
	uint8_t twritten;
    size_t  data_size;
    uint8_t data[1];
} ringbuffer_t;

void ringbuffer_reset(ringbuffer_t *rb);
ringbuffer_t* ringbuffer_alloc(size_t data_size, uint8_t *data); 
size_t ringbuffer_write_avail(ringbuffer_t *rb);
size_t ringbuffer_read_avail(ringbuffer_t *rb);
size_t ringbuffer_write(ringbuffer_t *rb, const uint8_t *src, size_t size);
size_t ringbuffer_read(ringbuffer_t *rb, uint8_t *dst, size_t size);
void ringbuffer_commit(ringbuffer_t *rb);
void ringbuffer_rollback(ringbuffer_t *rb);
void ringbuffer_update_flags(ringbuffer_t *rb, uint8_t set, uint8_t flags);


#define RINGBUF_ALLOC_SIZE(n) (sizeof(ringbuffer_t) - 1 + (n))

#endif

