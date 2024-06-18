#include "./utils/RecycleObjectStoreQueue.hpp"

#include <atomic>
#include <chrono>
#include <iostream>

using namespace spi;

struct TestStruct {
    std::atomic<size_t> responses{0};
    bool success = false;

    inline void gotResponse(){
        if(responses.fetch_add(1) == 2){
            success = true;
        }
    }

    inline void reset(){
        responses.store(3);
        success = false;
    }
};



int main(){
    const uint64_t ITERATIONS = 50000000;
    const uint64_t OPS_PER_ITERATION = 9000;
    
    RecycleObjectStoreQueue<TestStruct> store; // choose fasted option based on RecycleObjectStoreBenchmark




    //                          RELEASE         vs. DEBUG

    // Create/Delete(1):        ~ 31.7 Mio/sec  |   ~ 22.2 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        TestStruct *obj = new TestStruct();
        obj->gotResponse();
        obj->gotResponse();
        obj->gotResponse();
        delete obj;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Create/Delete(1): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStore(1):   ~ 36.1 Mio/sec  |   ~ 11.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        TestStruct *obj = store.aquire();
        obj->reset();
        obj->gotResponse();
        obj->gotResponse();
        obj->gotResponse();
        store.release(obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStore(1): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;






    std::vector<TestStruct*> objects(OPS_PER_ITERATION);

    // Create/Delete(∞):        ~ 32.1 Mio/sec  |   ~ 22.6 Mio/sec
    std::vector<size_t> indices(OPS_PER_ITERATION);
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS/OPS_PER_ITERATION; i++){
        TestStruct *obj;
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            obj = new TestStruct();
            obj->gotResponse();
            obj->gotResponse();
            obj->gotResponse();
            delete obj;
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Create/Delete(" << OPS_PER_ITERATION << "): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // RecycleObjectStore(∞):   ~ 33.4 Mio/sec  |   ~ 9.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS/OPS_PER_ITERATION; i++){
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            objects[j] = store.aquire();
            objects[j]->reset();
            objects[j]->gotResponse();
            objects[j]->gotResponse();
            objects[j]->gotResponse();
        }
        for(size_t j=0; j < OPS_PER_ITERATION; j++){
            store.release(objects[j]);
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "RecycleObjectStore(" << OPS_PER_ITERATION << "): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;


    return 0;
}