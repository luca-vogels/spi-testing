/**
 * Non-blocking thread-safe queue implementation that uses a linked list scheme.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_LOCK_HPP
#define SPI_QUEUE_LOCK_HPP

#include "./ThreadSynchronization.hpp"

#include <queue>
#include <mutex>

namespace spi {


template<typename T>
class QueueLock {
protected:

    Lock lock;
    std::queue<T> queue;

public:

    QueueLock(bool reduceCpuUsage) : lock(reduceCpuUsage) {}

    ~QueueLock() noexcept {
        cancelAll();
    }

    void cancelAll() noexcept {
        lock.lock();
        while(!queue.empty()) queue.pop();
        lock.unlock();
    }

    void push(T data) noexcept {
        lock.lock();
        queue.push(data);
        lock.unlock();
    }

    bool pop(T& data) noexcept {
        lock.lock();
        if(queue.empty()){
            lock.unlock();
            return false;
        }
        data = queue.front();
        queue.pop();
        lock.unlock();
        return true;
    }

    bool popAndCheckNext(T& data, bool &hasMore) noexcept {
        lock.lock();
        if(queue.empty()){
            hasMore = false;
            lock.unlock();
            return false;
        }
        data = queue.front();
        queue.pop();
        hasMore = !queue.empty();
        lock.unlock();
        return true;
    }

    bool empty() noexcept {
        lock.lock();
        bool result = queue.empty();
        lock.unlock();
        return result;
    }

};



template<typename T>
class QueueLockCustom {
protected:

    struct Node {
        T data;
        Node* next;

        Node(T data, Node* next) : data(data), next(next) {}
    };

    Lock lock;
    Node* head;
    Node* tail;

public:

    QueueLockCustom(bool reduceCpuUsage) : lock(reduceCpuUsage), head(nullptr), tail(nullptr) {}

    ~QueueLockCustom() noexcept {
        cancelAll();
    }


    void cancelAll() noexcept {
        lock.lock();
        while(this->head != nullptr){
            Node* oldHead = this->head;
            this->head = oldHead->next;
            delete oldHead;
        }
        this->tail = nullptr;
        lock.unlock();
    }

    void push(T data) noexcept {
        Node* newNode = new Node(data, nullptr);
        lock.lock();
        if(tail != nullptr) {
            tail->next = newNode;
        } else {
            head = newNode;
        }
        tail = newNode;
        lock.unlock();
    }

    bool pop(T& data) noexcept {
        lock.lock();
        if(head == nullptr){
            lock.unlock();
            return false;
        }
        data = head->data;
        Node* oldHead = head;
        head = head->next;
        delete oldHead;
        if(head == nullptr) tail = nullptr;
        lock.unlock();
        return true;
    }

    bool popAndCheckNext(T& data, bool &hasMore) noexcept {
        lock.lock();
        if(head == nullptr){
            hasMore = false;
            lock.unlock();
            return false;
        }
        data = head->data;
        Node* oldHead = head;
        head = head->next;
        delete oldHead;
        if(head == nullptr) tail = nullptr;
        hasMore = head != nullptr;
        lock.unlock();
        return true;
    }

    bool empty() noexcept {
        lock.lock();
        bool result = head == nullptr;
        lock.unlock();
        return result;
    }

};


}

#endif // SPI_QUEUE_LOCK_HPP