#include "./utils/ThreadSynchronization.hpp"

#include <iostream>

using namespace spi;

int main(){
    const uint64_t ITERATIONS = 100000000;
    BusyConditionWait cond;
    ReadOrWriteAccess rwCond;


    // ConditionWait::check():                  ~70.2 Mio/s
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        cond.check();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "ConditionWait::check(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // ReadWriteConditionWait::accessRead():    ~154.4 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        rwCond.accessRead();
        rwCond.releaseRead();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "ReadWriteConditionWait.accessRead(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // ReadWriteConditionWait::accessWrite():   ~212.9 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        rwCond.accessWrite();
        rwCond.releaseWrite();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "ReadWriteConditionWait.accessWrite(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}