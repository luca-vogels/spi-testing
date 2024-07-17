/**
 * Thread-safe queue implementation that uses a linked list scheme.
 * Best performance under low contention.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_TWOPARTY_LC_HPP
#define SPI_QUEUE_TWOPARTY_LC_HPP

#include "./ThreadSynchronization.hpp"

#include <string>

namespace spi {



/**
 * Queue implementation that uses a linked list scheme.
 * 
 * IMPORTANT:   this implementation is more performant,
 *              HOWEVER it is only thread-safe if 
 *              only a single thread for pushing
 *              and a single thread for popping is allowed!
 * IMPORTANT:   Best performance under low contention.
 * 
 */
template<typename T>
class QueueTwoPartyNoCritical {
protected:

    struct Node {
        T data;
        Node* next;

        //Node(T data, Node* next) : data(data), next(next) {}
    };

    Node* head;
    Node* tail;

    Node* recycleHead;
    Node* recycleTail;

public:

    QueueTwoPartyNoCritical() {
        Node* dummy = new Node();
        head = dummy;
        tail = dummy;
        Node* dummy2 = new Node();
        recycleHead = dummy2;
        recycleTail = dummy2;
    }

    ~QueueTwoPartyNoCritical() {
        cancelAll();
        delete head; // delete dummy

        Node* current = recycleHead;
        while(current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    void cancelAll(){
        while(head->next != nullptr){
            Node* oldHead = head;
            head = oldHead->next;
            delete oldHead;
        }
    }

    void push(T data) {
        Node* newNode;
        if(recycleHead->next != nullptr){
            newNode = recycleHead;
            recycleHead = recycleHead->next;
            newNode->next = nullptr;
        } else {
            newNode = new Node();
        }
        Node* oldTail = tail;
        oldTail->data = data;
        tail = newNode;
        oldTail->next = newNode;
    }

    bool pop(T& data) {
        if(head->next == nullptr){
            std::this_thread::yield(); // always yield because of better performance
            return false;
        }
        Node* oldHead = head;
        head = head->next;
        data = oldHead->data;
        
        oldHead->next = nullptr;
        Node* oldRecycleTail = recycleTail;
        recycleTail = oldHead;
        oldRecycleTail->next = oldHead;

        return true;
    }

    bool popAndCheckNext(T& data, bool &hasMore) {
        if(head->next == nullptr){
            hasMore = false;
            std::this_thread::yield(); // always yield because of better performance
            return false;
        }
        Node* oldHead = head;
        head = head->next;
        data = oldHead->data;
        
        oldHead->next = nullptr;
        Node* oldRecycleTail = recycleTail;
        recycleTail = oldHead;
        oldRecycleTail->next = oldHead;

        hasMore = head->next != nullptr;
        return true;
    }

    bool empty() {
        return head->next == nullptr;
    }

    std::string toString() const {
        std::string str = "";
        Node* current = head;
        while(current != nullptr) {
            str += "->"+std::to_string(current->data);
            current = current->next;
        }
        return "Queue(head="+(str.empty() ? "" : str.substr(2))+"; tail="+(tail != nullptr ? std::to_string(tail->data) : "nullptr")+")";
    }
};


/**
 * Queue implementation that uses a linked list scheme.
 * 
 * IMPORTANT:   this implementation is more performant,
 *              HOWEVER it is only thread-safe if 
 *              only a single thread for pushing
 *              and a single thread for popping is allowed!
 * IMPORTANT:   Best performance under low contention.
 * 
 */
/*
template<typename T>
class QueueTwoPartyNoCritical {
protected:

    struct Node {
        T data;
        Node* next;

        //Node(T data, Node* next) : data(data), next(next) {}
    };

    Node* head;
    Node* tail;
    ReadOrWriteAccess access;

public:

    QueueTwoPartyNoCritical(bool reduceCpuUsage) : access(reduceCpuUsage) {
        Node* dummy = new Node();
        head = dummy;
        tail = dummy;
    }

    ~QueueTwoPartyNoCritical() {
        cancelAll();
        delete head; // delete dummy
    }

    void cancelAll(){
        access.accessRead();
        while(head->next != nullptr){
            Node* oldHead = head;
            head = oldHead->next;
            delete oldHead;
        }
        access.accessRead();
    }

    void push(T data) {
        Node* newNode = new Node();
        access.accessWrite();
        Node* oldTail = tail;
        oldTail->data = data;
        tail = newNode;
        oldTail->next = newNode;
        access.releaseWrite();
    }

    bool pop(T& data) {
        access.accessRead();
        if(head->next == nullptr) return false;
        Node* oldHead = head;
        head = head->next;
        access.releaseRead();
        data = oldHead->data;
        delete oldHead;
        return true;
    }

    bool empty() {
        access.accessRead();
        bool isEmpty = head->next == nullptr;
        access.releaseRead();
        return isEmpty;
    }

    std::string toString() const {
        std::string str = "";
        Node* current = head;
        while(current != nullptr) {
            str += "->"+std::to_string(current->data);
            current = current->next;
        }
        return "Queue(head="+(str.empty() ? "" : str.substr(2))+"; tail="+(tail != nullptr ? std::to_string(tail->data) : "nullptr")+")";
    }
};*/



}

#endif // SPI_QUEUE_TWOPARTY_LC_HPP