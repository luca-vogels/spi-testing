#include "./utils/RecycleObjectStoreBitmap.hpp"
#include "./utils/RecycleObjectStoreVector.hpp"

#include <iostream>
#include <queue>
#include <stdexcept>
#include <unordered_set>

using namespace spi;

struct TestStruct {
    size_t a;
    int b;
    int c;
};

int main(){
    std::unordered_set<TestStruct*> inUse;
    std::unordered_set<TestStruct*> seen;
    bool doesRecycle = false;

    RecycleObjectStoreVector<TestStruct> store;
    
    TestStruct *obj;
    std::queue<size_t> indices;
    std::queue<TestStruct*> objects;

    // aquire
    for(size_t i=0; i < 8; i++){
        size_t index;
        obj = store.aquire(index);
        obj->a = i;
        std::cout << (i+1) << ". aquired(" << index << "): " << store.toString([](TestStruct *obj){ return std::to_string(obj->a); }) << std::endl; // TODO REMOVE
        std::cout << " - " << obj << std::endl; // TODO REMOVE

        indices.push(index);
        objects.push(obj);
        if(inUse.find(obj) != inUse.end()){
            throw std::runtime_error("Object already in use");
        }

        if(seen.find(obj) != seen.end()){
            doesRecycle = true;
        }
        inUse.insert(obj);
        seen.insert(obj);
    }
    std::cout << std::endl;

    // release
    for(size_t i=0; i < 4; i++){
        size_t index = indices.front();
        store.release(index);
        std::cout << (i+1) << ". released(" << index << "): " << store.toString([](TestStruct *obj){ return std::to_string(obj->a); }) << std::endl; // TODO REMOVE

        obj = objects.front();
        std::cout << " - " << obj << std::endl; // TODO REMOVE
        if(obj->a != i) throw std::runtime_error("Object not as expected (got: "+std::to_string(obj->a)+"; expected: "+std::to_string(i)+")");
        inUse.erase(obj);
        indices.pop();
        objects.pop();
    }
    std::cout << std::endl;

    // aquire
    for(size_t i=0; i < 6; i++){
        size_t index;
        obj = store.aquire(index);
        std::cout << (i+1) << ". aquired(" << index << "): " << store.toString([](TestStruct *obj){ return std::to_string(obj->a); }) << std::endl; // TODO REMOVE
        std::cout << " - " << obj << std::endl; // TODO REMOVE

        if(i < 4){
            if(obj->a != i) throw std::runtime_error("Object not as expected (got: "+std::to_string(obj->a)+"; expected: "+std::to_string(i)+")");
        } else obj->a = i + 4;
        indices.push(index);
        objects.push(obj);

        if(inUse.find(obj) != inUse.end()){
            throw std::runtime_error("Object already in use");
        }
        if(seen.find(obj) != seen.end()){
            doesRecycle = true;
        }
        inUse.insert(obj);
        seen.insert(obj);
    }
    std::cout << std::endl;

    // release
    for(size_t i=0; i < 8; i++){
        size_t index = indices.front();
        store.release(index);
        std::cout << (i+1) << ". released(" << index << "): " << store.toString([](TestStruct *obj){ return std::to_string(obj->a); }) << std::endl; // TODO REMOVE

        obj = objects.front();
        std::cout << " - " << obj << std::endl; // TODO REMOVE
        if(i < 4){
            if(obj->a != i+4) throw std::runtime_error("Object not as expected (got: "+std::to_string(obj->a)+"; expected: "+std::to_string(i+4)+")");
        } else {
            if(obj->a != i-4) throw std::runtime_error("Object not as expected (got: "+std::to_string(obj->a)+"; expected: "+std::to_string(i-4)+")");
        }
        inUse.erase(obj);
        indices.pop();
        objects.pop();
    }
    std::cout << std::endl;

    // aquire
    for(size_t i=0; i < 1; i++){
        size_t index;
        obj = store.aquire(index);
        std::cout << (i+1) << ". aquired(" << index << "): " << store.toString([](TestStruct *obj){ return std::to_string(obj->a); }) << std::endl; // TODO REMOVE
        std::cout << " - " << obj << std::endl; // TODO REMOVE

        indices.push(index);
        objects.push(obj);

        if(inUse.find(obj) != inUse.end()){
            throw std::runtime_error("Object already in use");
        }
        if(seen.find(obj) != seen.end()){
            doesRecycle = true;
        }
        inUse.insert(obj);
        seen.insert(obj);
    }
    std::cout << std::endl;

    // release
    for(size_t i=0; i < 3; i++){
        size_t index = indices.front();
        store.release(index);
        std::cout << (i+1) << ". released(" << index << "): " << store.toString([](TestStruct *obj){ return std::to_string(obj->a); }) << std::endl; // TODO REMOVE

        obj = objects.front();
        std::cout << " - " << obj << std::endl; // TODO REMOVE
        if(i < 2){
            if(obj->a != i+8) throw std::runtime_error("Object not as expected (got: "+std::to_string(obj->a)+"; expected: "+std::to_string(i+8)+")");
        } else {
            if(obj->a != 0) throw std::runtime_error("Object not as expected (got: "+std::to_string(obj->a)+"; expected: "+std::to_string(0)+")");
        }
        inUse.erase(obj);
        indices.pop();
        objects.pop();
    }
    std::cout << std::endl;

    if(!doesRecycle) throw std::runtime_error("Recycling not detected");
    
    return 0;
}