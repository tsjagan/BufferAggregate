#include <thread>
#include "block_cache.h"

uint8_t* create_buf(uint32_t size)
{
    uint8_t *buf = new uint8_t[size];
    for ( uint32_t i=0; i<size; i++) {
        char c = i + 65;
        memset(buf+i, c, 1);
    }
    return buf;
}

void add_blocks(BlockCache *cache)
{
    assert ( cache != NULL );
    for( uint32_t i = 0; i < 1000; i++ ) {
        for( uint32_t j = 0; j < 100; j++ ) {
            uint8_t *buf = create_buf(blk_size);
            cache->add_block(0, j*blk_size, buf);
        }
    }
}

void get_blocks(BlockCache *cache)
{
    assert ( cache != NULL );
    for( uint32_t i = 0; i < 1000; i++ ) {
        for( uint32_t j = 0; j < 100; j++ ) {
            BufPtr b;
            bool ret = false;
            ret = cache->get_block(0, j*blk_size, b);
        }
    }
}

void remove_blocks(BlockCache *cache)
{
    assert ( cache != NULL );
    for( uint32_t i = 0; i < 1000; i++ ) {
        for( uint32_t j = 0; j < 100; j++ ) {
            cache->remove_block(0, j*blk_size);
        }
    }
}

int main(void)
{
    BlockCache cache;
    std::srand(std::time(0));

    std::thread *ta = new std::thread(add_blocks, &cache);

    std::thread *tg = new std::thread(get_blocks, &cache);

    std::thread *tr = new std::thread(remove_blocks, &cache);

    ta->join();
    tg->join();
    tr->join();

    delete ta;
    delete tg;
    delete tr;
    return 0;
}
