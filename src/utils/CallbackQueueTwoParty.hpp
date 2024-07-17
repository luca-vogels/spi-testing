/**
 * Concurrent non-blocking queue specifically designed for use with callbacks.
 * 
 * @file CallbackQueueTwoParty.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef CALLBACK_QUEUE_TWO_PARTY_HPP
#define CALLBACK_QUEUE_TWO_PARTY_HPP

#include "./ThreadSynchronization.hpp"

#include <string>

namespace spi {



template<typename Callback, typename... Args>
class CallbackQueueTwoParty {
protected:

    struct Node {
        Callback callback = nullptr;
        Node* next = nullptr;

        std::string toString() const {
            return "{id="+std::to_string((uint16_t)(uint64_t)(void**)this)+
                        "; cb="+(callback!=nullptr ? "true" :  "nullptr")+
                        "}"+(next!=nullptr ? "->"+next->toString() : "");
        }
    };

    Node* head;
    Node* tail;

    Node* recycleHead;
    Node* recycleTail;

public:

    CallbackQueueTwoParty() {
        Node* dummy = new Node();
        head = dummy;
        tail = dummy;
        Node* dummy2 = new Node();
        recycleHead = dummy2;
        recycleTail = dummy2;
    }

    ~CallbackQueueTwoParty(){
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

    
    /**
     * Queues a callback function that will be executed 
     * when the execute() method gets invoked.
     * Callback will be popped from queue when it returns true.
     * 
     * This method is thread safe.
     * 
     * @param callback Callback that will be queued and executed later.
     */
    void push(Callback callback){
        Node* newNode;
        if(recycleHead->next != nullptr){
            newNode = recycleHead;
            recycleHead = recycleHead->next;
            newNode->next = nullptr;
        } else {
            newNode = new Node();
        }
        Node* oldTail = tail;
        oldTail->callback = callback;
        tail = newNode;
        oldTail->next = newNode;
    }

    /**
     * Executes queued callbacks one after another as long as each 
     * callback returns true. As soon as a callback returns false, 
     * it won't be popped from the queue and the execution will stop 
     * until this method gets invoked again.
     * 
     * Invoking this method while its already running will have no effect.
     * 
     * This method is thread safe.
     * 
     * @param args Arguments that will be passed to the callback functions.
     * @return True if all callbacks got successfully executed and no more are left in the queue.
     */
    bool execute(Args... args){
        while(head->next != nullptr){
            Node* oldHead = head;
            head = head->next;
            const Callback cb = oldHead->callback;
            if(cb == nullptr) throw std::runtime_error("CallbackQueueTwoParty: Callback is nullptr"); // TODO REMOVE

            oldHead->next = nullptr;
            Node* oldRecycleTail = recycleTail;
            recycleTail = oldHead;
            oldRecycleTail->next = oldHead;

            if(cb == nullptr) throw std::runtime_error("CallbackQueueTwoParty: Callback is nullptr"); // TODO REMOVE
            if(!cb(args...)) return false;
        }
        return true;
    }

    std::string toString() const {
        return "CallbackQueueTwoParty{ head="+(head != nullptr ? head->toString() : "nullptr")+
                                "; tail="+(tail != nullptr ? tail->toString() : "nullptr")+" }";
    }
};


template<typename Callback, typename... Args>
class CallbackQueueTwoPartyOLD {
protected:

    struct Entry {
        Callback callback = nullptr;
        Entry* next = nullptr;

        std::string toString() const {
            return "{id="+std::to_string((uint16_t)(uint64_t)(void**)this)+
                        "; cb="+(callback!=nullptr ? "true" :  "nullptr")+
                        "}"+(next!=nullptr ? "->"+next->toString() : "");
        }
    };

    Entry* head = nullptr;
    Entry* tail = nullptr;
    ReadOrWriteAccess access;

public:

    CallbackQueueTwoPartyOLD(bool reduceCpuUsage) : access(reduceCpuUsage) {
        
    }

    ~CallbackQueueTwoPartyOLD(){
        cancelAll();
    }


    void cancelAll(){
        access.accessWrite();
        while(this->head != nullptr){
            Entry* oldHead = this->head;
            this->head = oldHead->next;
            delete oldHead;
        }
        access.releaseWrite();
    }

    
    /**
     * Queues a callback function that will be executed 
     * when the execute() method gets invoked.
     * Callback will be popped from queue when it returns true.
     * 
     * This method is thread safe.
     * 
     * @param callback Callback that will be queued and executed later.
     */
    void push(Callback callback){
        access.accessRead();
        Entry* entry = new Entry();
        entry->callback = callback;
        if(this->tail != nullptr){
            this->tail->next = entry;
        } else {
            this->head = entry;
        }
        this->tail = entry;
        access.releaseRead();
    }

    /**
     * Executes queued callbacks one after another as long as each 
     * callback returns true. As soon as a callback returns false, 
     * it won't be popped from the queue and the execution will stop 
     * until this method gets invoked again.
     * 
     * Invoking this method while its already running will have no effect.
     * 
     * This method is thread safe.
     * 
     * @param args Arguments that will be passed to the callback functions.
     * @return True if all callbacks got successfully executed and no more are left in the queue.
     */
    bool execute(Args... args){
        access.accessWrite();
        while(this->head != nullptr){
            if(this->head->callback(args...)) {
                Entry *oldHead = this->head;
                this->head = oldHead->next;
                delete oldHead;
            } else {
                access.releaseWrite();
                return false;
            }
        }
        access.releaseWrite();
        return true;
    }

    std::string toString() const {
        return "CallbackQueueTwoParty{ head="+(head != nullptr ? head->toString() : "nullptr")+
                                "; tail="+(tail != nullptr ? tail->toString() : "nullptr")+" }";
    }
};


}

#endif // CALLBACK_QUEUE_TWO_PARTY_HPP