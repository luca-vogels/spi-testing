/**
 * Concurrent non-blocking queue specifically designed for use with callbacks.
 * 
 * @file CallbackQueueNaive.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef CALLBACK_QUEUE_NAIVE_HPP
#define CALLBACK_QUEUE_NAIVE_HPP

#include <atomic>
#include <functional>
#include <string>

namespace spi {

class CallbackQueueNaive {
protected:

    class Entry {
    public:
        std::function<bool()> callback = nullptr;
        Entry* next = nullptr;

        std::string toString() const {
            return "{id="+std::to_string((uint16_t)(uint64_t)(void**)this)+
                        "; cb="+(callback!=nullptr ? "true" :  "nullptr")+
                        "}"+(next!=nullptr ? "->"+next->toString() : "");
        }
    };

    Entry* head = nullptr;
    std::atomic<Entry*> tail{nullptr};
    std::atomic<bool> executing{false};

public:

    ~CallbackQueueNaive(){
        while(this->head != nullptr){
            Entry* oldHead = this->head;
            this->head = oldHead->next;
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
    void push(std::function<bool()> callback){
        Entry* entry = new Entry();
        entry->callback = callback;
        Entry* oldTail = this->tail.exchange(entry);
        if (oldTail == nullptr) {
            this->head = entry;
        } else {
            oldTail->next = entry;
        }
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
     * @return True if all callbacks got successfully executed and no more are left in the queue.
     */
    bool execute(){
        if(executing.exchange(true)) return true;
        bool hasMore = this->head != nullptr;
        while(hasMore){
            if(this->head->callback()) {
                Entry *oldHead = this->head;
                Entry *tmp = oldHead; // tmp needed because compare_exchange_strong() assigns current value to it
                this->head = oldHead->next; 
                this->tail.compare_exchange_strong(tmp, oldHead->next); // if tail is same as head set it to nullptr

                delete oldHead;

                hasMore = this->head != nullptr;
            } else {
                break;
            }
        }
        executing.store(false);
        return !hasMore;
    }

    std::string toString() const {
        Entry *tail = this->tail.load();
        return "CallbackQueueNaive{ head="+(head != nullptr ? head->toString() : "nullptr")+
                                "; tail="+(tail != nullptr ? tail->toString() : "nullptr")+" }";
    }
};


}

#endif // CALLBACK_QUEUE_NAIVE_HPP