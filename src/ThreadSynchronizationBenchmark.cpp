#include "./utils/Thread.hpp"
#include "./utils/ThreadSynchronization.hpp"

#include <iostream>
#include <mutex>
#include <vector>

using namespace spi;


const uint64_t ITERATIONS = 50000000; // before 500000000
std::mutex mutex;
Lock spinLock(false);
BusyConditionWait busyConditionWait;
ReadOrWriteAccess rwCond(false, false);
std::vector<Thread*> threads;


const uint64_t THREAD_ITERATIONS = ITERATIONS / 2;
int main(){


    //                                              RELEASE         |   DEBUG

    // single std::lock_guard<std::mutex>:          ~ 104 Mio/s     |   ~  41 Mio/s
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::lock_guard<std::mutex> lock(mutex);
        (void)i;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single std::lock_guard<std::mutex>: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single Lock::lock():                         ~ 112 Mio/s     |   ~  43 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        spinLock.lock();
        (void)i;
        spinLock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single Lock::lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single BusyConditionWait::check():           ~ 1051 Mio/s    |   ~ 85 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        busyConditionWait.check();
        (void)i;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single BusyConditionWait::check(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single ReadOrWriteAccess::accessRead():      ~ 642 Mio/s     |   ~ 230 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        rwCond.accessRead();
        (void)i;
        rwCond.releaseRead();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single ReadOrWriteAccess.accessRead(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single ReadOrWriteAccess::accessWrite():     ~ 661 Mio/s     |   ~ 233 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        rwCond.accessWrite();
        (void)i;
        rwCond.releaseWrite();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single ReadOrWriteAccess.accessWrite(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;






    // multi std::lock_guard<std::mutex>:           ~ 21.2 Mio/s    |   ~  13.4 Mio/s
    for(size_t i=0; i < 2; i++){
        threads.push_back(new Thread([](){
            for(uint64_t i=0; i < THREAD_ITERATIONS; i++){
                std::lock_guard<std::mutex> lock(mutex);
                (void)i;
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) threads[i]->start();
    for(size_t i=0; i < threads.size(); i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) delete threads[i];
    threads.clear();
    std::cout << "multi std::lock_guard<std::mutex>: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    

    // multi Lock:                                  ~ 100.0 Mio/s   |   ~  32.7 Mio/s
    for(size_t i=0; i < 2; i++){
        threads.push_back(new Thread([](){
            for(uint64_t i=0; i < THREAD_ITERATIONS; i++){
                spinLock.lock();
                (void)i;
                spinLock.unlock();
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) threads[i]->start();
    for(size_t i=0; i < threads.size(); i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) delete threads[i];
    threads.clear();
    std::cout << "multi Lock: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // multi BusyConditionWait:                     ~ 2.2 Mio/s     |   ~  2.1 Mio/s
    busyConditionWait.setProceed(true);
    threads.push_back(new Thread([](){
        for(uint64_t i=0; i < THREAD_ITERATIONS; i++){
            busyConditionWait.check();
            (void)i;
        }
    }));
    threads.push_back(new Thread([](){
        for(uint64_t i=0; i < THREAD_ITERATIONS; i++){
            busyConditionWait.setProceed(false);
            std::this_thread::yield();
            busyConditionWait.setProceed(true);
        }
    }));
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) threads[i]->start();
    for(size_t i=0; i < threads.size(); i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) delete threads[i];
    threads.clear();
    std::cout << "multi BusyConditionWait: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // multi ReadOrWriteAccess:                     ~ 9.7 Mio/s     |   ~  9.0 Mio/s
    threads.push_back(new Thread([](){
        for(uint64_t i=0; i < THREAD_ITERATIONS; i++){
            rwCond.accessRead();
            (void)i;
            rwCond.releaseRead();
        }
    }));
    threads.push_back(new Thread([](){
        for(uint64_t i=0; i < THREAD_ITERATIONS; i++){
            rwCond.accessWrite();
            (void)i;
            rwCond.releaseWrite();
        }
    }));
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) threads[i]->start();
    for(size_t i=0; i < threads.size(); i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < threads.size(); i++) delete threads[i];
    threads.clear();
    std::cout << "multi ReadOrWriteAccess: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;


    return 0;
}