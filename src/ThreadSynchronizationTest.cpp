#include "./utils/ThreadSynchronization.hpp"
#include "./utils/Thread.hpp"

#include <iostream>
#include <stdexcept>

using namespace spi;


ReadOrWriteAccess cond(false);
const size_t ITERATIONS = 250000000;
volatile int readAccessCounter = 0;
volatile int writeAccessCounter = 0;


void runRead(){
    for(size_t i=0; i < ITERATIONS; i++){
        cond.accessRead();

        readAccessCounter = readAccessCounter + 1;
        const int currRead = readAccessCounter;
        const int currWrite = writeAccessCounter;
        if(currRead != 1 || currWrite != 0)
            throw std::runtime_error("Multiple readers at the same time A: reads="+std::to_string(currRead)+" writes="+std::to_string(currWrite)+" i="+std::to_string(i));

        //Thread::sleepUs(100);

        readAccessCounter = readAccessCounter - 1;
        const int currRead2 = readAccessCounter;
        const int currWrite2 = writeAccessCounter;
        if(currRead2 != 0 || currWrite2 != 0)
            throw std::runtime_error("Multiple readers at the same time B: reads="+std::to_string(currRead2)+" writes="+std::to_string(currWrite2)+" i="+std::to_string(i));
        
        cond.releaseRead();
    }
}

void runWrite(){
    for(size_t i=0; i < ITERATIONS; i++){
        cond.accessWrite();

        writeAccessCounter = writeAccessCounter + 1;
        const int currRead = readAccessCounter;
        const int currWrite = writeAccessCounter;
        if(currWrite != 1 || currRead != 0)
            throw std::runtime_error("Multiple writes at the same time A: writes="+std::to_string(currWrite)+" reads="+std::to_string(currRead)+" i="+std::to_string(i));

        //Thread::sleepUs(100);

        writeAccessCounter = writeAccessCounter - 1;
        const int currRead2 = readAccessCounter;
        const int currWrite2 = writeAccessCounter;
        if(currWrite2 != 0 || currRead2 != 0)
            throw std::runtime_error("Multiple writes at the same time B: writes="+std::to_string(currWrite2)+" reads="+std::to_string(currRead2)+" i="+std::to_string(i));
        
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