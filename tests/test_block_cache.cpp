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
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));
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
            if ( ret == true ) {
                cout << "Got block in file " << 0 << " at off " << j
                 << ": "<< std::hex << b->buf() << endl;
            } else {
                cout << "Not found block in file " << i << " at off " << j << endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));
        }
    }
}

void remove_blocks(BlockCache *cache)
{
    assert ( cache != NULL );
    for( uint32_t i = 0; i < 1000; i++ ) {
        for( uint32_t j = 0; j < 100; j++ ) {
            cache->remove_block(0, j*blk_size);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));
        }
    }
}

int main(void)
{
    BlockCache cache;
    std::srand(std::time(0));

    std::thread *ta = new std::thread(add_blocks, &cache);

    std::thread *tr = new std::thread(remove_blocks, &cache);

    std::thread *tg = new std::thread(get_blocks, &cache);

    ta->join();
    tr->join();
    tg->join();

    delete ta;
    delete tr;
    delete tg;
    return 0;
}
