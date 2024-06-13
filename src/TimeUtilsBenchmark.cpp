#include "./utils/TimeUtils.hpp"

#include <iostream>

using namespace spi;

int main(){
    const uint64_t ITERATIONS = 500000000;


    // TimeUtils::now():    ~ 46 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        TimeUtils::now();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "TimeUtils::now(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}