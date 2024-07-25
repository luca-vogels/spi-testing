/**
 * Non-blocking thread-safe queue implementation that uses a linked list scheme.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_RING_HPP
#define SPI_QUEUE_RING_HPP

#include <atomic>
#include <vector>

namespace spi {


template<typename T>
class QueueRing {
protected:

    std::vector<T> data;
    std::atomic<size_t> readOffset{0};
    std::atomic<size_t> writeOffset{0};
    std::atomic<size_t> count{0};

public:

    QueueRing(size_t size){
        data.resize(size);
    }


    void push(T data){
        size_t oldCount = count.load();
        if(oldCount == this->data.size()) throw std::runtime_error("Queue is full");
        while(!count.compare_exchange_weak(oldCount, oldCount + 1)){
            oldCount = count.load();
            if(oldCount == this->data.size()) throw std::runtime_error("Queue is full");
        }

        size_t oldWriteOffset = writeOffset.load();
        size_t newWriteOffset = (oldWriteOffset + 1) % this->data.size();
        while(!writeOffset.compare_exchange_weak(oldWriteOffset, newWriteOffset)){
            oldWriteOffset = writeOffset.load();
            newWriteOffset = (oldWriteOffset + 1) % this->data.size();
        }

        this->data[oldWriteOffset] = data;
    }

    bool pop(T& data) noexcept {
        size_t oldCount = count.load();
        if(oldCount == 0) return false;
        while(!count.compare_exchange_weak(oldCount, oldCount - 1)){
            oldCount = count.load();
            if(oldCount == 0) return false;
        }

        size_t oldReadOffset = readOffset.load();
        size_t newReadOffset = (oldReadOffset + 1) % this->data.size();
        while(!readOffset.compare_exchange_weak(oldReadOffset, newReadOffset)){
            oldReadOffset = readOffset.load();
            newReadOffset = (oldReadOffset + 1) % this->data.size();
        }

        data = this->data[oldReadOffset];
        return true;
    }

    bool empty() noexcept {
        return count.load() == 0;
    }

    ~QueueRing() {
        
    }


};



}

#endif // SPI_QUEUE_RING_HPP