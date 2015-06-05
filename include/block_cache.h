#ifndef __BLOCK_CACHE_H_
#define __BLOCK_CACHE_H_

#include <mutex>
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
            std::lock_guard<std::mutex> lock(cache_mutex_);
            AggrPtr bag;
            if ( this->exists(id) == true )
                bag = std::make_shared<BuffAggr>(*(*rmap_)[id]);
            else
                bag = std::make_shared<BuffAggr>(id);
            bag->set_buf(buf, off, blk_size);
            (*rmap_)[id] = bag;
        }

        bool get_block(uint32_t id, uint32_t off, BufPtr &res) {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            if ( this->exists(id) == false ){
                return false;
            }
            res = (*rmap_)[id]->get_buf(off);
            if ( res == nullptr )
                return false;
            return true;
        }
        void remove_block(uint32_t id, uint32_t off){
            std::lock_guard<std::mutex> lock(cache_mutex_);
            if ( this->exists(id) == false ) {
                return;
            }
            AggrPtr bag = std::make_shared<BuffAggr>(*(*rmap_)[id]);
            bag->remove_buf(off);
            (*rmap_)[id] = bag;
        }
};
#endif
