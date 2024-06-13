#include "./utils/ThreadSynchronization.hpp"
#include "./utils/Thread.hpp"

#include <iostream>
#include <stdexcept>

using namespace spi;


ReadOrWriteAccess cond;
const size_t ITERATIONS = 2500;
int accessCount = 0;


void runRead(){
    for(size_t i=0; i < ITERATIONS; i++){
        cond.accessRead();
        accessCount++;
        if(accessCount != 1) throw std::runtime_error("Multiple readers at the same time: "+std::to_string(accessCount));

        Thread::sleepUs(100);

        accessCount--;
        if(accessCount != 0) throw std::runtime_error("Multiple readers at the same time: "+std::to_string(accessCount));
        cond.releaseRead();
    }
}

void runWrite(){
    for(size_t i=0; i < ITERATIONS; i++){
        cond.accessWrite();
        accessCount++;
        if(accessCount != 1) throw std::runtime_error("Multiple writers at the same time: "+std::to_string(accessCount));

        Thread::sleepUs(100);

        accessCount--;
        if(accessCount != 0) throw std::runtime_error("Multiple writers at the same time: "+std::to_string(accessCount));
        cond.releaseWrite();
    }
}


int main(){
    Thread reader(runRead);
    reader.start();

    runWrite();

    reader.join();
    return 0;
}