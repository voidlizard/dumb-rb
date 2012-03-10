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

/*    printf("DEBUG STATE %d\n", ringbuffer_get_state(rb));*/

    switch( ringbuffer_get_state(rb) ) {
        case RINGBUF_STATE_1:
            memcpy(wp, src, towrite);
            break;
        case RINGBUF_STATE_2:
        case RINGBUF_STATE_3: {
                size_t rest = (size_t)(be - wp);
                size_t w1 = towrite <= rest ? towrite : rest;
                size_t w2 = safe_sub(towrite, w1);
                size_t rest2 = (size_t)(rp - bs);
                size_t w3 = w2 <= rest2 ? w2 : rest2;
/*                printf("w1: %d, w2: %d, rest1: %d, rest2: %d\n", w1, w2, rest, rest2);*/
                if( w1 ) memcpy(wp, src, w1);
                if( w3 ) memcpy(bs, src + w1, w2);
            }
            break;
        default:
            towrite = 0;
            break;
    }

    rb->wp = ringbuffer_shift_ptr(wp, bs, be, towrite);
    rb->written += towrite;

/*    printf("DEBUG: %08X %d\n", rb->wp, rb->written);*/

    return towrite;
}


size_t ringbuffer_read(ringbuffer_t *rb, void *dst, size_t size) {
}


void test_dump(uint8_t *from, uint8_t *to, char *fmt) {
    uint8_t *bp = from;
    uint8_t *be = to;
    for(; bp < be; bp++ ) {
        printf(fmt, *bp);
    }
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

int test_case_3() {
    ringbuffer_t *rb;
    static uint8_t databuf[(sizeof(ringbuffer_t) - 1 + 64)];
    uint8_t pattern[6] = { '0', '1', '2', '3', '4', '5' };
    uint8_t result[64 + 1] = { 0 };
    int i = 0;
    const char expected[] = "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmno";
    size_t expectedLen = sizeof(expected) - 1;
    printf("TEST CASE #3 :: NAME = Loop write \n");
    rb = ringbuffer_alloc(sizeof(databuf), databuf);
    while( ringbuffer_write_avail(rb) ) {
        uint8_t *sp = pattern;
        uint8_t *pe = pattern + sizeof(pattern);
        for(; sp < pe; sp++) *sp += i;
        ringbuffer_write(rb, pattern, sizeof(pattern));
/*        test_dump(rb->rp, rb->wp, "%c");*/
/*        printf("\n");*/
        i = (pattern[sizeof(pattern)-1] + 1) - pattern[0];
    }

    memcpy(result, rb->rp, (size_t)(rb->be - rb->rp));
    printf("TEST CASE #3 :: LOG = %s \n", expected);
    printf("TEST CASE #3 :: LOG = %s \n", result);
    if( !strncmp(result, expected, expectedLen) ) {
        printf("TEST CASE #3 :: RESULT = PASS\n");
        return 0;
    }

    printf("TEST CASE #3 :: RESULT = FAIL\n");
    return (-1);
}


int main(void) {

    test_case_1();
    test_case_2();
    test_case_3();

    return 0;
}


