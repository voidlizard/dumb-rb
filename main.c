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

#define ringbuffer_shift_ptr(p, s, e, w) ((p) + (w) <= (e) ? (p) + (w) : (s) + ((w) - ((e)-(p))))

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

size_t ringbuffer_write_avail(ringbuffer_t *rb) {
    uint8_t *rp = rb->rp;
    uint8_t *wp = rb->wp;
    uint8_t *bs = rb->bs;
    uint8_t *be = rb->be;
    size_t  written = rb->written;

    if( wp < rp ) {
        return (size_t)(rp - wp);
    }

    if( wp > rp ) {
        return ((size_t)(be - wp) + (size_t)(rp - bs));
    }

    if( wp == rp && written == 0 ) {
        return ((size_t)(be - wp) + (size_t)(wp - bs));
    }

    if( wp == rp && written != 0 ) {
        return 0;
    }

    return 0;
}


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

size_t ringbuffer_read_avail(ringbuffer_t *rb) {
    uint8_t *rp = rb->rp;
    uint8_t *wp = rb->wp;
    uint8_t *bs = rb->bs;
    uint8_t *be = rb->be;
    size_t  written = rb->written;

    if( rp > wp ) {
        return ((size_t)(be - rp) + (size_t)(wp - bs));
    }

    if( rp < wp ) {
        return ((size_t)(wp - rp));
    }

    if( wp == rp && written == 0 ) {
        return 0;
    }

    if( wp == rp && written != 0 ) {
        return ((size_t)(be - rp) + (size_t)(bs - wp));
    }

    return 0;
}


size_t ringbuffer_read(ringbuffer_t *rb, void *dst, size_t size) {
}


int test_case_1() {
    ringbuffer_t *rb;
    static uint8_t databuf[(sizeof(ringbuffer_t) - 1 + 256)];
    printf("TEST CASE #1 :: NAME = Buffer init\n");
    rb = ringbuffer_alloc(sizeof(databuf), databuf);
    printf("TEST CASE #1 :: LOG = %d %d %d\n", rb->data_size, ringbuffer_read_avail(rb), ringbuffer_write_avail(rb));
    if( rb->data_size == 256 && ringbuffer_write_avail(rb) == 256 && ringbuffer_read_avail(rb) == 0 ) {
        printf("TEST CASE #1 :: RESULT = PASS\n");
        return 0;
    }
    printf("TEST CASE #1 :: RESULT = FAIL\n");
    return (-1);
}


int main(void) {

    test_case_1();

    return 0;
}


