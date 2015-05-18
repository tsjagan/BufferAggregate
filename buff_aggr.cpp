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

/*
 * JAGAN:
 * Implementation of an aggregate buffer.
 * Mugen block_cache should use aggregate buffer.
 * As a consequence, other layers that interact with
 * the cache should also use this. We want to avoid all
 * memory copies.
 */

// when buffers go out of scope, they are freed automotically

// Forward declarations
class Buffer;
class BuffAggr;
class BuffAggrIterator;
using BufPtr = std::shared_ptr<Buffer>;
using BufMap = std::map<uint32_t, BufPtr>;
using AggrPtr = std::shared_ptr<BuffAggr>;


// A simple buffer class
class Buffer {
    friend BuffAggrIterator;
    uint8_t *bytes_;
    uint32_t off_;
    uint32_t size_;
    public:
        Buffer(uint8_t *b, uint32_t o, uint32_t l) :
                bytes_(b), off_(o), size_(l) { }

        ~Buffer() {
            //std::cout << "Deleting buffer at offset:" << off_
            //          << " and size " << size_ << std::endl;
            delete[] bytes_;
        }

        uint8_t* buf() {
            return bytes_;
        }

        uint32_t off() {
            return off_;
        }

        uint32_t size() {
            return size_;
        }
};

class BuffAggr {
    friend BuffAggrIterator;
    private:
    private:
        int fd_;
        BufMap bufs_;
        bool rdonly_;
    public:
        BuffAggr(int fd, bool rdonly=true) : fd_(fd), rdonly_(rdonly) { }
        ~BuffAggr() {
            bufs_.clear();
        }
        BuffAggrIterator begin();
        BuffAggrIterator end();
        BuffAggrIterator operator[](uint32_t idx);

        int get_fd() const { return fd_; }
        void set_fd(int fd) { fd_ = fd; }

        BufMap get_buffmap() const {
            return bufs_;
        }

        void set_buffmap(BufMap bufs) {
            bufs_ = bufs;
        }

        BuffAggr(const BuffAggr& other) {
            this->fd_ = other.get_fd();
            this->bufs_ = other.get_buffmap();
        }

        void set_buf(uint8_t *buf, uint32_t off, uint32_t size) {
            assert( size != 0 );
            assert( buf != NULL );
            uint32_t o = 0;
            bool exists = this->find_buf(off, o);
            if ( rdonly_ ) {
                assert( exists == false );
            }else {
                this->splice(off, size);
            }
            BufPtr b = std::make_shared<Buffer>(buf, off, size);
            bufs_[off] = b;
        }

        std::string ToString() {
            std::stringstream ss;
            for ( auto b : bufs_ ) {
                ss << "< ";
                ss << b.second->off() << ",";
                ss << b.second->size();
                ss << " >";
            }
            return ss.str();
        }

        void print();

        BuffAggr operator=(BuffAggr& other) {
            this->fd_ = other.get_fd();
            this->bufs_ = other.get_buffmap();
            return *this;
        }

        bool find_buf(uint32_t off, uint32_t &res) const {
            for ( auto b : bufs_ ) {
                uint32_t o = b.second->off();
                uint32_t s = b.second->size();
                if ( o <= off && off<o+s ) {
                    res = o;
                    return true;
                }
            }
            return false;
        }

        bool splice(uint32_t off, uint32_t size) {
            this->split(off);
            this->split(off + size);
            return true;
        }

        void split(uint32_t off) {
            uint32_t o = 0;
            if ( not this->find_buf(off, o) )
                return;
            if ( o == off )
                return;
            BufPtr b = bufs_[o];
            uint32_t s = b->size();

            uint8_t *p = new uint8_t[off-o];
            memcpy(p, b->buf(), off-o);
            BufPtr bnew = std::make_shared<Buffer>(p, o, off-o);
            bufs_[o] = bnew;

            p = new uint8_t[s-(off-o)];
            memcpy(p, b->buf()+((off-o)), s-(off-o));
            bnew = std::make_shared<Buffer>(p, off, s-(off-o));
            bufs_[off] = bnew;
         }
};

class BuffAggrIterator {
    private:
        uint32_t curr_off_;
        uint32_t curr_idx_;
        AggrPtr aggr_;
    public:
        BuffAggrIterator(AggrPtr aggr) :
                aggr_(aggr), curr_off_(-1), curr_idx_(0) {
        }
        BuffAggrIterator(AggrPtr aggr, uint32_t off) :
                aggr_(aggr), curr_off_(off), curr_idx_(0) {
            if ( curr_off_ != -1 ) {
                uint32_t o = 0;
                bool ret = aggr_->find_buf(curr_off_, o);
                assert( ret == true );
            }
        }

        bool valid() {
            uint32_t res = 0;
            bool ret = aggr_->find_buf(curr_off_, res);

            BufMap bufs = aggr_->get_buffmap();
            uint32_t b = (bufs.begin())->first;
            uint32_t e = (--bufs.end())->first;
            if ( res < b )
                return false;
            if ( res > e )
                return false;
            return true;
        }

        void operator++(int count) {
            curr_off_ += 1;
            uint32_t o = 0;
            bool ret = aggr_->find_buf(curr_off_, o);
            if ( ret == false ) {
                BufMap bufs = aggr_->get_buffmap();
                if ( curr_off_ > (--bufs.end())->first ) {
                    curr_off_ = (uint32_t)(-1);
                } else {
                    curr_idx_++;
                    BufMap::iterator it = bufs.begin();
                    std::advance(it, curr_idx_);
                    curr_off_ = it->first;
                }
            }
        }

        void operator--(int count) {
            curr_off_ -= 1;
        }

        bool operator!=(const BuffAggrIterator& other) {
            if ( other.curr_off_ == this->curr_off_ ) {
                return false;
            }
            return true;
        }

        BuffAggrIterator& operator=(BuffAggrIterator other) {
            this->aggr_ = other.aggr_;
            this->curr_off_ = other.curr_off_;
            return *this;
        }

        uint8_t operator*() {
            assert( this->valid() == true );
            uint32_t res = 0;
            bool ret = aggr_->find_buf(curr_off_, res);
            assert( ret == true );
            BufMap bufs = aggr_->get_buffmap();
            BufPtr buf = bufs[res];
            return buf->bytes_[ curr_off_- res ];
        }
};

BuffAggrIterator BuffAggr::begin()
{
    return BuffAggrIterator(std::make_shared<BuffAggr>(*this),
                            (bufs_.begin())->first);
}

BuffAggrIterator BuffAggr::end()
{
    return BuffAggrIterator(std::make_shared<BuffAggr>(*this),
                            (uint32_t)(-1));
}

BuffAggrIterator BuffAggr::operator[](uint32_t idx)
{
    return BuffAggrIterator(std::make_shared<BuffAggr>(*this),
                            idx);
}

void BuffAggr::print() {
    std::cout << this->ToString() << std::endl;
    BuffAggrIterator it = this->begin();
    for( ; it != this->end(); it++ ) {
        std::cout << *it << "..";
    }
    std::cout << std::endl;
};

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
