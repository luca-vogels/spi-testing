#include "./utils/Lock.hpp"
#include "./utils/Thread.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace spi;


// Lock
const bool SPIN_LOCK_TEST = false;
const size_t SPIN_LOCK_ITERATIONS = 5000000;
const size_t SPIN_LOCK_THREADS = 8;
Lock spinLock(true);
bool spinLockAccessTracker[SPIN_LOCK_THREADS];

void runSpinLock(size_t myId){
    const size_t iterations = SPIN_LOCK_ITERATIONS / SPIN_LOCK_THREADS;
    for(size_t i=0; i < iterations; i++){
        spinLock.lock();
        spinLockAccessTracker[myId] = true;
        for(size_t j=0; j < SPIN_LOCK_THREADS; j++){
            if(j != myId && spinLockAccessTracker[j])
                throw std::runtime_error("Multiple threads accessing the same spinlock at the same time myId="+std::to_string(myId)+" otherId="+std::to_string(j)+
                                            " at iteration="+std::to_string(i));
        }
        spinLockAccessTracker[myId] = false;
        spinLock.unlock();
    }
}




// ReadOrWriteAccess
const bool READ_OR_WRITE_ACCESS_TEST = true;
const size_t READ_OR_WRITE_ACCESS_ITERATIONS = 100000;
ReadOrWriteAccess readOrWriteAccess(false, false, false);
volatile int readAccessCounter = 0;
volatile int writeAccessCounter = 0;

void runReadOrWriteAccessREAD(size_t &progress){
    for(size_t i=0; i < READ_OR_WRITE_ACCESS_ITERATIONS; i++){
        readOrWriteAccess.accessRead();

        readAccessCounter = readAccessCounter + 1;
        const int currRead = readAccessCounter;
        const int currWrite = writeAccessCounter;
        if(currRead != 1 || currWrite != 0)
            throw std::runtime_error("Multiple readers at the same time A: reads="+std::to_string(currRead)+" writes="+std::to_string(currWrite)+" i="+std::to_string(i));

        progress = i;
        Thread::sleepUs(1); // needed so compiler does not rearrange code

        readAccessCounter = readAccessCounter - 1;
        const int currRead2 = readAccessCounter;
        const int currWrite2 = writeAccessCounter;
        if(currRead2 != 0 || currWrite2 != 0)
            throw std::runtime_error("Multiple readers at the same time B: reads="+std::to_string(currRead2)+" writes="+std::to_string(currWrite2)+" i="+std::to_string(i));
        
        readOrWriteAccess.releaseRead();
    }
}

void runReadOrWriteAccessWRITE(size_t &progress){
    for(size_t i=0; i < READ_OR_WRITE_ACCESS_ITERATIONS; i++){
        readOrWriteAccess.accessWrite();

        writeAccessCounter = writeAccessCounter + 1;
        const int currRead = readAccessCounter;
        const int currWrite = writeAccessCounter;
        if(currWrite != 1 || currRead != 0)
            throw std::runtime_error("Multiple writes at the same time A: writes="+std::to_string(currWrite)+" reads="+std::to_string(currRead)+" i="+std::to_string(i));

        progress = i;
        Thread::sleepUs(1); // needed so compiler does not rearrange code

        writeAccessCounter = writeAccessCounter - 1;
        const int currRead2 = readAccessCounter;
        const int currWrite2 = writeAccessCounter;
        if(currWrite2 != 0 || currRead2 != 0)
            throw std::runtime_error("Multiple writes at the same time B: writes="+std::to_string(currWrite2)+" reads="+std::to_string(currRead2)+" i="+std::to_string(i));
        
        readOrWriteAccess.releaseWrite();
    }
}


int main(){


    // Lock
    if(SPIN_LOCK_TEST){
        std::cout << "Lock test" << std::endl;  
        Thread* threads[SPIN_LOCK_THREADS];
        for(size_t i=0; i < SPIN_LOCK_THREADS; i++){
            const size_t myId = i;
            spinLockAccessTracker[i] = false;
            threads[i] = new Thread(runSpinLock, myId);
        }
        for(size_t i=0; i < SPIN_LOCK_THREADS; i++)
            threads[i]->start();
        for(size_t i=0; i < SPIN_LOCK_THREADS; i++){
            threads[i]->join();
            delete threads[i];
        }
        std::cout << "Lock test passed" << std::endl;
    }


    // ReadOrWriteAccess
    if(READ_OR_WRITE_ACCESS_TEST){
        std::cout << "ReadOrWriteAccess test" << std::endl;
        size_t readerProgress = 0, writerProgress = 0;
        Thread reader([&readerProgress]{ runReadOrWriteAccessREAD(readerProgress); });
        Thread writer([&writerProgress]{ runReadOrWriteAccessWRITE(writerProgress); });
        reader.start();
        writer.start();
        do {
            Thread::sleepSec(2);
            std::cout << "Reader: " << readerProgress << "/" << READ_OR_WRITE_ACCESS_ITERATIONS <<
                            " (" << std::floor(readerProgress*100/READ_OR_WRITE_ACCESS_ITERATIONS) << "%) | " << 
                        " Writer: " << writerProgress << "/" << READ_OR_WRITE_ACCESS_ITERATIONS << 
                            " (" << std::floor(writerProgress*100/READ_OR_WRITE_ACCESS_ITERATIONS) << "%) | " << std::endl;
        } while(reader.isRunning() || writer.isRunning());  
        reader.join();
        writer.join();
        std::cout << "ReadOrWriteAccess test passed" << std::endl;
    }


    return 0;
}