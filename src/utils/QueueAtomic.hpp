/**
 * Non-blocking thread-safe queue implementation that uses a linked list scheme.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_ATOMIC_HPP
#define SPI_QUEUE_ATOMIC_HPP

#include <atomic>

#include <iostream> // TODO REMOVE

namespace spi {


template<typename T>
class QueueAtomic {
protected:

    struct Node {
        T data;
        Node* next;

        Node(T data, Node* next) : data(data), next(next) {}

    };

    alignas(alignof(Node*)) std::atomic<Node*> head{nullptr};
    alignas(alignof(Node*)) std::atomic<Node*> tail{nullptr};

public:

    QueueAtomic(){
        Node* sentinel = new Node(T(), nullptr);
        head.store(sentinel);
        tail.store(sentinel);
    }


    void push(T data) {
        Node* newNode = new Node(data, nullptr);
        while(true){
            Node* oldTail = tail.load();
            if(tail.compare_exchange_weak(oldTail, newNode)){
                oldTail->next = newNode;
                break;
            }
        }
    }

    bool pop(T& data) {
        while(true){
            Node* oldHead = head.load();
            Node* oldTail = tail.load();
            Node* firstNode = oldHead->next;

            if(oldHead == oldTail){
                if(firstNode == nullptr) return false;
                // Help other threads complete push operation
                tail.compare_exchange_weak(oldTail, firstNode);
            } else {
                if(head.compare_exchange_weak(oldHead, firstNode)){
                    data = firstNode->data;
                    delete oldHead;
                    return true;
                }
            }
        }
    }

    bool empty() {
        return head.load()->next.load() == nullptr;
    }

    ~QueueAtomic() {
        Node* h = this->head;
        while(h != nullptr) {
            Node* n = h->next;
            delete h;
            h = n;
        }
    }


};



}

#endif // SPI_QUEUE_ATOMIC_HPP