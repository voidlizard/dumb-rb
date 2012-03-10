#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void ringbuffer_reset(ringbuffer_t *rb) {
    rb->bs = &rb->data[0];
    rb->be = &rb->data[rb->data_size];
    rb->rp = rb->bs;
    rb->wp = rb->bs;
    rb->written = 0;
}

ringbuffer_t* ringbuffer_alloc(size_t data_size, uint8_t *data) {
    if( data_size < sizeof(ringbuffer_t) ) {
        return (ringbuffer_t*)0;
    } else {
        ringbuffer_t *tmp = (ringbuffer_t*)data;
        tmp->data_size = data_size - sizeof(ringbuffer_t) + 1;
        ringbuffer_reset(tmp);
        return tmp;
    }
    return (ringbuffer_t*)0;
}

size_t ringbuffer_read_avail(ringbuffer_t *rb) {
    
    uint8_t *rp = rb->rp;
    uint8_t *wp = rb->wp;
    uint8_t *bs = rb->bs;
    uint8_t *be = rb->be;
    size_t  written = rb->written;

    if( rp < wp ) { // LESS
        return (size_t)(wp - rp);
    } else if( rp > wp  ) { // MORE
        return ((size_t)(be - rp) + (size_t)(wp - bs));
    }

    return 0;
}

size_t ringbuffer_write_avail(ringbuffer_t *rb) {
    
    uint8_t *rp = rb->rp;
    uint8_t *wp = rb->wp;
    uint8_t *bs = rb->bs;
    uint8_t *be = rb->be;
    size_t  written = rb->written;

    if( wp < rp ) { // LESS
        return (size_t)(rp - wp);
    } else if( rp > wp  ) { // MORE
        return ((size_t)(be - wp) + (size_t)(rp - bs));
    }

    return 0;
}

#define ringbuffer_shift_ptr(p, s, e, w) ((p) + (w) <= (e) ? (p) + (w) : (s) + ((w) - ((e)-(p))))

size_t ringbuffer_write(ringbuffer_t *rb, void *src, size_t size) {
    size_t towrite = size;
    size_t avail = ringbuffer_write_avail(rb);
    
    towrite = towrite < avail ? towrite : avail;

    if( !avail || !towrite ) return 0;
    else {
        uint8_t *rp = rb->rp;
        uint8_t *wp = rb->wp;
        uint8_t *bs = rb->bs;
        uint8_t *be = rb->be;
/*        size_t chunk1 = (size_t)(wp > rp ? (be - wp) : (rp - wp));*/
/*        size_t chunk2 = (size_t)((wp > rp) ? towrite - chunk1 : 0);*/
/*        size_t w1     = towrite <= chunk1 ? towrite : chunk1;*/
/*        size_t w2     = */
/*        */
/*        if( chunk ) memcpy(wp, src, w1);*/
/*        wp = ringbuffer_shift_ptr(wp, bs, be, chunk);*/
/*        if( rest )  memcpy(wp, src, rest);*/
/*        wp = ringbuffer_shift_ptr(wp, bs, be, chunk);*/

/*        rb->wp = wp;*/
/*        rb->written += towrite;*/

    }

    return towrite;

}

size_t ringbuffer_read(ringbuffer_t *rb, void *dst, size_t size) {
}

uint8_t databuf[(sizeof(ringbuffer_t) - 1 + 256)];

int main(void) {
    ringbuffer_t *rb;

    printf("%d %d\n", sizeof(ringbuffer_t), sizeof(databuf));
    rb = ringbuffer_alloc(sizeof(databuf), databuf);
    printf("RB (p: %08X) (size: %u)\n", rb, rb->data_size);

    // TEST
    printf("test1 --- (wa: %d) (ra: %d)\n", ringbuffer_write_avail(rb), ringbuffer_read_avail(rb));

    // TEST  ---
/*    while( ringbuffer_write_avail(rb) ) {*/
/*        uint8_t pattern[3] = { 0xFA, 0xFE, 0xAA };*/
/*        ringbuffer_write(rb, pattern, 3);*/
/*        printf("(wa: %d) (ra: %d)\n", ringbuffer_write_avail(rb), ringbuffer_read_avail(rb));*/
/*    }*/

    return 0;
}


