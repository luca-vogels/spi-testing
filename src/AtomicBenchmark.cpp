#include "utils/Atomic.hpp"

#include <algorithm> // max
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>

using namespace spi;

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
    Atomic<int> atomicTwoparty(false, 0);


    //                              RELEASE         |   DEBUG

    // std::atomic.store():                 ~ 172 Mio/sec   |   ~ 100 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        atomicInt.store((int)i);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::atomic.store(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomicTwoparty.store():              ~ 526 Mio/sec   |   ~ 46 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        atomicTwoparty.storeA((int)i);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomicTwoparty.store(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;




    // std::atomic.load():                  ~ 2938 Mio/sec  |   ~ 174 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        volatile int value = atomicInt.load();
        (void)value;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::atomic.load(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomicTwoparty.load():               ~ 463 Mio/sec   |   ~ 48 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        volatile int value = atomicTwoparty.loadA();
        (void)value;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomicTwoparty.load(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;




    // atomic.fetch_add():                  ~ 169 Mio/sec   |   ~ 113 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        atomicInt.fetch_add(1);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::atomic.fetch_add(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomicTwoparty.fetchAdd():           ~ 433 Mio/sec   |   ~ 45 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        atomicTwoparty.fetchAddA(1);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomicTwoparty.fetchAdd(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;




    // std::atomic.exchange():              ~ 175 Mio/sec   |   ~ 112 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < HALF_ITERATIONS; i++){
        atomicStruct.exchange(VAL1);
        atomicStruct.exchange(VAL2);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::atomic.exchange(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomicTwoparty.exchange():           ~ 682 Mio/sec   |   ~ 47 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < HALF_ITERATIONS; i++){
        atomicTwoparty.exchangeA(1);
        atomicTwoparty.exchangeA(2);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomicTwoparty.exchange(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;




    // std::atomic.compareAndSwap():        ~ 162 Mio/sec   |   ~ 34 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < HALF_ITERATIONS; i++){
        atomicStruct.compare_exchange_strong(VAL1, VAL2);
        atomicStruct.compare_exchange_strong(VAL2, VAL1);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::atomic.compareAndSwap(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // atomicTwoparty.compareExchange():    ~ 610 Mio/sec   |   ~ 45 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < HALF_ITERATIONS; i++){
        atomicTwoparty.compareExchangeA(1, 2);
        atomicTwoparty.compareExchangeA(2, 1);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "atomicTwoparty.compareExchange(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;




    // new MyStruct() with delete:          ~ 15 Mio/sec |  ~ 15 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        volatile MyStruct *tmp = new MyStruct();
        (void)tmp;
        delete tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "new MyStruct() with delete: " << (ITERATIONS * 1000000) / std::max((int64_t)1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) << "/s" << std::endl;

    // new MyStruct() no delete:            ~ 15 Mio/sec |  ~ 15 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        volatile MyStruct *tmp = new MyStruct();
        (void)tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "new MyStruct() no delete: " << (ITERATIONS * 1000000) / std::max((int64_t)1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) << "/s" << std::endl;

    return 0;
}