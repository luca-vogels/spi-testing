#include "./utils/ThreadSynchronization.hpp"

#include <iostream>
#include <mutex>

using namespace spi;

int main(){
    const uint64_t ITERATIONS = 500000000;
    std::mutex mutex;
    SpinLock spinLock;
    BusyConditionWait busyConditionWait;
    ReadOrWriteAccess rwCond(false);


    // FOR MULTI THREADED SPIN LOCK PERFORMANCE SEE MutexBenchmark.cpp



    //                                          RELEASE         |   DEBUG

    // std::lock_guard<std::mutex>:             ~ 113 Mio/s     |   ~  43 Mio/s
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::lock_guard<std::mutex> lock(mutex);
        (void)i;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::lock_guard<std::mutex>: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // SpinLock::lock():                  ~ 88 Mio/s      |   ~  44 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        spinLock.lock();
        (void)i;
        spinLock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "SpinLock::lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // BusyConditionWait::check():              ~ 1057 Mio/s    |   ~ 81 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        busyConditionWait.check();
        (void)i;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "BusyConditionWait::check(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // ReadOrWriteAccess::accessRead():         ~ 632 Mio/s     |   ~ 203 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        rwCond.accessRead();
        (void)i;
        rwCond.releaseRead();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "ReadOrWriteAccess.accessRead(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // ReadOrWriteAccess::accessWrite():        ~ 938 Mio/s     |   ~ 177 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        rwCond.accessWrite();
        (void)i;
        rwCond.releaseWrite();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "ReadOrWriteAccess.accessWrite(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}