/**
 * Provides classes for thread synchronization.
 * 
 * @file ThreadSynchronization.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_THREAD_SYNCHRONIZATION_HPP
#define SPI_THREAD_SYNCHRONIZATION_HPP

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

namespace spi {


/**
 * An exclusive access lock providing a lock and unlock method that can be accessed by multiple threads simultaneously.
 * Achieves comparable and better performance than traditional mutexes especially in high contention scenarios, 
 * however it is computational more expensive (if reduceCpuUsage is false).
 * Is not automatically locked on creation and also does not unlock on destruction.
 * 
 */
class Lock {
private:

    bool reduceCpuUsage;

    // for high performance
    std::atomic<bool> aquired{false};

    // for reduce cpu usage
    std::mutex mtx;

public:

    Lock(bool reduceCpuUsage) : reduceCpuUsage(reduceCpuUsage) {}

    /**
     * Changes the mode of the Lock to reduce cpu usage.
     * IMPORTANT: calling thread is not allowed to hold this lock!
     */
    void setReduceCpuUsage(bool reduceCpuUsage){
        if(reduceCpuUsage == this->reduceCpuUsage) return;
        lock();
        this->reduceCpuUsage = reduceCpuUsage;
        unlock();
    }

    void lock() noexcept {
        if(reduceCpuUsage){
            mtx.lock();
            return;
        }
        while(true){

            if(!aquired.exchange(true, std::memory_order_acquire)){
                return; // successfully aquired lock
            }

            while(aquired.load(std::memory_order_relaxed)){ // optimization because load is way faster and uses cache
                std::this_thread::yield(); // way better performance when always including this!
            }
        }
    }

    void unlock() noexcept {
        if(reduceCpuUsage){
            mtx.unlock();
            return;
        }
        aquired.store(false, std::memory_order_release);
    }

};



/**
 * Simple condition wait that can be used to pause a thread until a condition is met.
 * Optimized for minimal overhead if condition is met most of the time.
 */
class BusyConditionWait {
private:
    volatile bool proceed = true;

public:

    /**
     * Calling thread will pause until the condition is met.
     * Otherwise immediately returns with minimal overhead.
     * @param needToPause 
     */
    void check(std::function<void()> needToPause = nullptr) noexcept {
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
    void setWait() noexcept {
        proceed = false;
    }

    /**
     * Will allow threads hitting or waiting at the check() method to proceed.
     */
    void setProceed() noexcept {
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
    bool reduceCpuUsage;

    volatile bool read = false;  // if reader is interested
    volatile bool write = false; // if writer is interested
    volatile bool writersTurn = false;  // whose turn it is (false = reader, true = writer)

public:

    ReadOrWriteAccess(bool reduceCpuUsage) : reduceCpuUsage(reduceCpuUsage) {}

    
    /**
     * Reader will pause until the writer is done 
     * and will then acquire exlucsive access.
     */
    void accessRead() noexcept {
        read = true;
        writersTurn = true;
        while(write && writersTurn) {
            if(reduceCpuUsage) {
                std::this_thread::yield();
            }
        }
    }

    /**
     * Writer will pause until the reader is done
     * and will then acquire exlucsive access.
     */
    void accessWrite() noexcept {
        write = true;
        writersTurn = false;
        while(read && !writersTurn) {
            if(reduceCpuUsage) {
                std::this_thread::yield();
            }
        }
    }
    
    /**
     * Invoked by the reader to release the resource.
     */
    void releaseRead() noexcept {
        read = false;
    }

    /**
     * Invoked by the writer to release the resource.
     */
    void releaseWrite() noexcept {
        write = false;
    }
};



}

#endif // SPI_THREAD_SYNCHRONIZATION_HPP