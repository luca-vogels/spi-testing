#include "./utils/Future.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>

using namespace spi;


int main(){
    const uint64_t ITERATIONS = 10000000;


    // plain future ( old: ~1.17 Mio/sec ) ( new: ~5.30 Mio/sec )
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Future<int> fut = Future<int>(42);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "plain future: \t\t" << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // with then value ( old: ~0.23 Mio/sec ) ( new: ~2.06 Mio/sec )
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Future<void> fut = Future<int>(42).then<void>([](int val){ (void)val; });
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "with then value: \t" << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // with then future ( old: ~0.19 Mio/sec ) ( new: ~1.57 Mio/sec )
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Future<void> fut = Future<int>(42).then<void>([](int val){ (void)val; return Future<void>(); });
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "with then future: \t" << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // with then void ( old: ~0.23 Mio/sec ) ( new: ~2.10 Mio/sec )
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Future<void> fut = Future<int>(42).then<void>([](int val){ (void)val; return; });
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "with then void: \t" << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // with then future void ( old: 0.19 Mio/sec ) ( new: ~1.56 Mio/sec )
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Future<void> fut = Future<int>(42).then<void>([](int val){ (void)val; return Future<void>(); });
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "with then future void: \t" << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    return 0;
}