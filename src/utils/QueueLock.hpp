/**
 * Non-blocking thread-safe queue implementation that uses a linked list scheme.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_LOCK_HPP
#define SPI_QUEUE_LOCK_HPP

#include <mutex>

namespace spi {


template<typename T>
class QueueLock {
protected:

    struct Node {
        T data;
        Node* next;

        Node(T data, Node* next) : data(data), next(next) {}
    };

    std::mutex mutex;
    Node* head;
    Node* tail;

public:

    QueueLock() : head(nullptr), tail(nullptr) {}

    void push(T data) {
        Node* newNode = new Node(data, nullptr);
        std::lock_guard<std::mutex> lock(mutex);
        if(tail != nullptr) {
            tail->next = newNode;
        } else {
            head = newNode;
        }
        tail = newNode;
    }

    bool pop(T& data) {
        std::lock_guard<std::mutex> lock(mutex);
        if(head == nullptr) return false;
        data = head->data;
        Node* oldHead = head;
        head = head->next;
        delete oldHead;
        if(head == nullptr) tail = nullptr;
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mutex);
        return head == nullptr;
    }

    ~QueueLock() {
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