#include "./utils/CallbackQueueNaive.hpp"
#include "./utils/CallbackQueueRecycle.hpp"
#include "./utils/CallbackQueueThreadSafe.hpp"
#include "./utils/CallbackQueueTwoParty.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>

using namespace spi;

volatile bool b = true;

bool callbackFunction(){
    return b;
}


int main(){
    const uint64_t ITERATIONS = 50000000; // previous 50000000
    CallbackQueueNaive queueNaive;
    CallbackQueueRecycle queueRecycle;
    CallbackQueueThreadSafe<bool(*)()> queueThreadSafe;
    CallbackQueueTwoParty<bool(*)()> queueTwoParty(callbackFunction);


    //                                      RELEASE             |   DEBUG


    // plain callback:                      ~ 2177.8 Mio/sec    |   ~ 35.6 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        bool r = callbackFunction();
        (void)r;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "plain callback: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





    // CallbackQueueNaive() empty:          ~ 84.7 Mio/sec      |   ~ 44.1 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 1x:             ~ 23.6 Mio/sec      |   ~ 13.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push(callbackFunction);
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 2x:             ~ 14.7 Mio/sec      |   ~ 8.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push(callbackFunction);
        queueNaive.push(callbackFunction);
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueNaive() 5x:             ~ 6.7 Mio/sec       |   ~ 3.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueNaive.push(callbackFunction);
        queueNaive.push(callbackFunction);
        queueNaive.push(callbackFunction);
        queueNaive.push(callbackFunction);
        queueNaive.push(callbackFunction);
        queueNaive.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueNaive() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;
    




    // CallbackQueueRecycle() empty:        ~ 87.8 Mio/sec      |   ~ 43.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueRecycle() 1x:           ~ 13.9 Mio/sec      |   ~ 9.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push(callbackFunction);
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueRecycle() 2x:           ~ 7.7 Mio/sec       |   ~ 4.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push(callbackFunction);
        queueRecycle.push(callbackFunction);
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueRecycle() 5x:          ~ 3.2 Mio/sec        |   ~ 2.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueRecycle.push(callbackFunction);
        queueRecycle.push(callbackFunction);
        queueRecycle.push(callbackFunction);
        queueRecycle.push(callbackFunction);
        queueRecycle.push(callbackFunction);
        queueRecycle.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueRecycle() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;
    




    // CallbackQueueThreadSafe() empty:     ~ 108.4 Mio/sec     |   ~ 37.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueThreadSafe.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueThreadSafe() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueThreadSafe() 1x:        ~ 45.2 Mio/sec      |   ~ 15.1 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueThreadSafe() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueThreadSafe() 2x:        ~ 29.3 Mio/sec      |   ~ 9.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueThreadSafe() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueThreadSafe() 5x:        ~ 14.9 Mio/sec      |   ~ 4.5 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.push(callbackFunction);
        queueThreadSafe.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueThreadSafe() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;
    




    // CallbackQueueTwoParty() empty:       ~ 1500.8 Mio/sec    |   ~ 403.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() empty: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // CallbackQueueTwoParty() 1x:          ~ 265.1 Mio/sec     |   ~ 61.1 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.push(callbackFunction);
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() 1x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueTwoParty() 2x:          ~ 109.9 Mio/sec     |   ~ 30.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.push(callbackFunction);
        queueTwoParty.push(callbackFunction);
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() 2x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // CallbackQueueTwoParty() 5x:          ~ 56.1 Mio/sec      |   ~ 13.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint32_t i=0; i < ITERATIONS; i++){
        queueTwoParty.push(callbackFunction);
        queueTwoParty.push(callbackFunction);
        queueTwoParty.push(callbackFunction);
        queueTwoParty.push(callbackFunction);
        queueTwoParty.push(callbackFunction);
        queueTwoParty.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "CallbackQueueTwoParty() 5x: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;
    


    return 0;
}