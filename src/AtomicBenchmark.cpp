#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>

struct MyStruct {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
};

int main(){
    const uint64_t ITERATIONS = 500000000;


    const uint64_t HALF_ITERATIONS = ITERATIONS >> 1;
    MyStruct* VAL1 = new MyStruct();
    MyStruct* VAL2 = new MyStruct();
    std::atomic<MyStruct*> atomicStruct{VAL1};
    std::atomic<int> atomicInt{0};

    //                              RELEASE         |   DEBUG

    // atomic.store():             ~ 173.3 Mio/sec  |   ~ 86 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        atomicInt.store((int)i);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomic.store(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomic.load():              ~ 1560.8 Mio/sec |   ~ 368 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        volatile int value = atomicInt.load();
        (void)value;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomic.load(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomic.fetch_add():          ~ 175.4 Mio/sec |   ~ 124 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        atomicInt.fetch_add(1);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomic.fetch_add(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomic.exchange():           ~ 174.9 Mio/sec |   ~ 113 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < HALF_ITERATIONS; i++){
        atomicStruct.exchange(VAL1);
        atomicStruct.exchange(VAL2);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomic.exchange(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomic.compareAndSwap():     ~ 175.2 Mio/sec |   ~ 59 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < HALF_ITERATIONS; i++){
        atomicStruct.compare_exchange_strong(VAL1, VAL2);
        atomicStruct.compare_exchange_strong(VAL2, VAL1);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomic.compareAndSwap(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;



    // new MyStruct() with delete:  ~ 61 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        volatile MyStruct *tmp = new MyStruct();
        (void)tmp;
        delete tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "new MyStruct() with delete: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // new MyStruct() no delete:  ~ 23 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        volatile MyStruct *tmp = new MyStruct();
        (void)tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "new MyStruct() no delete: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}