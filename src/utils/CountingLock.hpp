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

    virtual void acquire() = 0;

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

    inline void acquire() override {
        int32_t curr;
        while(true){
            curr = counter.loadA();
            if(curr < maxCounter){
                if(counter.compareExchangeWeakA(curr, curr+1)){
                    return;
                }
            } else {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock);
            }
        }
    }

    inline void release() override {
        int32_t curr;
        while(true){
            curr = counter.loadB();
            if(curr > 0){
                if(counter.compareExchangeWeakB(curr, curr-1)){
                    cv.notify_all();
                    return;
                }
            } else {
                throw std::runtime_error("Counter is already at 0.");
            }
        }
    }
};



class CountingLockFetch : public AbstractCountingLock{
protected:
    Atomic<int32_t> counter;
    int32_t maxCounter;
    std::mutex mutex;
    std::condition_variable cv;

public:

    CountingLockFetch(
        int32_t max, bool reduceCpuUsage, bool multithreaded
    ) : AbstractCountingLock(), counter(reduceCpuUsage, multithreaded, 0), maxCounter(max) {
        if(max < 1) throw std::invalid_argument("Max must be at least 1.");
    }

    inline void acquire() override {
        int32_t curr;
        while(true){
            curr = counter.fetchAddA(1, std::memory_order_acquire);
            if(curr < maxCounter){
                return;
            } else {
                counter.fetchSubA(1, std::memory_order_release);
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock);
            }
        }
    }

    inline void release() override {
        int32_t old = counter.fetchSubB(1, std::memory_order_acquire);
        if(old <= 0){
            counter.fetchAddB(1, std::memory_order_release);
            throw std::runtime_error("Counter is already at 0.");
        }
        cv.notify_all();
    }
};





class CountingLockSemaphore : public AbstractCountingLock{
protected:
    std::counting_semaphore<65535> counter;

public:

    CountingLockSemaphore(int32_t max) : AbstractCountingLock(), counter(max) {
        if(max < 1) throw std::invalid_argument("Max must be at least 1.");
    }

    inline void acquire() override {
        counter.acquire();
    }

    inline void release() override {
        counter.release();
    }
};




}
#endif