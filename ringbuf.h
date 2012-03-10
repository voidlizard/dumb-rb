#ifndef __voidlizard_ringbuf_h
#define __voidlizard_ringbuf_h

#include "ringbuf_setup.h"
#include <stdint.h>

typedef struct ring_buffer_t_ {
    uint8_t *bs;
    uint8_t *be;
    uint8_t *rp;
    uint8_t *wp;
    size_t  written;
    size_t  data_size;
    uint8_t data[1];
} ringbuffer_t;

void ringbuffer_reset(ringbuffer_t *rb);
ringbuffer_t* ringbuffer_alloc(size_t data_size, uint8_t *data); 
size_t ringbuffer_write_avail(ringbuffer_t *rb);
size_t ringbuffer_read_avail(ringbuffer_t *rb);
size_t ringbuffer_write(ringbuffer_t *rb, const void *src, size_t size);
size_t ringbuffer_read(ringbuffer_t *rb, void *dst, size_t size);

#define RINGBUF_ALLOC_SIZE(n) (sizeof(ringbuffer_t) - 1 + (n))

#endif

