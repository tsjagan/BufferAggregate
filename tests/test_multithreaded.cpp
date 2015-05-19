#include "../buff_aggr.h"
#include <thread>
#include <mutex>

// Fixed size blocks <4096 bytes>
// No locks in read path

static const uint32_t blk_size = 10;
using CacheMap = std::map<uint32_t, AggrPtr>;
using CacheMapPtr = std::shared_ptr<CacheMap>;
using std::cout;
using std::endl;


class BlockCache {
    CacheMapPtr rmap_;
    CacheMapPtr wmap_;

    public:
        BlockCache() {
            rmap_ = std::make_shared<CacheMap>();
        }
        bool exists(uint32_t id) {
            if ( rmap_->find(id) == rmap_->end() ) {
                return false;
            }
            return true;
        }
        void add_block(uint32_t id, uint32_t off, uint8_t *buf) {
            CacheMap *wmap = new CacheMap(*rmap_);
            AggrPtr bag;
            if ( this->exists(id) == true )
                bag = std::make_shared<BuffAggr>(*(*wmap)[id]);
            else
                bag = std::make_shared<BuffAggr>(id);
            bag->set_buf(buf, off, blk_size);
            (*wmap)[id] = bag;
            CacheMapPtr tmap = std::make_shared<CacheMap>(*wmap);
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            rmap_ = tmap;
            delete wmap;
        }
        bool get_block(uint32_t id, uint32_t off, BufPtr &res) {
            if ( this->exists(id) == false ){
                return false;
            }
            res = (*rmap_)[id]->get_buf(off);
            if ( res == nullptr )
                return false;
            return true;
        }
        void remove_block(uint32_t id, uint32_t off){
            CacheMap *wmap = new CacheMap(*rmap_);
            assert ( this->exists(id) == true );
            AggrPtr bag = std::make_shared<BuffAggr>(*(*wmap)[id]);
            bag->remove_buf(off);
            (*wmap)[id] = bag;
            CacheMapPtr tmap = std::make_shared<CacheMap>(*wmap);
            rmap_ = tmap;
            delete wmap;
        }
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

void add_blocks(BlockCache *cache)
{
    assert ( cache != NULL );
    for( uint32_t i = 0; i < 10; i++ ) {
        for( uint32_t j = 0; j < 10; j++ ) {
            uint8_t *buf = create_buf(blk_size);
            cache->add_block(i, j*blk_size, buf);
        }
    }
}

void get_blocks(BlockCache *cache)
{
    assert ( cache != NULL );
    std::srand(std::time(0));
    for( uint32_t i = 0; i < 10; i++ ) {
        for( uint32_t j = 0; j < 10; j++ ) {
            BufPtr b;
            bool ret = false;
            ret = cache->get_block(i, j*blk_size, b);
            if ( ret == true ) {
                cout << "Got block in file " << i << " at off " << j
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
    for( uint32_t i = 0; i < 10; i++ ) {
        for( uint32_t j = 0; j < 5; j++ ) {
            cache->remove_block(i, j*blk_size);
        }
    }
}

int main(void)
{
    BlockCache cache;
    std::thread *ta = new std::thread(add_blocks, &cache);
    ta->join();


    std::thread *tr = new std::thread(remove_blocks, &cache);

    std::thread *tg = new std::thread(get_blocks, &cache);
    tr->join();
    tg->join();
    delete ta;
    delete tr;
    delete tg;
    return 0;
}
