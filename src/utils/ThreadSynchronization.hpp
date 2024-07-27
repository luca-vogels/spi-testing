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
#include <shared_mutex>
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

    inline void lock() noexcept {
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

    inline void unlock() noexcept {
        if(reduceCpuUsage){
            mtx.unlock();
            return;
        }
        aquired.store(false, std::memory_order_release);
    }

};



/**
 * Simple condition wait that can be used to pause a thread until a condition is met.
 * Optimized for minimal overhead if condition is met (no waiting needed).
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
    inline void check(std::function<void()> needToPause = nullptr) noexcept {
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

    /**
     * Will allow threads hitting or waiting at the check() method to proceed.
     * @param proceed If true threads will proceed, otherwise they will wait.
     */
    void setProceed(bool proceed) noexcept {
        this->proceed = proceed;
    }
};



/**
 * Synchronizes two groups of threads (one for reading, one for writing) that want to 
 * access a shared resource.
 * In the reading group all threads can access the resource simultaneously however 
 * only one thread of the writing group can access the resource at a time.
 * 
 * Highly optimized if only one reading and only one writing thread and only one of both
 * threads is active most of the time (almost no overhead).
 * 
 * IMPORTANT: if not multithreaded=true then bad performance under high contention between reader and writer!
 * 
 * Uses Peterson's Algorithm (if multithreaded=false)
 */
class ReadOrWriteAccess {
private:
    bool reduceCpuUsage;
    bool multithreaded; // more than one thread per group
    bool simultaneousReads; // if multiple readers can access the resource simultaneously

    volatile bool read = false;  // if reader is interested
    volatile bool write = false; // if writer is interested
    volatile bool writersTurn = false;  // whose turn it is (false = reader, true = writer)

    std::shared_mutex mtx; // only used if multithreaded=true

public:

    /**
     * Create a new ReadOrWriteAccess object.
     * 
     * @param reduceCpuUsage If true the object will use less cpu resources but will be slower.
     * @param multithreaded Set to true if there are multiple readers or multiple writer threads (if only one per group set to false).
     * @param simultaneousReads If true multiple readers can access the resource simultaneously (only relevant if multithreaded=true).
     */
    ReadOrWriteAccess(
        bool reduceCpuUsage, bool multithreaded, bool simultaneousReads
    ) : reduceCpuUsage(reduceCpuUsage), multithreaded(multithreaded), simultaneousReads(simultaneousReads) {}


    /**
     * Sets the CPU usage mode.
     * IMPORTANT: calling thread is not allowed to hold this lock (acts as writer thread)!
     */
    void setReduceCpuUsage(bool reduceCpuUsage){
        if(reduceCpuUsage == this->reduceCpuUsage) return;
        accessWrite();
        this->reduceCpuUsage = reduceCpuUsage;
        releaseWrite();
    }

    /**
     * Sets the multithreaded mode.
     * IMPORTANT: calling thread is not allowed to hold this lock (acts as writer thread)!
     */
    void setMultithreaded(bool multithreaded){
        if(multithreaded == this->multithreaded) return;
        accessWrite(); // lock using old lock method first

        // now first aquire new lock method as well
        if(multithreaded){
            mtx.lock();
        } else {
            write = true;
        }
        this->multithreaded = multithreaded;

        // unlock old lock method first
        if(multithreaded){
            // previously not multi threaded
            write = false;
        } else {
            mtx.unlock();
        }
        releaseWrite(); // unlock new method
    }

    
    /**
     * Reader will pause until the writer is done 
     * and will then acquire exlucsive access.
     */
    inline void accessRead() noexcept {
        if(multithreaded){
            if(simultaneousReads){
                mtx.lock_shared();
            } else {
                mtx.lock();
            }
        } else {
            read = true;
            writersTurn = true;
            while(write && writersTurn) {
                if(reduceCpuUsage) {
                    std::this_thread::yield();
                }
            }
        }
    }

    /**
     * Writer will pause until the reader is done
     * and will then acquire exlucsive access.
     */
    inline void accessWrite() noexcept {
        if(multithreaded){
            mtx.lock();
        } else {
            write = true;
            writersTurn = false;
            while(read && !writersTurn) {
                if(reduceCpuUsage) {
                    std::this_thread::yield();
                }
            }
        }
    }
    
    /**
     * Invoked by the reader to release the resource.
     */
    inline void releaseRead() noexcept {
        if(multithreaded){
            if(simultaneousReads){
                mtx.unlock_shared();
            } else {
                mtx.unlock();
            }
        } else {
            read = false;
        }
    }

    /**
     * Invoked by the writer to release the resource.
     */
    inline void releaseWrite() noexcept {
        if(multithreaded){
            mtx.unlock();
        } else {
            write = false;
        }
    }
};



}

#endif // SPI_THREAD_SYNCHRONIZATION_HPP