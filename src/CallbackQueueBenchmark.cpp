#include "./utils/CallbackQueueLock.hpp"
#include "./utils/CallbackQueueNaive.hpp"
#include "./utils/CallbackQueueRecycle.hpp"
#include "./utils/CallbackQueueTwoParty.hpp"

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
    CallbackQueueTwoParty<bool(*)()> queueTwoParty(false);



    // plain callback:                      ~ 34.6 Mio/sec
    std::function<bool()> plainCallback = [](){return true;};
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        bool r = plainCallback();
        (void)r;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "plain callback: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





    // CallbackQueueNaive() empty:          ~ 44.1 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 1x:             ~ 13.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push([](){return true;});
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 2x:             ~ 8.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push([](){return true;});
        queueNaive.push([](){return true;});
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 5x:             ~ 3.6 Mio/sec
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
    std::cout << std::endl;
    




    // CallbackQueueRecycle() empty:        ~ 43.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueRecycle() 1x:           ~ 9.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push([](){return true;});
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueRecycle() 2x:           ~ 4.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push([](){return true;});
        queueRecycle.push([](){return true;});
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueRecycle() 5x:          ~ 2.0 Mio/sec
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
    std::cout << std::endl;
    




    // CallbackQueueLock() empty:       ~ 26.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueLock.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueLock() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueLock() 1x:          ~ 9.2 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueLock.push([](){return true;});
        queueLock.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueLock() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueLock() 2x:          ~ 5.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueLock.push([](){return true;});
        queueLock.push([](){return true;});
        queueLock.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueLock() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueLock() 5x:          ~ 2.4 Mio/sec
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
    std::cout << std::endl;
    




    // CallbackQueueTwoParty() empty:   ~ 119.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueTwoParty() 1x:      ~ 16.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.push([](){return true;});
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueTwoParty() 2x:      ~ 8.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.push([](){return true;});
        queueTwoParty.push([](){return true;});
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueTwoParty() 5x:      ~ 3.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.push([](){return true;});
        queueTwoParty.push([](){return true;});
        queueTwoParty.push([](){return true;});
        queueTwoParty.push([](){return true;});
        queueTwoParty.push([](){return true;});
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;
    


    return 0;
}