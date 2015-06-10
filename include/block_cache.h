#ifndef __BLOCK_CACHE_H_
#define __BLOCK_CACHE_H_

#include <mutex>
#include <atomic>
#include "buff_aggr.h"

// Lockless block cache using Buffer Aggregates

static const uint32_t blk_size = 10;
using CacheMap = std::map<uint32_t, AggrPtr>;
using CacheMapPtr = std::shared_ptr<CacheMap>;
using std::cout;
using std::endl;

class BlockCache {
    CacheMapPtr rmap_;
    std::mutex cache_mutex_;

    public:
        BlockCache() :
            rmap_(new CacheMap()) {
        }

        // read only operations
        bool exists(CacheMapPtr r, uint32_t id) {
            if ( r->find(id) == r->end() ) {
                return false;
            }
            return true;
        }

        CacheMapPtr get_cache_ptr() {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            CacheMapPtr temp = std::make_shared<CacheMap>(*rmap_);
            return temp;
        }

        void put_cache_ptr(CacheMapPtr in){
            std::lock_guard<std::mutex> lock(cache_mutex_);
            rmap_ = in;
        }

        bool get_block(uint32_t id, uint32_t off, BufPtr &res) {
            CacheMapPtr temp = this->get_cache_ptr();
            if ( this->exists(temp, id) == false ){
                return false;
            }
            res = (*temp)[id]->get_buf(off);
            if ( res == nullptr )
                return false;
            return true;
        }

        // read write operations
        void add_block(uint32_t id, uint32_t off, uint8_t *buf) {
            CacheMapPtr temp = this->get_cache_ptr();
            AggrPtr bag;
            if ( this->exists(temp, id) == true )
                bag = std::make_shared<BuffAggr>(*(*temp)[id]);
            else
                bag = std::make_shared<BuffAggr>(id);
            bag->set_buf(buf, off, blk_size);
            (*temp)[id] = bag;
            this->put_cache_ptr(temp);
        }

        void remove_block(uint32_t id, uint32_t off){
            CacheMapPtr temp = this->get_cache_ptr();
            if ( this->exists(temp, id) == false ) {
                return;
            }
            AggrPtr bag = std::make_shared<BuffAggr>(*(*temp)[id]);
            bag->remove_buf(off);
            (*temp)[id] = bag;
            this->put_cache_ptr(temp);
        }
};
#endif
