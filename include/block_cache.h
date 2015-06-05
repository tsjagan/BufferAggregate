#ifndef __BLOCK_CACHE_H_
#define __BLOCK_CACHE_H_

#include "buff_aggr.h"

// Lockless block cache using Buffer Aggregates

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
            if ( this->exists(id) == false ) {
                return;
            }
            AggrPtr bag = std::make_shared<BuffAggr>(*(*wmap)[id]);
            bag->remove_buf(off);
            (*wmap)[id] = bag;
            CacheMapPtr tmap = std::make_shared<CacheMap>(*wmap);
            rmap_ = tmap;
            delete wmap;
        }
};
#endif
