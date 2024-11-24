#include <chrono>
#include <cstdint>
#include <iostream>

#include "utils/Thread.hpp"
#include "utils/Lock.hpp"

#include <condition_variable>
#include <mutex>
#include <shared_mutex>

using namespace spi;


const uint64_t ITERATIONS = 50000000;
const uint64_t THREADS = 8;
std::mutex mutex;
std::shared_mutex sharedMutex;
Lock spinLock(true);
std::condition_variable conditionVariable;
std::vector<Thread*> threads;


int main(){
    const uint64_t MULTITHREADED_ITERATIONS = ITERATIONS / THREADS;



    //                                          RELEASE         |   DEBUG

    // single mutex::lock_guard():              ~ 106 Mio/s     |   ~ 41 Mio/s
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::lock_guard<std::mutex> lock(mutex);
        (void)i;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single mutex::lock_guard(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single mutex::unique_lock():             ~ 124 Mio/s     |   ~ 33 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::unique_lock<std::mutex> lock(mutex);
        (void)i;
        lock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single mutex::unique_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single shared_mutex::unique_lock():      ~ 34 Mio/s      |   ~ 22 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::unique_lock<std::shared_mutex> lock(sharedMutex);
        (void)i;
        lock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single shared_mutex::unique_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single shared_mutex::shared_lock():      ~ 52 Mio/s      |   ~ 25 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::shared_lock<std::shared_mutex> lock(sharedMutex);
        (void)i;
        lock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single shared_mutex::shared_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // single Lock:                         ~ 114 Mio/s     |   ~ 43 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        spinLock.lock();
        (void)i;
        spinLock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "single Lock: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





    // multi mutex::lock_guard():               ~ 12.8 Mio/s    |   ~ 8.4 Mio/s
    for(uint64_t i=0; i < THREADS; i++)
        threads.push_back(new Thread([](){
            for(uint64_t i=0; i < MULTITHREADED_ITERATIONS; i++){
                std::lock_guard<std::mutex> lock(mutex);
                (void)i;
            }
        }));
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->start();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        delete threads[i];
    threads.clear();
    std::cout << "multi mutex::lock_guard(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // multi mutex::unique_lock():              ~ 13.3 Mio/s    |   ~ 7.1 Mio/s
    for(uint64_t i=0; i < THREADS; i++)
        threads.push_back(new Thread([](){
            for(uint64_t i=0; i < MULTITHREADED_ITERATIONS; i++){
                std::unique_lock<std::mutex> lock(mutex);
                (void)i;
            }
        }));
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->start();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        delete threads[i];
    threads.clear();
    std::cout << "multi mutex::unique_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // multi shared_mutex::unique_lock():       ~ 3.8 Mio/s     |   ~ 2.4 Mio/s
    for(uint64_t i=0; i < THREADS; i++)
        threads.push_back(new Thread([](){
            for(uint64_t i=0; i < MULTITHREADED_ITERATIONS; i++){
                std::unique_lock<std::shared_mutex> lock(sharedMutex);
                (void)i;
                lock.unlock();
            }
        }));
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->start();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        delete threads[i];
    threads.clear();
    std::cout << "multi shared_mutex::unique_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // multi shared_mutex::shared_lock():       ~ 6.9 Mio/s     |   ~ 6.3 Mio/s
    for(uint64_t i=0; i < THREADS; i++)
        threads.push_back(new Thread([](){
            for(uint64_t i=0; i < MULTITHREADED_ITERATIONS; i++){
                std::shared_lock<std::shared_mutex> lock(sharedMutex);
                (void)i;
                lock.unlock();
            }
        }));
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->start();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        delete threads[i];
    threads.clear();
    std::cout << "multi shared_mutex::shared_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // multi Lock:                          ~ 42.5 Mio/s    |   ~ 13.7 Mio/s
    for(uint64_t i=0; i < THREADS; i++)
        threads.push_back(new Thread([](){
            for(uint64_t i=0; i < MULTITHREADED_ITERATIONS; i++){
                spinLock.lock();
                (void)i;
                spinLock.unlock();
            }
        }));
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->start();
    for(uint64_t i=0; i < THREADS; i++)
        threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < THREADS; i++)
        delete threads[i];
    threads.clear();
    std::cout << "multi Lock: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





    // condition_variable::notify_one():        ~ 186 Mio/s     |   ~ 149 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        conditionVariable.notify_one();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "condition_variable::notify_one(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // condition_variable::notify_all():        ~ 285 Mio/s     |   ~ 192 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        conditionVariable.notify_all();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "condition_variable::notify_all(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}