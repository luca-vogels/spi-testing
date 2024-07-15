/**
 * Thread-safe queue implementation that uses a linked list scheme.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_TWOPARTY_HC_HPP
#define SPI_QUEUE_TWOPARTY_HC_HPP

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace spi {



/**
 * Queue implementation that uses a linked list scheme.
 * 
 * IMPORTANT:   this implementation is more performant,
 *              HOWEVER it is only thread-safe if 
 *              only a single thread for pushing
 *              and a single thread for popping is allowed!
 * IMPORTANT:   Better performance under high contention.
 * 
 */
template<typename T>
class QueueTwoPartyHighContention {
protected:


    // TODO:    see QueueAtomic.hpp for further optimizations

    struct Node; // forward declaration

    struct Pointer {
        Node* ptr = nullptr;
        uint64_t count = 0; // used to uniquely identify a Pointer instance.

        Pointer(){}
        Pointer(Node* node) : ptr(node) {}
        Pointer(Node* node, uint64_t count) : ptr(node), count(count) {}

        bool operator==(const Pointer &other) const {
            return ptr == other.ptr && count == other.count;
        }
    };

    struct Node {
        T data;
        std::atomic<Pointer*> next;

        Node() : next(nullptr) {}
        Node(T data) : data(data) {}
    };

    std::atomic<Pointer*> head;
    std::atomic<Pointer*> tail;

public:

    QueueTwoPartyHighContention(){
        Node* dummy = new Node();
        head.store(new Pointer(dummy));
        tail.store(new Pointer(dummy));
    }

    ~QueueTwoPartyHighContention() {
        cancelAll();
    }

    

    void cancelAll(){
        T data;
        while(pop(data));
    }

    void push(T data) {
        Node* newNode = new Node(data);
        Pointer* oldTail;
        while(true){
            oldTail = tail.load();
            Pointer* newTail = oldTail->ptr->next.load();
            if(oldTail == tail.load()){
                if(newTail->ptr == nullptr){
                    if(oldTail->ptr->next.compare_exchange_strong(newTail, new Pointer(newNode, newTail->count + 1))){
                        delete newTail;
                        break;
                    }
                } else {
                    if(tail.compare_exchange_strong(oldTail, new Pointer(newNode, oldTail->count + 1))){
                        delete newTail;
                    }
                }
            }
        }
        if(tail.compare_exchange_strong(oldTail, new Pointer(newNode, oldTail->count + 1))){
            delete oldTail;
        }
    }

    bool pop(T& data) {
        Pointer* oldHead;
        while(true){
            oldHead = head.load();
            Pointer* oldTail = tail.load();
            Pointer* newHead = oldHead->ptr->next.load();
            if(oldHead == head.load()){
                if(oldHead->ptr == oldTail->ptr){
                    if(newHead->ptr == nullptr)
                        return false;
                    if(tail.compare_exchange_strong(oldTail, new Pointer(newHead->ptr, oldTail->count + 1))){
                        delete oldTail;
                    }
                } else {
                    data = newHead->ptr->data;
                    if(head.compare_exchange_strong(oldHead, new Pointer(newHead->ptr, oldHead->count + 1))){
                        break;
                    }
                }
            }
        }
        delete oldHead->ptr;
        delete oldHead;
        return true;
    }

    bool empty() {
        return head.load()->ptr == tail.load()->ptr;
    }

};



/**
 * Queue implementation that uses a linked list scheme.
 * 
 * IMPORTANT:   this implementation is more performant,
 *              HOWEVER it is only thread-safe if 
 *              only a single thread for pushing
 *              and a single thread for popping is allowed!
 * IMPORTANT:   Better performance under high contention.
 * 
 */
