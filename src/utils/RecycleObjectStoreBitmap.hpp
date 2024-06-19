/**
 * RecycleObjectStore stores a dynamic amount of objects that can be acquired and released again for reuse.
 * This is much faster than creating and deleting objects all the time.
 * 
 * IMPORTANT: only use if you cannot store objects on the heap.
 * 
 * @file RecycleObjectStore.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */
#ifndef SPI_RECYCLE_OBJECT_STORE_BITMAP_HPP
#define SPI_RECYCLE_OBJECT_STORE_BITMAP_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace spi {


template<typename T>
class RecycleObjectStoreBitmap {
protected:
typedef uint64_t BitMapEntry;

    std::vector<T*> objects;
    std::vector<BitMapEntry> availability; // bitmap

    const BitMapEntry bitMapEntrySize = sizeof(BitMapEntry) * 8;

public:
    RecycleObjectStoreBitmap() = default;

    ~RecycleObjectStoreBitmap(){
        for(T *obj : objects){
            delete obj;
        }
    }


    inline T* acquire(size_t &index){
        for(size_t i=0; i < availability.size(); i++){
            if(availability[i] != 0){
                for(BitMapEntry b=0; b < bitMapEntrySize; b++){
                    if(availability[i] & (1 << b)){
                        availability[i] &= (BitMapEntry) ~((BitMapEntry)1 << b);
                        index = i * bitMapEntrySize + b;
                        return objects[index];
                    }
                }
            }
        }

        // not enough objects available, create a new one
        objects.push_back(new T());
        index = objects.size() - 1;
        size_t b = objects.size() % bitMapEntrySize;
        if(b == 1){
            availability.push_back(0);
        }
        return objects[index];
    }
    

    inline void release(size_t index){
        size_t i = index / bitMapEntrySize;
        size_t b = index % bitMapEntrySize;
        availability[i] |= (1 << b);
    }

    std::string toString(std::function<std::string(T*)> objToStr) const {
        std::string objStr = "";
        for(size_t i=0; i < objects.size(); i++){
            objStr += ", "+objToStr(objects[i]);
        }
        objStr = objStr.empty() ? objStr : objStr.substr(2);

        std::string availabilityStr = "";
        for(size_t i=0; i < availability.size(); i++){
            availabilityStr += ", ";
            for(BitMapEntry b=0; b < bitMapEntrySize; b++){
                availabilityStr += (availability[i] & (1 << b)) ? "1" : "0";
            }
        }
        availabilityStr = availabilityStr.empty() ? availabilityStr : availabilityStr.substr(2);

        return "RecycleObjectStoreBitmap(objects="+std::to_string(objects.size())+"["+objStr+
                "]; availability="+std::to_string(availability.size())+"["+availabilityStr+"])";
    }
};


}
#endif // SPI_RECYCLE_OBJECT_STORE_BITMAP_HPP