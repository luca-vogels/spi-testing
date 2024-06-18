#include "./utils/RecycleObjectStoreBitmap.hpp"
#include "./utils/RecycleObjectStoreQueue.hpp"
#include "./utils/RecycleObjectStoreVector.hpp"

#include <chrono>
#include <iostream>

using namespace spi;

struct TestStruct {
    int a;
    int b = 1;
    int c = 1;
};

int main(){
    const uint64_t ITERATIONS = 5000000;
    const uint64_t OPS_PER_ITERATION = 9000;
    
    RecycleObjectStoreBitmap<TestStruct> storeBitmap;
    RecycleObjectStoreQueue<TestStruct> storeQueue;
    RecycleObjectStoreVector<TestStruct> storeVector;


    // CONCLUSION:  CHOSE RecycleObjectStoreQueue FOR BEST PERFORMANCE



    //                                  RELEASE         vs. DEBUG

    // RecycleObjectStoreBitmap(1):     ~ 73.9 Mio/sec  |   ~ 24.7 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        size_t index;
        TestStruct *obj = storeBitmap.aquire(index);
        obj->a = obj->b + obj->c;
        storeBitmap.release(index);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreBitmap(1): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStoreQueue(1):      ~ 326.1 Mio/sec |   ~ 16.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        TestStruct *obj = storeQueue.aquire();
        obj->a = obj->b + obj->c;
        storeQueue.release(obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreQueue(1): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStoreVector(1):     ~ 279.5 Mio/sec |   ~ 5.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        size_t index;
        TestStruct *obj = storeVector.aquire(index);
        obj->a = obj->b + obj->c;
        storeVector.release(index);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreVector(1): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;




    std::vector<size_t> indices(OPS_PER_ITERATION);
    std::vector<TestStruct*> objects(OPS_PER_ITERATION);

    // RecycleObjectStoreBitmap(∞):     ~ 19.7 Mio/sec  |   ~ 5.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS/OPS_PER_ITERATION; i++){
        TestStruct *obj;
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            obj = storeBitmap.aquire(indices[j]);
            obj->a = obj->b + obj->c;
        }
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            storeBitmap.release(indices[j]);
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreBitmap(" << OPS_PER_ITERATION << "): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStoreQueue(∞):      ~ 225.2 Mio/sec |   ~ 13.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS/OPS_PER_ITERATION; i++){
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            objects[j] = storeQueue.aquire();
            objects[j]->a = objects[j]->b + objects[j]->c;
        }
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            storeQueue.release(objects[j]);
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreQueue(" << OPS_PER_ITERATION << "): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStoreVector(∞):     ~ 0.1 Mio/sec   |   unuseable
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS/OPS_PER_ITERATION; i++){
        TestStruct *obj;
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            obj = storeVector.aquire(indices[j]);
            obj->a = obj->b + obj->c;
        }
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            storeVector.release(indices[j]);
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreVector(" << OPS_PER_ITERATION << "): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;


    return 0;
}