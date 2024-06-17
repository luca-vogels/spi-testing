#include <chrono>
#include <cstdint>
#include <iostream>

#include <condition_variable>
#include <mutex>
#include <shared_mutex>


int main(){
    const uint64_t ITERATIONS = 50000000;
    std::mutex mutex;
    std::shared_mutex sharedMutex;
    std::condition_variable conditionVariable;


    // mutex::lock_guard():             ~ 42.4 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::lock_guard<std::mutex> lock(mutex);
        (void)i;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "mutex::lock_guard(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // mutex::unique_lock():              ~ 33.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::unique_lock<std::mutex> lock(mutex);
        (void)i;
        lock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "mutex::unique_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // shared_mutex::unique_lock():          ~ 22.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::unique_lock<std::shared_mutex> lock(sharedMutex);
        (void)i;
        lock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "shared_mutex::unique_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // shared_mutex::shared_lock():           ~ 24.5 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::shared_lock<std::shared_mutex> lock(sharedMutex);
        (void)i;
        lock.unlock();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "shared_mutex::shared_lock(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;



    // condition_variable::notify_one():     ~  235.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        conditionVariable.notify_one();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "condition_variable::notify_one(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // condition_variable::notify_all():     ~  258.1 Mio/sec   <-- ALWAYS WINNER over notify_one()
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        conditionVariable.notify_all();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "condition_variable::notify_all(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}