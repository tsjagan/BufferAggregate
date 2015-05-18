#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <map>
#include <iostream>
#include <sstream>
#include <memory>
#include <assert.h>
#include <cstdint>
#include <iterator>
#include <algorithm>

#include "../buff_aggr.h"

uint8_t* create_buf(uint32_t size)
{
    uint8_t *buf = new uint8_t[size];
    for ( uint32_t i=0; i<size; i++) {
        char c = i + 65;
        memset(buf+i, c, 1);
    }
    return buf;
}

void test_buffer()
{
    // test Buffer class
    BufPtr bf_o;
    {
        uint8_t *buf = create_buf(9);
        BufPtr bf = std::make_shared<Buffer>(buf, 0, 10);
        bf_o = bf;
        std::cout << "Going out of scope" << std::endl;
    }
}

void test_additions(void)
{
    BuffAggr b1(5);
    // test for simple non-overlapping additions
    uint32_t off = 10;
    for ( uint32_t i=1; i<5; i++) {
        uint8_t *buf = create_buf(i * 3);
        b1.set_buf(buf, off, (i*3));
        off += i*3;
    }
}

void test_copy_constructor()
{
    BuffAggr b1(5);
    // test for simple non-overlapping additions
    uint32_t off = 10;
    for ( uint32_t i=1; i<5; i++) {
        uint8_t *buf = create_buf(i * 3);
        b1.set_buf(buf, off, (i*3));
        off += i*3;
    }
    // test for copy constructor
    BuffAggr b2(b1);
    std::cout << "B1 State: " << b1.ToString() << std::endl;
    std::cout << "B2 State: " << b2.ToString() << std::endl;

    // assignment operator test
    BuffAggr b3 = b1;
    std::cout << "B1 State: " << b1.ToString() << std::endl;
    std::cout << "B3 State: " << b3.ToString() << std::endl;
}

void test_iteration()
{
    // test for iteration and printing
    BuffAggr b3(6);
    uint32_t off = 0;
    for ( uint32_t i=1; i<3; i++) {
        uint8_t *buf = create_buf(i+1);
        b3.set_buf(buf, off, i+1);
        off += i+1;
    }
    std::cout << "B3 State: " << b3.ToString() << std::endl;
    BuffAggrIterator it = b3.begin();
    for( ; it != b3.end(); it++ ) {
        std::cout << *it << "..";
    }
    std::cout << std::endl;

    // test for iteration with [] operator
    it = b3[3];
    for( ; it != b3.end(); it++ ) {
        std::cout << *it << "..";
    }
    std::cout << std::endl;
    return;
}

void test_rw_aggr_buf()
{
    BuffAggr b3(6, false);
    uint32_t off = 0;
    for ( uint32_t i=1; i<3; i++) {
        uint8_t *buf = create_buf(i+4);
        b3.set_buf(buf, off, i+4);
        off += i+4;
    }
    b3.print();
    uint8_t *buf = create_buf(2);
    b3.set_buf(buf, 2, 2);
    b3.print();
}

void test_non_contiguous()
{
    BuffAggr b3(6, false);
    uint32_t off = 0;
    for ( uint32_t i=0; i<7; i++) {
        uint8_t *buf = create_buf(i+4);
        b3.set_buf(buf, off, i+4);
        off += i + 4 + 10;
    }
    b3.print();
}

void unit_tests()
{
    test_buffer();
    test_additions();
    test_copy_constructor();
    test_iteration();
    test_rw_aggr_buf();
    test_non_contiguous();
}

int main(void)
{
    unit_tests();
    return 0;
}
