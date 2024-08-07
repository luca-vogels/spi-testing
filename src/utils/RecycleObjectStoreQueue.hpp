/**
 * RecycleObjectStore stores a dynamic amount of objects that can be acquired and released again for reuse.
 * This is much faster than creating and deleting objects all the time.
 * 
 * IMPORTANT: only use if you cannot store objects on the heap.
 * 
 * @file RecycleObjectStore.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */
#ifndef SPI_RECYCLE_OBJECT_STORE_QUEUE_HPP
#define SPI_RECYCLE_OBJECT_STORE_QUEUE_HPP

#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <vector>

namespace spi {


template<typename T>
class RecycleObjectStoreQueue {
protected:
    std::queue<T*> available;

public:
    RecycleObjectStoreQueue() = default;

    ~RecycleObjectStoreQueue() noexcept {
        while(!available.empty()){
            delete available.front();
            available.pop();
        }
    }


    inline T* acquire() noexcept {
        if(available.empty()){
            return new T();
        }
        T *obj = available.front();
        available.pop();
        return obj;
    }
    

    inline void release(T* obj) noexcept {
        available.push(obj);   
    }

    std::string toString() const {
        return "RecycleObjectStoreQueue(available="+std::to_string(available.size())+")";
    }
};


}
#endif // SPI_RECYCLE_OBJECT_STORE_QUEUE_HPP