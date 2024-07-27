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

    void push(T data) {
        lock.lock(); //std::lock_guard<std::mutex> lock(mutex);
        queue.push(data);
        lock.unlock();
    }

    bool pop(T& data) {
        lock.lock(); //std::lock_guard<std::mutex> lock(mutex);
        if(queue.empty()){
            lock.unlock();
            return false;
        }
        data = queue.front();
        queue.pop();
        lock.unlock();
        return true;
    }

    bool empty() {
        lock.lock(); //std::lock_guard<std::mutex> lock(mutex);
        bool result = queue.empty();
        lock.unlock();
        return result; // return queue.empty();
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

    void push(T data) {
        Node* newNode = new Node(data, nullptr);
        lock.lock(); //std::lock_guard<std::mutex> lock(mutex);
        if(tail != nullptr) {
            tail->next = newNode;
        } else {
            head = newNode;
        }
        tail = newNode;
        lock.unlock();
    }

    bool pop(T& data) {
        lock.lock(); //std::lock_guard<std::mutex> lock(mutex);
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

    bool empty() {
        lock.lock(); //std::lock_guard<std::mutex> lock(mutex);
        bool result = head == nullptr;
        lock.unlock();
        return result; // return head == nullptr;
    }

    ~QueueLockCustom() {
        lock.lock();
        Node* h = head;
        while(h != nullptr) {
            Node* n = h->next;
            delete h;
            h = n;
        }
    }

};


}

#endif // SPI_QUEUE_LOCK_HPP