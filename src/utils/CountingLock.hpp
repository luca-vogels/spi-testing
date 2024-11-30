/**
 * CountingLocks are a thread synchronization mechanism that allows multiple threads to access a shared resource 
 * simultaneously up to a certain limit.
 * 
 * The special thing is that one thread can aquire the lock multiple times (increases the counter), 
 * while another thread can release multiple times (decreases the counter) for this first thread.
 * 
 * @file CountingLock.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */
#ifndef SPI_COUNTERLOCK_HPP
#define SPI_COUNTERLOCK_HPP

#include "Atomic.hpp"

#include <condition_variable>
#include <mutex>
#include <semaphore>

namespace spi {




class AbstractCountingLock {
public:

    virtual ~AbstractCountingLock() = default;

    virtual bool acquire(bool block = true) = 0;

    virtual void release() = 0;

};




class CountingLockCompSwap : public AbstractCountingLock {
protected:
    Atomic<int32_t> counter;
    int32_t maxCounter;
    std::mutex mutex;
    std::condition_variable cv;

public:

    CountingLockCompSwap(
        int32_t max, bool reduceCpuUsage, bool multithreaded
    ) : AbstractCountingLock(), counter(reduceCpuUsage, multithreaded, 0), maxCounter(max) {
        if(max < 1) throw std::invalid_argument("Max must be at least 1.");
    }

    inline bool acquire(bool block = true) override {
        int32_t curr;
        while(true){
            curr = counter.loadA(std::memory_order_acquire);
            if(curr < maxCounter){
                if(counter.compareExchangeWeakA(curr, curr+1, std::memory_order_acquire)){
                    return true;
                }
            } else if(block){
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock);
            } else {
                return false;
            }
        };
        return false;
    }

    inline void release() override {
        int32_t curr;
        while(true){
            curr = counter.loadB(std::memory_order_release);
            if(curr > 0){
                if(counter.compareExchangeWeakB(curr, curr-1, std::memory_order_release)){
                    cv.notify_all();
                    return;
                }
            } else {
                throw std::runtime_error("Counter is already at 0.");
            }
        }
    }
};



class CountingLockFetch : public AbstractCountingLock {
protected:
    Atomic<int32_t> counter;
    int32_t maxCounter;
    std::mutex mutex;
    std::condition_variable cv;
    bool closing = false;

public:

    /**
     * Creates a counting lock.
     * 
     * @param max Maximum amount of times the lock can be acquired simultaneously before blocking further acquires.
     * @param reduceCpuUsage If true the object will use less cpu resources but will be slower.
     * @param multithreaded Set to true if there are multiple threads either acquiring or releasing the lock.
     *                      If at most one thread acquiring and at most one thread releasing (even if two different ones) set to false.
     */
    CountingLockFetch(
        int32_t max, bool reduceCpuUsage, bool multithreaded
    ) : counter(reduceCpuUsage, multithreaded, 0), maxCounter(max) {
        
    }

    ~CountingLockFetch(){
        close();
    }

    /**
     * Closes the counting lock.
     * Wakes up all waiting threads that are currently trying to aquire the lock
     * and returns false for them as well as for all future calls to acquire().
     */
    inline void close(){
        closing = true;
        cv.notify_all();
    }

    /**
     * Sets the current counter to a given value. 
     * Does not change the maximum.
     * 
     * @param counter New value of the counter.
     */
    inline void setCounter(int32_t counter){
        this->counter.storeB(counter, std::memory_order_release);
    }

    /**
     * Returns the current value of the counter.
     * 
     * @return Current value of the counter.
     */
    inline int32_t getCounter(){
        return counter.loadB(std::memory_order_acquire);
    }

    /**
     * Sets the maximum value of the counter.
     * Does not change the actual counter value.
     * 
     * @param max New maximum value of the counter.
     */
    inline void setMaximum(int32_t max){
        this->maxCounter = max;
    }

    /**
     * Returns the maximum value of the counter.
     * 
     * @return Maximum value of the counter.
     */
    inline int32_t getMaximum(){
        return maxCounter;
    }

    /**
     * Increases the counter.
     * If the counter has already reached the maximum value the calling will be blocked 
     * until the counter is decreased again by calling release().
     * 
     * release() is not required to be called by the same thread that called acquire().
     * 
     * @param block If true and the counter has reached the maxmimum then the calling thread will be blocked 
     *              until the counter is decreased again by calling release().
     * @return True if acquired successfully, false if the counter was already at the maximum and block is false.
     */
    inline bool acquire(bool block = true) override {
        int32_t prevValue;
        while(!this->closing){
            prevValue = counter.fetchAddA(1, std::memory_order_acquire);
            if(prevValue < maxCounter){
                return true;
            } else {
                counter.fetchSubA(1, std::memory_order_acquire);
                if(!block) return false;
                std::unique_lock<std::mutex> lock(mutex);
                if(!this->closing)
                    cv.wait(lock);
            }
        }
        return false;
    }

    /**
     * Decreases the counter and notifies all waiting threads.
     * 
     * release() is not required to be called by the same thread that called acquire().
     */
    inline void release() override {
        int32_t prevValue = counter.fetchSubB(1, std::memory_order_release);
        if(prevValue <= 0){
            counter.fetchAddB(1, std::memory_order_release);
            throw std::runtime_error("Counter is already at 0.");
        }
        cv.notify_all();
    }
};





class CountingLockSemaphore : public AbstractCountingLock {
protected:
    std::counting_semaphore<65535> counter;

public:

    CountingLockSemaphore(int32_t max) : AbstractCountingLock(), counter(max) {
        if(max < 1) throw std::invalid_argument("Max must be at least 1.");
    }

    inline bool acquire(bool block = true) override {
        (void)block;
        counter.acquire();
        return true;
    }

    inline void release() override {
        counter.release();
    }
};




}
#endif