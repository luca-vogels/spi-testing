#include "./utils/CallbackQueueLock.hpp"
#include "./utils/CallbackQueueNaive.hpp"
#include "./utils/CallbackQueueRecycle.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>

using namespace spi;

int main(){
    const uint64_t ITERATIONS = 10000000;
    CallbackQueueNaive queueNaive;
    CallbackQueueRecycle queueRecycle;
    CallbackQueueLock queueLock;


    // RANKING / TESTS:         empty   1x      2x      5x      avg
    // 1. CallbackQueueNaive:   2       3       1       1       1.8
    // 2. CallbackQueueLock:    3       1       2       2       2.0
    // 3. CallbackQueueRecycle: 1       2       3       3       2.3



    // plain callback:                      ~ 34.2 Mio/sec
    std::function<bool()> plainCallback = [](){return true;};
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        bool r = plainCallback();
        (void)r;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "plain callback: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;





    // CallbackQueueNaive() empty:          ~ 42.1 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 1x:          ~ 3.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push([&i](){return true;});
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 2x:          ~ 1.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push([](){return true;});
        queueNaive.push([](){return true;});
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 5x:          ~ 0.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push([](){return true;});
        queueNaive.push([](){return true;});
        queueNaive.push([](){return true;});
        queueNaive.push([](){return true;});
        queueNaive.push([](){return true;});
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;





    // CallbackQueueRecycle() empty:          ~ 44.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueRecycle() 1x:          ~ 3.2 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push([](){return true;});
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueRecycle() 2x:          ~ 1.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push([](){return true;});
        queueRecycle.push([](){return true;});
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueRecycle() 5x:          ~ 0.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push([](){return true;});
        queueRecycle.push([](){return true;});
        queueRecycle.push([](){return true;});
        queueRecycle.push([](){return true;});
        queueRecycle.push([](){return true;});
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;





    // CallbackQueueLock() empty:          ~29.5 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueLock.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueLock() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueLock() 1x:          ~ 3.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueLock.push([](){return true;});
        queueLock.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueLock() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueLock() 2x:          ~ 1.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueLock.push([](){return true;});
        queueLock.push([](){return true;});
        queueLock.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueLock() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueLock() 5x:          ~ 0.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueLock.push([](){return true;});
        queueLock.push([](){return true;});
        queueLock.push([](){return true;});
        queueLock.push([](){return true;});
        queueLock.push([](){return true;});
        queueLock.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueLock() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}