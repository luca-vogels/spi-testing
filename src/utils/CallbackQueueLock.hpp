/**
 * Concurrent queue specifically designed for use with callbacks.
 * 
 * @file CallbackQueueLock.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef CALLBACK_QUEUE_LOCK_HPP
#define CALLBACK_QUEUE_LOCK_HPP

#include <functional>
#include <mutex>
#include <string>

namespace spi {

class CallbackQueueLock {
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
    Entry* tail = nullptr;
    bool executing = false;
    std::mutex mutex;

public:

    ~CallbackQueueLock(){
        std::lock_guard<std::mutex> lock(this->mutex);
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
        std::lock_guard<std::mutex> lock(this->mutex);
        Entry *oldTail = this->tail;
        this->tail = entry;
        if(oldTail == nullptr) {
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
        std::unique_lock<std::mutex> lock(this->mutex);
        if(this->executing) return true;
        this->executing = true;
        lock.unlock();
        
        bool hasMore = this->head != nullptr;
        while(hasMore){
            if(this->head->callback()) {
                lock.lock();
                Entry *oldHead = this->head;
                this->head = oldHead->next; 
                if(this->head == nullptr)
                    this->tail = nullptr;

                delete oldHead;

                hasMore = this->head != nullptr;
                lock.unlock();
            } else {
                break;
            }
        }
        return !hasMore;
    }

    std::string toString() {
        std::lock_guard<std::mutex> lock(this->mutex);
        return "CallbackQueueLock{ head="+(head != nullptr ? head->toString() : "nullptr")+
                                "; tail="+(tail != nullptr ? tail->toString() : "nullptr")+" }";
    }
};


}

#endif // CALLBACK_QUEUE_LOCK_HPP