#include "utils/CountingLock.hpp"
#include "utils/Thread.hpp"

#include <algorithm> // max
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <semaphore>
#include <vector>

using namespace spi;




int main(){
    const uint32_t SIMPLE_ITERATIONS = 10000000; // <-  Debug: 5000000;     Release: 10000000
    const uint32_t SIMPLE_SLEEP_US = 0;

    const uint32_t CONTENTION_MAX = 10;
    const uint32_t CONTENTION_THREADS = 100;
    const uint32_t CONTENTION_ITERATIONS = 50000; // <- Debug: 50000;       Release: 50000
    const uint32_t CONTENTION_SLEEP_US = 0;

    // Simple
    std::counting_semaphore<1> semaphoreTwoParty{1};
    CountingLockCompSwap lockCompSwapTwoParty(1, false, false);
    CountingLockFetch lockFetchTwoParty(1, false, false);

    // High Contention
    std::counting_semaphore<CONTENTION_MAX> semaphoreSafe(CONTENTION_MAX);
    CountingLockCompSwap lockCompSwapSafe(CONTENTION_MAX, false, true);
    CountingLockFetch lockFetchSafe(CONTENTION_MAX, false, true);



    //                              RELEASE         |   DEBUG

    // Simple std::counting_semaphore:     ~ 165 /sec   |   ~ 154 /sec         
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < SIMPLE_ITERATIONS;){
        semaphoreTwoParty.acquire();
        if(SIMPLE_SLEEP_US > 0) Thread::sleepUs(SIMPLE_SLEEP_US);
        i++;
        semaphoreTwoParty.release();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Simple counting_semaphore: " << (SIMPLE_ITERATIONS * 1000000) / 
                    std::max((int64_t)1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() - 
                            (SIMPLE_ITERATIONS * SIMPLE_SLEEP_US)) << "/s" << std::endl;


    // Simple CountingLockCompSwap:        ~ 3651 /sec  |   ~ 1071 /sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < SIMPLE_ITERATIONS;){
        lockCompSwapTwoParty.acquire();
        if(SIMPLE_SLEEP_US > 0) Thread::sleepUs(SIMPLE_SLEEP_US);
        i++;
        lockCompSwapTwoParty.release();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Simple CountingLockCompSwap: " << (SIMPLE_ITERATIONS * 1000000) / 
                    std::max((int64_t)1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() - 
                            (SIMPLE_ITERATIONS * SIMPLE_SLEEP_US)) << "/s" << std::endl;


    // Simple CountingLockFetch:           ~ 4337 /sec  |   ~ 1741 /sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < SIMPLE_ITERATIONS;){
        lockFetchTwoParty.acquire();
        if(SIMPLE_SLEEP_US > 0) Thread::sleepUs(SIMPLE_SLEEP_US);
        i++;
        lockFetchTwoParty.release();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Simple CountingLockFetch: " << (SIMPLE_ITERATIONS * 1000000) / 
                    std::max((int64_t)1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() - 
                            (SIMPLE_ITERATIONS * SIMPLE_SLEEP_US)) << "/s" << std::endl;
    std::cout << std::endl;





    // Contention std::counting_semaphore:  ~ 124 /sec  |   ~ 119 /sec
    std::atomic<uint32_t> readyCounter{0};
    std::vector<Thread*> threads;
    for(uint32_t i=0; i < CONTENTION_THREADS; i++){
        threads.push_back(new Thread([&semaphoreSafe, CONTENTION_THREADS, CONTENTION_ITERATIONS, &readyCounter](){
            readyCounter.fetch_add(1);
            while(readyCounter.load() < CONTENTION_THREADS) std::this_thread::yield();

            for(uint32_t i=0; i < CONTENTION_ITERATIONS;){
                semaphoreSafe.acquire();
                if(CONTENTION_SLEEP_US > 0) Thread::sleepUs(CONTENTION_SLEEP_US); else std::this_thread::yield();
                i++;
                semaphoreSafe.release();
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(Thread* thr : threads) thr->start();
    for(Thread* thr : threads) thr->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(Thread* thr : threads) delete thr;
    std::cout << "Contention counting_semaphore: " << (CONTENTION_THREADS * CONTENTION_ITERATIONS * 1000000) / 
                std::max((int64_t )1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() - 
                    (CONTENTION_ITERATIONS * CONTENTION_SLEEP_US)) << "/s" << std::endl;


    // Contention CountingLockCompSwap:     ~ 114 /sec  |   ~ 91 /sec
    readyCounter.store(0);
    threads.clear();
    for(uint32_t i=0; i < CONTENTION_THREADS; i++){
        threads.push_back(new Thread([&lockCompSwapSafe, CONTENTION_THREADS, CONTENTION_ITERATIONS, &readyCounter](){
            readyCounter.fetch_add(1);
            while(readyCounter.load() < CONTENTION_THREADS) std::this_thread::yield();

            for(uint32_t i=0; i < CONTENTION_ITERATIONS; ){
                lockCompSwapSafe.acquire();
                if(CONTENTION_SLEEP_US > 0) Thread::sleepUs(CONTENTION_SLEEP_US); else std::this_thread::yield();
                i++;
                lockCompSwapSafe.release();
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(Thread* thr : threads) thr->start();
    for(Thread* thr : threads) thr->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(Thread* thr : threads) delete thr;
    std::cout << "Contention CountingLockCompSwap: " << (CONTENTION_THREADS * CONTENTION_ITERATIONS * 1000000) / 
                std::max((int64_t )1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() - 
                    (CONTENTION_ITERATIONS * CONTENTION_SLEEP_US)) << "/s" << std::endl;


    // Contention CountingLockFetch:        ~ 128 /sec  |   ~ 127 /sec
    readyCounter.store(0);
    threads.clear();
    for(uint32_t i=0; i < CONTENTION_THREADS; i++){
        threads.push_back(new Thread([&lockFetchSafe, CONTENTION_THREADS, CONTENTION_ITERATIONS, &readyCounter](){
            readyCounter.fetch_add(1);
            while(readyCounter.load() < CONTENTION_THREADS) std::this_thread::yield();

            for(uint32_t i=0; i < CONTENTION_ITERATIONS; ){
                lockFetchSafe.acquire();
                if(CONTENTION_SLEEP_US > 0) Thread::sleepUs(CONTENTION_SLEEP_US); else std::this_thread::yield();
                i++;
                lockFetchSafe.release();
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(Thread* thr : threads) thr->start();
    for(Thread* thr : threads) thr->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(Thread* thr : threads) delete thr;
    std::cout << "Contention CountingLockFetch: " << (CONTENTION_THREADS * CONTENTION_ITERATIONS * 1000000) / 
                std::max((int64_t )1, std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() - 
                    (CONTENTION_ITERATIONS * CONTENTION_SLEEP_US)) << "/s" << std::endl;
    std::cout << std::endl;

    return 0;
}