/**
 * Thread-safe queue implementation that uses a linked list scheme.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_TWOPARTY_HPP
#define SPI_QUEUE_TWOPARTY_HPP

#include "./ThreadSynchronization.hpp"

#include <mutex>

namespace spi {



/**
 * Queue implementation that uses a linked list scheme.
 * 
 * IMPORTANT:   this implementation is more performant,
 *              HOWEVER it is only thread-safe if 
 *              only a single thread for pushing
 *              and a single thread for popping is allowed!
 * 
 */
template<typename T>
class QueueTwoParty {
protected:

    struct Node {
        T data;
        Node* next;

        Node(T data, Node* next) : data(data), next(next) {}
    };

    Node* head = nullptr;
    Node* tail = nullptr;
    ReadOrWriteAccess access;

public:

    ~QueueTwoParty() {
        cancelAll();
    }

    void cancelAll(){
        access.accessRead();
        while(this->head != nullptr){
            Node* oldHead = this->head;
            this->head = oldHead->next;
            delete oldHead;
        }
        this->tail = nullptr;
        access.accessRead();
    }

    void push(T data) {
        Node* newNode = new Node(data, nullptr);
        access.accessWrite();
        if(tail != nullptr) {
            tail->next = newNode;
        } else {
            head = newNode;
        }
        tail = newNode;
        access.releaseWrite();
    }

    bool pop(T& data) {
        access.accessRead();
        if(head == nullptr){
            access.releaseRead();
            return false;
        }
        data = head->data;
        Node* oldHead = head;
        head = head->next;
        if(head == nullptr) tail = nullptr;
        access.releaseRead();
        delete oldHead;
        return true;
    }

    bool empty() {
        access.accessRead();
        bool isEmpty = head == nullptr;
        access.releaseRead();
        return isEmpty;
    }

};



}

#endif // SPI_QUEUE_TWOPARTY_HPP