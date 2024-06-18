/**
 * RecycleObjectStore stores a dynamic amount of object that can be aquired and released again for reuse.
 * This is much more starter instead of creating and deleting objects all the time if the objects would 
 * be stored on the heap otherwise.
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

    ~RecycleObjectStoreQueue(){
        while(!available.empty()){
            delete available.front();
            available.pop();
        }
    }


    inline T* aquire(){
        if(available.empty()){
            return new T();
        }
        T *obj = available.front();
        available.pop();
        return obj;
    }
    

    inline void release(T* obj){
        available.push(obj);   
    }

    std::string toString() const {
        return "RecycleObjectStoreQueue(available="+std::to_string(available.size())+")";
    }
};


}
#endif // SPI_RECYCLE_OBJECT_STORE_QUEUE_HPP