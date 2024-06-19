/**
 * Provides classes for thread synchronization.
 * 
 * @file ThreadSynchronization.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_THREAD_SYNCHRONIZATION_HPP
#define SPI_THREAD_SYNCHRONIZATION_HPP

#include <functional>
#include <thread>

namespace spi {


/**
 * Simple condition wait that can be used to pause a thread until a condition is met.
 * Optimized for minimal overhead if condition is met most of the time.
 */
class BusyConditionWait {
private:
    bool proceed = true;

public:

    /**
     * Calling thread will pause until the condition is met.
     * Otherwise immediately returns with minimal overhead.
     * @param needToPause 
     */
    void check(std::function<void()> needToPause = nullptr) {
        while(!proceed) {
            if(needToPause != nullptr){
                needToPause();
                needToPause = nullptr;
            }
            std::this_thread::yield();
        }
    }

    /**
     * Will pause threads hitting the check() method until setProceed() is called.
     */
    void setWait() {
        proceed = false;
    }

    /**
     * Will allow threads hitting or waiting at the check() method to proceed.
     */
    void setProceed() {
        proceed = true;
    }
};



/**
 * Synchronizes two threads (one for reading, one for writing) that want to 
 * access a shared resource.
 * Optimized for minimal overhead if one of the threads is interested most of the time.
 * 
 * Uses Peterson's Algorithm.
 */
class ReadOrWriteAccess {
private:
    bool read = false;  // if reader is interested
    bool write = false; // if writer is interested
    bool turn = false;  // whose turn it is (false = reader, true = writer)

public:
    
    /**
     * Reader will pause until the writer is done 
     * and will then acquire exlucsive access.
     */
    void accessRead(){
        read = true;
        turn = true; // give writer the turn
        while(write && turn){
            std::this_thread::yield();
        }
    }

    /**
     * Writer will pause until the reader is done
     * and will then acquire exlucsive access.
     */
    void accessWrite(){
        write = true;
        turn = false; // give reader the turn
        while(read && !turn){
            std::this_thread::yield();
        }
    }
    
    /**
     * Invoked by the reader to release the resource.
     */
    void releaseRead(){
        read = false;
    }

    /**
     * Invoked by the writer to release the resource.
     */
    void releaseWrite(){
        write = false;
    }
};



}

#endif // SPI_THREAD_SYNCHRONIZATION_HPP