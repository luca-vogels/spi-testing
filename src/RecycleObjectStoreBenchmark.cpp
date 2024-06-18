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


    // CONCLUSION:  not sure if you should use this at all and simply use new/delete instead ...



    //                                  RELEASE         vs. DEBUG

    // RecycleObjectStoreBitmap(1):     ~ 68.8 Mio/sec  |   ~ 25.6 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        size_t index;
        TestStruct *obj = storeBitmap.aquire(index);
        obj->a = obj->b + obj->c;
        storeBitmap.release(index);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreBitmap(1): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStoreQueue(1):      ~ 325.2 Mio/sec |   ~ 16.2 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        TestStruct *obj = storeQueue.aquire();
        obj->a = obj->b + obj->c;
        storeQueue.release(obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStoreQueue(1): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStoreVector(1):     ~ 234.0 Mio/sec |   ~ 5.6 Mio/sec
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





    // RecycleObjectStoreBitmap(∞):     ~ 27.1 Mio/sec  |   ~ 6.4 Mio/sec
    std::vector<size_t> indices(OPS_PER_ITERATION);
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


    // RecycleObjectStoreQueue(∞):      ~ 212.8 Mio/sec |   ~ 15.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS/OPS_PER_ITERATION; i++){
        TestStruct *obj;
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            obj = storeQueue.aquire();
            obj->a = obj->b + obj->c;
        }
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            storeQueue.release(obj);
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