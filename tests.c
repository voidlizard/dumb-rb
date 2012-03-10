#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "fsm.h"

#include "ringbuf.h"

void test_print_rw(ringbuffer_t *rb) {
    uint8_t *p  = rb->bs;
    uint8_t *be = rb->be;
    for(p = rb->bs; p < be; p++) {
        char c = ' ';
        if( p == rb->wp ) c = 'W';
        if( p == rb->rp ) c = 'R';
        if( p == rb->wp && p == rb->rp ) c = rb->written ? 'F' : 'E';
        putchar(c);
    }
    putchar('\n');
    for(p = rb->bs; p < be; p++) {
        char c = '-';
        putchar(c);
    }
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

int test_case_4() {
    ringbuffer_t *rb;
    static uint8_t databuf[(sizeof(ringbuffer_t) - 1 + 64)];
    static const uint8_t pattern[] = { '*', '@' };
    static uint8_t result[64 + 1] = { 0 };
    static const char expected[65] = "DEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmno0123456789*@*@*@*@*@";
    int i = 0;

    size_t ra0 = 0, ra1 = 0;
    size_t wa0 = 0, wa1 = 0;
    size_t read = 0;

    rb = ringbuffer_alloc(sizeof(databuf), databuf);

    for(i=0; i<64; i++) {
        rb->bs[i] = '0' + i;
    }

    printf("TEST CASE #4 :: NAME = STATE_1_READ_WRITE_UNDER\n");


    rb->wp += 10;
    rb->rp += 20;

    printf("\n");
    test_print_rw(rb);
    printf("\n");
    test_dump(rb->bs, rb->be, "%c");
    printf("\n");
    printf("\n");

    ra0 = ringbuffer_read_avail(rb);
    wa0 = ringbuffer_write_avail(rb);

    while( ringbuffer_write_avail(rb) ) {
        ringbuffer_write(rb, pattern, sizeof(pattern));
    }

    printf("\n");
    test_print_rw(rb);
    printf("\n");
    test_dump(rb->bs, rb->be, "%c");
    printf("\n");
    printf("\n");

    ra1 = ringbuffer_read_avail(rb);
    wa1 = ringbuffer_write_avail(rb);

    printf("TEST CASE #4 :: LOG = ra0: %d, wa0: %d\n", ra0, wa0);
    printf("TEST CASE #4 :: LOG = ra1: %d, wa1: %d\n", ra1, wa1);

    read = ringbuffer_read(rb, result, 64);

    printf("TEST CASE #4 :: LOG = read: %d, %s\n", read, result);


    if( !strncmp(result, expected, sizeof(expected)-1) 
        && ra0 == 54 
        && wa0 == 10 
        && ra1 == 64 
        && wa1 == 0 
        && read == ra1 ) {
 
        printf("TEST CASE #4 :: RESULT = PASS\n");
        return 0; 
    }

    printf("TEST CASE #4 :: RESULT = FAIL\n");
    return (-1);
}

int test_case_5() {
    ringbuffer_t *rb;
    static uint8_t databuf[(sizeof(ringbuffer_t) - 1 + 64)];
    static uint8_t pattern[32] = { 0 };
    static uint8_t result[64 + 1] = { 0 };
    static const char expected[65] = "DEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmno0123456789++++++++++";
    int i = 0;

    size_t ra0 = 0, ra1 = 0;
    size_t wa0 = 0, wa1 = 0;
    size_t read = 0, written = 0;

    rb = ringbuffer_alloc(sizeof(databuf), databuf);

    for(i=0; i<64; i++) {
        rb->bs[i] = '0' + i;
    }

    printf("TEST CASE #5 :: NAME = STATE_1_READ_WRITE_OVER\n");

    rb->wp += 10;
    rb->rp += 20;

    printf("\n");
    test_print_rw(rb);
    printf("\n");
    test_dump(rb->bs, rb->be, "%c");
    printf("\n");
    printf("\n");

    ra0 = ringbuffer_read_avail(rb);
    wa0 = ringbuffer_write_avail(rb);

    memset(pattern, '+', sizeof(pattern));
    test_dump(pattern, pattern + sizeof(pattern), "%c");
    printf("\n");
    written = ringbuffer_write(rb, pattern, sizeof(pattern));

    printf("TEST CASE #5 :: LOG = pattern: %d, written: %d\n", sizeof(pattern), written);

    printf("\n");
    test_print_rw(rb);
    printf("\n");
    test_dump(rb->bs, rb->be, "%c");
    printf("\n");
    printf("\n");

    ra1 = ringbuffer_read_avail(rb);
    wa1 = ringbuffer_write_avail(rb);

    printf("TEST CASE #5 :: LOG = ra0: %d, wa0: %d\n", ra0, wa0);
    printf("TEST CASE #5 :: LOG = ra1: %d, wa1: %d\n", ra1, wa1);

    read = ringbuffer_read(rb, result, ra1);
    printf("TEST CASE #5 :: LOG = read: %d, %s\n", read, result);

    if( !strncmp(result, expected, sizeof(expected)-1)  
        && ra0 == 54 
        && wa0 == 10 
        && wa0 == written
        && ra1 == read
      ) {

        printf("TEST CASE #5 :: RESULT = PASS\n");
        return 0; 
    }

    printf("TEST CASE #5 :: RESULT = FAIL\n");
    return (-1);
}


int test_case_6() {
    ringbuffer_t *rb;
    static uint8_t databuf[(sizeof(ringbuffer_t) - 1 + 64)];
    static const uint8_t pattern[] = { '+', '-' };
    static uint8_t result[64 + 1] = {0};
    static const char expected[65] = ";<=>?@ABCDEFGH+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-";
    int i = 0;

    size_t ra0 = 0, ra1 = 0, ra2 = 0;
    size_t wa0 = 0, wa1 = 0, wa2 = 0;
    size_t read = 0;

    rb = ringbuffer_alloc(sizeof(databuf), databuf);

    for(i=0; i<64; i++) {
        rb->bs[i] = '0' + i;
    }

    printf("TEST CASE #6 :: NAME = STATE_2_READ_WRITE_UNDER\n");

    rb->wp += 25;
    rb->rp += 11;

    printf("\n");
    test_print_rw(rb);
    printf("\n");
    test_dump(rb->bs, rb->be, "%c");
    printf("\n");
    printf("\n");

    ra0 = ringbuffer_read_avail(rb);
    wa0 = ringbuffer_write_avail(rb);

    while( ringbuffer_write_avail(rb) ) {
        ringbuffer_write(rb, pattern, sizeof(pattern));
    }

    printf("\n");
    test_print_rw(rb);
    printf("\n");
    test_dump(rb->bs, rb->be, "%c");
    printf("\n");
    printf("\n");

    ra1 = ringbuffer_read_avail(rb);
    wa1 = ringbuffer_write_avail(rb);

    printf("TEST CASE #6 :: LOG = ra0: %d, wa0: %d\n", ra0, wa0);
    printf("TEST CASE #6 :: LOG = ra1: %d, wa1: %d\n", ra1, wa1);

    read = ringbuffer_read(rb, result, 64);

    printf("\n");
    test_print_rw(rb);
    printf("\n");
    test_dump(rb->bs, rb->be, "%c");
    printf("\n");
    printf("\n");

    printf("TEST CASE #6 :: LOG = read: %d, %s\n", read, result);

    ra2 = ringbuffer_read_avail(rb);
    wa2 = ringbuffer_write_avail(rb);

    printf("TEST CASE #6 :: LOG = ra2: %d, wa2: %d\n", ra2, wa2);

    if( !strncmp(result, expected, sizeof(expected)-1) 
        && ra0 == 14 
        && wa0 == 50 
        && ra1 == 64 
        && wa1 == 0 
        && wa2 == 64
        && ra2 == 0 ) {

        printf("TEST CASE #6 :: RESULT = PASS\n");
        return 0; 
    }

    printf("TEST CASE #6 :: RESULT = FAIL\n");
    return (-1);
}

FSM_DECLARE(fsm_test_7, INIT)
    FSM_STATE_DECL(PRODUCE)
    FSM_STATE_DECL(CONSUME)
FSM_DECLARE_END(fsm_test_7)

#define TEST_CASE_7_ITERS    1
#define TEST_CASE_7_CHUNK   64
#define TEST_CASE_7_LEN     (TEST_CASE_7_ITERS*TEST_CASE_7_CHUNK)

int test_case_7() {
    int iterations = 200;
    ringbuffer_t *rb;
    static uint8_t databuf[RINGBUF_ALLOC_SIZE(TEST_CASE_7_CHUNK*2)];
    static uint8_t result1[TEST_CASE_7_LEN] = { 0 };
    static uint8_t result2[TEST_CASE_7_LEN] = { 0 };
    uint8_t *dst2 = &result2[0];
    uint8_t *dst1 = &result1[0];
    uint8_t *dst1_end = &result1[TEST_CASE_7_LEN];
    uint8_t *dst2_end = &result2[TEST_CASE_7_LEN];
    uint8_t tmp[TEST_CASE_7_CHUNK] = { 0 };
    static uint8_t chr = 'A';

    srand(time(0));

    printf("TEST CASE #7 :: NAME = RANDOM_RW_FSM\n");
 
    rb = ringbuffer_alloc(sizeof(databuf), databuf);

    for(;;) {

        if( dst1 >= dst1_end && dst2 >= dst2_end ) break;

        FSM_BEGIN(fsm_test_7, 1)

            FSM_STATE_BEGIN(INIT)
                chr = 'A';
            FSM_STATE_END(FSM_NEXT_STATE)

            FSM_STATE_BEGIN(PRODUCE)
                size_t len = rand() % TEST_CASE_7_CHUNK;
                size_t avail = ringbuffer_write_avail(rb);
                if( !avail || len > avail || !(len % 7) || dst2 >= dst2_end ) {
                    FSM_TRANS(FSM_NEXT_STATE);
                } else {
                    uint8_t *p = &tmp[0];
                    int i = 0;
                    printf("TEST CASE #7 :: LOG = PRODUCE len: %d \n", len);
                    for(i = 0; i < len; i++ ) {
                        *p++ = chr + (i % 26);
                    }
                    ringbuffer_write(rb, tmp, len);
/*                    test_dump(rb->bs, rb->be, "%c");*/
/*                    printf("\n");*/
                    memcpy(dst2, tmp, len);
                    dst2 += len;
                    FSM_TRANS(FSM_NEXT_STATE);
                }
            FSM_STATE_END(FSM_CURRENT_STATE)

            FSM_STATE_BEGIN(CONSUME)
                size_t avail = ringbuffer_read_avail(rb);
                size_t wtf = rand() % 100;
                size_t len = rand() % TEST_CASE_7_CHUNK;
                if( !avail || !(wtf % 7) || dst1 >= dst1_end ) {
                    FSM_TRANS(FSM_S(PRODUCE));
                } else {
                    size_t read = ringbuffer_read(rb, dst1, len);
                    if( read ) {
                        printf("TEST CASE #7 :: LOG = CONSUME len: %d, avail: %d, read: %d\n", len, avail, read);
                        dst1 += read;
                    } else {
                        FSM_TRANS(FSM_S(PRODUCE));
                    }
                }
            FSM_STATE_END(FSM_CURRENT_STATE)

        FSM_END(fsm_test_7)
    }

    if( dst1 < dst1_end && ringbuffer_read_avail(rb) ) {
        ringbuffer_read(rb, dst2, (size_t)(dst1_end - dst1));
    }
    
/*    test_dump(result1, dst1, "%c");*/
/*    printf("\n");*/
    test_dump(result2, dst2, "%c");
    printf("\n");

    printf("TEST CASE #7 :: RESULT = FAIL\n");
    return (-1);
}


int test_case_8() {
    ringbuffer_t *rb;
    const uint8_t data[46] = "QWERTYUIOP{}|ASDFGHJKL:'ZXCVBNM<>?0123456789~";
    uint8_t result[sizeof(data)] = { 0 };
    uint8_t databuf[RINGBUF_ALLOC_SIZE(46+9)] = { 0 };
    uint8_t written = 0;
    uint8_t read = 0;
    size_t datalen = sizeof(data) - 1;
    size_t ra0 = 0, wa0 = 0;
    size_t i = 1;
    size_t iter = 0;
    int res = 1;

    rb = ringbuffer_alloc(sizeof(databuf), databuf);

    printf("TEST CASE #8 :: NAME = CONSUME_TRIVIAL\n");
    printf("TEST CASE #8 :: LOG = datalen: %d, data: %s\n", datalen, data);

    for(iter = 0; iter < 10; iter++ ) {
        uint8_t *rp = &result[0];
        memset(result, '#', 45);

        written = ringbuffer_write(rb, data, datalen);

        while( ringbuffer_read_avail(rb) ) {
            ra0 = ringbuffer_read_avail(rb);
            wa0 = ringbuffer_write_avail(rb);
            printf("TEST CASE #8 :: LOG = ra0: %d, wa0: %d, i: %d \n", ra0, wa0, i);
            read = ringbuffer_read(rb, rp, i);
            i += 2;
            i %= 16;
            rp += read;
        }

        printf("\n");
        test_print_rw(rb);
        printf("\n");
        test_dump(result, rp, "%c");
        printf("\n");
        ra0 = ringbuffer_read_avail(rb);
        wa0 = ringbuffer_write_avail(rb);
        printf("TEST CASE #8 :: LOG = ra0: %d, wa0: %d, i: %d \n", ra0, wa0, i);

        res = res && !strncmp(result, data, datalen);
        if( !strncmp(result, data, datalen) ) {
            printf("TEST CASE #8 :: LOG = iter: %d, result: %s\n", iter, res?"OK":"FAIL");
        }

    }

    printf("TEST CASE #8 :: RESULT = FAIL\n");
    return (-1);
}



int main(void) {

    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    test_case_6();
/*    test_case_7();*/
    test_case_8();

    return 0;
}

