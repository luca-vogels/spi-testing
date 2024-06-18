/**
 * RecycleObjectStore stores a dynamic amount of object that can be aquired and released again for reuse.
 * This is much more starter instead of creating and deleting objects all the time if the objects would 
 * be stored on the heap otherwise.
 * 
 * @file RecycleObjectStore.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */
#ifndef SPI_RECYCLE_OBJECT_STORE_VECTOR_HPP
#define SPI_RECYCLE_OBJECT_STORE_VECTOR_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace spi {


template<typename T>
class RecycleObjectStoreVector {
protected:
    std::vector<T*> objects;
    std::vector<bool> availability;

public:
    RecycleObjectStoreVector() = default;

    ~RecycleObjectStoreVector(){
        for(T *obj : objects){
            delete obj;
        }
    }


    inline T* aquire(size_t &index){
        for(size_t i=0; i < availability.size(); i++){
            if(availability[i]){
                availability[i] = false;
                index = i;
                return objects[index];
            }
        }

        // not enough objects available, create a new one
        objects.push_back(new T());
        index = objects.size() - 1;
        availability.push_back(false);
        return objects[index];
    }
    

    inline void release(size_t index){
        availability[index] = true;
    }

    std::string toString(std::function<std::string(T*)> objToStr) const {
        std::string objStr = "";
        for(size_t i=0; i < objects.size(); i++){
            objStr += ", "+objToStr(objects[i]);
        }
        objStr = objStr.empty() ? objStr : objStr.substr(2);

        std::string availabilityStr = "";
        for(size_t i=0; i < availability.size(); i++){
            availabilityStr += std::to_string(availability[i]);
        }

        return "RecycleObjectStoreVector(objects="+std::to_string(objects.size())+"["+objStr+
                "]; availability="+std::to_string(availability.size())+"["+availabilityStr+"])";
    }
};


}
#endif // SPI_RECYCLE_OBJECT_STORE_VECTOR_HPP