/*
template<typename T>
class QueueTwoPartyHighContention {
protected:


    // TODO:    see QueueAtomic.hpp for further optimizations

    struct Node; // forward declaration

    struct Pointer {
        Node* ptr = nullptr;
        uint64_t count = 0; // used to uniquely identify a Pointer instance.

        Pointer(){}
        Pointer(Node* node) : ptr(node) {}
        Pointer(Node* node, uint64_t count) : ptr(node), count(count) {}

        bool operator==(const Pointer &other) const {
            return ptr == other.ptr && count == other.count;
        }
        bool operator==(const std::shared_ptr<Pointer> &other) const {
            return ptr == other->ptr && count == other->count;
        }
    };

    struct Node {
        T data;
        std::atomic<std::shared_ptr<Pointer>> next;

        Node() : next(nullptr) {}
        Node(T data) : data(data) {}
    };

    std::atomic<std::shared_ptr<Pointer>> head;
    std::atomic<std::shared_ptr<Pointer>> tail;

public:

    QueueTwoPartyHighContention(){
        Node* dummy = new Node();
        head.store(std::make_shared<Pointer>(dummy));
        tail.store(std::make_shared<Pointer>(dummy));
    }

    QueueTwoPartyHighContention(const QueueTwoPartyHighContention&) = delete;
    QueueTwoPartyHighContention& operator=(const QueueTwoPartyHighContention&) = delete;

    ~QueueTwoPartyHighContention() {
        cancelAll();
    }

    

    void cancelAll(){
        T data;
        while(pop(data));
    }

    void push(T data) {
        Node* newNode = new Node(data);
        std::shared_ptr<Pointer> oldTail;
        while(true){
            oldTail = tail.load();
            std::shared_ptr<Pointer> newTail = oldTail->ptr->next.load();
            if(oldTail == tail.load()){
                if(newTail->ptr == nullptr){
                    if(oldTail->ptr->next.compare_exchange_strong(newTail, std::make_shared<Pointer>(newNode, newTail->count + 1)))
                        break;
                } else {
                    tail.compare_exchange_strong(oldTail, std::make_shared<Pointer>(newNode, oldTail->count + 1));
                }
            }
        }
        tail.compare_exchange_strong(oldTail, std::make_shared<Pointer>(newNode, oldTail->count + 1));
    }

    bool pop(T& data) {
        std::shared_ptr<Pointer> oldHead;
        while(true){
            oldHead = head.load();
            std::shared_ptr<Pointer> oldTail = tail.load();
            std::shared_ptr<Pointer> newHead = oldHead->ptr->next.load();
            if(oldHead == head.load()){
                if(oldHead->ptr == oldTail->ptr){
                    if(newHead->ptr == nullptr)
                        return false;
                    tail.compare_exchange_strong(oldTail, std::make_shared<Pointer>(newHead->ptr, oldTail->count + 1));
                } else {
                    data = newHead->ptr->data;
                    if(head.compare_exchange_strong(oldHead, std::make_shared<Pointer>(newHead->ptr, oldHead->count + 1))){
                        break;
                    }
                }
            }
        }
        delete oldHead->ptr;
        return true;
    }

    bool empty() {
        return head.load()->ptr == tail.load()->ptr;
    }

};
*/


/**
 * Queue implementation that uses a linked list scheme.
 * 
 * IMPORTANT:   this implementation is more performant,
 *              HOWEVER it is only thread-safe if 
 *              only a single thread for pushing
 *              and a single thread for popping is allowed!
 * IMPORTANT:   Better performance under high contention.
 * 
 */
/*
template<typename T>
class QueueTwoPartyHighContention {
protected:



    // TODO ISSUE:  when pushing ends data remains in writeHead indefinitely and is never read in case the trade was full on the last push call!



    struct Node {
        T data;
        Node* next;

        Node(T data, Node* next) : data(data), next(next) {}
    };

    Node* writeHead = nullptr;
    Node* writeTail = nullptr;
    
    std::atomic<Node*> trade{nullptr}; // for transferring writeHead to readHead

    Node* readHead = nullptr;

public:

    ~QueueTwoPartyHighContention() {
        cancelAll();
    }

    void cancelAll(){
        while(this->writeHead != nullptr){
            Node* oldHead = this->writeHead;
            this->writeHead = oldHead->next;
            delete oldHead;
        }
        this->writeTail = nullptr;

        while(this->readHead != nullptr){
            Node* oldHead = this->readHead;
            this->readHead = oldHead->next;
            delete oldHead;
        }

        this->readHead = this->trade.exchange(nullptr);
        while(this->readHead != nullptr){
            Node* oldHead = this->readHead;
            this->readHead = oldHead->next;
            delete oldHead;
        }
    }

    void push(T data) {
        Node* newNode = new Node(data, nullptr);
        if(writeTail != nullptr) {
            writeTail->next = newNode;
        } else {
            writeHead = newNode;
        }
        writeTail = newNode;
        
        // try to trade with reader
        newNode = nullptr;
        if(trade.compare_exchange_weak(newNode, writeHead)){
            writeHead = nullptr;
            writeTail = nullptr;
        }
    }

    bool pop(T& data) {
        if(readHead == nullptr){
            readHead = trade.exchange(nullptr);
            if(readHead == nullptr) return false;
        }
        data = readHead->data;
        Node* oldHead = readHead;
        readHead = readHead->next;
        delete oldHead;
        return true;
    }

    bool empty() {
        return readHead == nullptr && writeHead == nullptr && trade.load() == nullptr;
    }

};
*/


}

#endif // SPI_QUEUE_TWOPARTY_HC_HPP