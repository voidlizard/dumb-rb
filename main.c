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
#define safe_sub(a, b) (a) >= ((b) ? ((a) - (b)) : 0)

typedef enum {
  RINGBUF_STATE_1 = 0      //  #   RA   [WP] WA  [RP]  RA   #
, RINGBUF_STATE_2          //  #   WA   [RP] RA  [WP]  WA   #
, RINGBUF_STATE_3          //  #   WA   [WP,RP]        WA   # | WRITTEN == 0
, RINGBUF_STATE_4          //  #   RA   [WP,RP]        RA   # | WRITTEN != 0
, RINGBUF_STATE_INVALID    // SHOULD NOT HAPPEN
} ringbuffer_state_t;

ringbuffer_state_t ringbuffer_get_state(ringbuffer_t *rb) {
    uint8_t *rp = rb->rp;
    uint8_t *wp = rb->wp;
    uint8_t *bs = rb->bs;
    uint8_t *be = rb->be;
    size_t  written = rb->written;
    ringbuffer_state_t state = RINGBUF_STATE_INVALID;
    if( wp < rp ) {
        state = RINGBUF_STATE_1;
    } else if( wp > rp ) {
        state = RINGBUF_STATE_2;
    } else if( wp == rp && !written ) {
        state = RINGBUF_STATE_3;
    } else if( wp == rp && written ) {
        state = RINGBUF_STATE_4;
    }
    return state;
}

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
    size_t  avail = 0;

    switch( ringbuffer_get_state(rb) ) {
        case RINGBUF_STATE_1:
            avail = (size_t)(rp - wp);
            break;
        case RINGBUF_STATE_2:
            avail = ((size_t)(be - wp) + (size_t)(rp - bs));
            break;
        case RINGBUF_STATE_3:
            avail = ((size_t)(be - wp) + (size_t)(wp - bs));
            break;
        case RINGBUF_STATE_4:
            avail = 0;
            break;
    }

    return avail;
}

size_t ringbuffer_read_avail(ringbuffer_t *rb) {
    uint8_t *rp = rb->rp;
    uint8_t *wp = rb->wp;
    uint8_t *bs = rb->bs;
    uint8_t *be = rb->be;
    size_t  avail = 0;

    switch( ringbuffer_get_state(rb) ) {
        case RINGBUF_STATE_1:
            avail = ((size_t)(be - rp) + (size_t)(wp - bs));
            break;
        case RINGBUF_STATE_2:
            avail = ((size_t)(wp - rp));
            break;
        case RINGBUF_STATE_3:
            avail = 0;
            break;
        case RINGBUF_STATE_4:
            avail = ((size_t)(be - rp) + (size_t)(bs - wp));
            break;
    }

    return avail;
}

size_t ringbuffer_write(ringbuffer_t *rb, void *src, size_t size) {
    uint8_t *rp = rb->rp;
    uint8_t *wp = rb->wp;
    uint8_t *bs = rb->bs;
    uint8_t *be = rb->be;
    size_t towrite = size;
    size_t avail = ringbuffer_write_avail(rb);
    
    towrite = towrite < avail ? towrite : avail;

    if( !avail || !towrite ) return 0;

    switch( ringbuffer_get_state(rb) ) {
        case RINGBUF_STATE_1:
            memcpy(wp, src, towrite);
            break;
        case RINGBUF_STATE_2:
        case RINGBUF_STATE_3: {
                size_t w1 = safe_sub(towrite, (size_t)(be - wp));
                size_t w2 = towrite - w1;
                memcpy(wp, src, w1);
                memcpy(bs, src + w1, w2);
                wp = bs + w2;
            }
            break;
        default:
            towrite = 0;
            break;
    }

    rb->wp = wp;
    rb->written += towrite;

    return towrite;

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

int test_case_2() {
    ringbuffer_t *rb;
    static uint8_t databuf[(sizeof(ringbuffer_t) - 1 + 256)];
    uint8_t pattern[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    uint8_t written = 0, ra = 0, wa = 0;
    printf("TEST CASE #2 :: NAME = Simple write \n");
    rb = ringbuffer_alloc(sizeof(databuf), databuf);
    written = ringbuffer_write(rb, pattern, sizeof(pattern));
    ra = ringbuffer_read_avail(rb);
    wa = ringbuffer_write_avail(rb);
    printf("TEST CASE #2 :: LOG = written: %d, wa: %d, ra: %d\n", written, wa, ra);
    if( written == sizeof(pattern) && ra == written && wa == (rb->data_size - sizeof(pattern)) ) {
        printf("TEST CASE #2 :: RESULT = PASS\n");
        return 0;
    }
    printf("TEST CASE #2 :: RESULT = FAIL\n");
    return (-1);
}


int main(void) {

    test_case_1();
    test_case_2();

    return 0;
}


