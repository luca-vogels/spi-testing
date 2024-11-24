/**
 * Concurrent non-blocking queue specifically designed for use with callbacks.
 * 
 * @file CallbackQueueTwoParty.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef CALLBACK_QUEUE_TWO_PARTY_HPP
#define CALLBACK_QUEUE_TWO_PARTY_HPP

#include "./Lock.hpp"

#include <string>

namespace spi {



/**
 * Callback queue that stores callback functions 
 * and executes them one after another.
 * 
 * IMPORTANT:   this implementation is more performant,
 *              HOWEVER only a single thread for pushing
 *              and a single thread for executing is allowed!
 * 
 * @tparam Callback Type of the callback function that returns a bool to indicate if execution was successful.
 * @tparam CallbackArgs Arguments that will be passed to the callback functions.
 */
template<typename Callback, typename... CallbackArgs>
class CallbackQueueTwoParty {
protected:

    struct Node {
        Callback callback;
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


    void cancelAll() noexcept {
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
    void push(Callback callback) noexcept {
        Node* newNode;
        if(recycleHead->next != nullptr){
            newNode = recycleHead;
            recycleHead = recycleHead->next;
            newNode->next = nullptr;
        } else {
            newNode = new Node(callback);
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
    bool execute(CallbackArgs... args){
        while(head->next != nullptr){
            Node* oldHead = head;
            head = head->next;
            Callback cb = oldHead->callback;
            //if(cb == nullptr) throw std::runtime_error("CallbackQueueTwoParty: Callback is nullptr"); // TODO REMOVE

            oldHead->next = nullptr;
            Node* oldRecycleTail = recycleTail;
            recycleTail = oldHead;
            oldRecycleTail->next = oldHead;

            //if(cb == nullptr) throw std::runtime_error("CallbackQueueTwoParty: Callback is nullptr"); // TODO REMOVE
            if(!cb(args...)) return false;
        }
        return true;
    }

    std::string toString() const {
        return "CallbackQueueTwoParty(head="+(head != nullptr ? head->toString() : "nullptr")+
                                "; tail="+(tail != nullptr ? tail->toString() : "nullptr")+")";
    }
};


}

#endif // CALLBACK_QUEUE_TWO_PARTY_HPP