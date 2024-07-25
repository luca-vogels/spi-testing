/**
 * Concurrent queue specifically designed for use with callbacks.
 * 
 * @file CallbackQueueLock.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef CALLBACK_QUEUE_LOCK_HPP
#define CALLBACK_QUEUE_LOCK_HPP

#include <mutex>
#include <string>

namespace spi {


/**
 * Callback queue that stores callback functions 
 * and executes them one after another.
 * 
 * Fully thread-safe.
 * 
 * @tparam Callback Type of the callback function that returns a bool to indicate if execution was successful.
 * @tparam CallbackArgs Arguments that will be passed to the callback functions.
 */
template<typename Callback, typename... CallbackArgs>
class CallbackQueueThreadSafe {
protected:

    class Entry {
    public:
        Callback callback;
        Entry* next = nullptr;

        Entry(Callback callback) : callback(callback) {}

        std::string toString() const {
            return "{id="+std::to_string((uint16_t)(uint64_t)(void**)this)+
                        "; cb="+(callback!=nullptr ? "true" :  "nullptr")+
                        "}"+(next!=nullptr ? "->"+next->toString() : "");
        }
    };

    std::mutex mutex;
    Entry* head = nullptr;
    Entry* tail = nullptr;

    Entry* recycleHead = nullptr;
    Entry* recycleTail = nullptr;

public:

    ~CallbackQueueThreadSafe(){
        cancelAll();

        Entry* current = recycleHead;
        while(current != nullptr) {
            Entry* next = current->next;
            delete current;
            current = next;
        }
    }


    void cancelAll(){
        std::lock_guard<std::mutex> lock(mutex);
        while(this->head != nullptr){
            Entry* oldHead = this->head;
            this->head = oldHead->next;
            
            oldHead->next = nullptr;
            if(this->recycleTail != nullptr){
                this->recycleTail->next = oldHead;
            } else {
                this->recycleHead = oldHead;
            }
            this->recycleTail = oldHead;
        }
        this->tail = nullptr;
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
        Entry* entry;
        std::lock_guard<std::mutex> lock(mutex);
        if(this->recycleHead != nullptr){
            entry = this->recycleHead;
            this->recycleHead = entry->next;
            entry->next = nullptr;
            entry->callback = callback;
            if(this->recycleHead == nullptr){
                this->recycleTail = nullptr;
            }
        } else {
            entry = new Entry(callback);
        }

        if(this->tail != nullptr){
            this->tail->next = entry;
        } else {
            this->head = entry;
        }
        this->tail = entry;
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
        std::lock_guard<std::mutex> lock(mutex);
        while(this->head != nullptr){
            if(this->head->callback(args...)) {
                Entry *oldHead = this->head;
                this->head = oldHead->next;
                
                oldHead->next = nullptr;
                if(this->recycleTail != nullptr){
                    this->recycleTail->next = oldHead;
                } else {
                    this->recycleHead = oldHead;
                }
                this->recycleTail = oldHead;

            } else {
                return false;
            }
        }
        this->tail = nullptr;
        return true;
    }

    std::string toString() const {
        Entry *tail = this->tail.load();
        return "CallbackQueueThreadSafe{ head="+(head != nullptr ? head->toString() : "nullptr")+
                                "; tail="+(tail != nullptr ? tail->toString() : "nullptr")+" }";
    }
};


}

#endif // CALLBACK_QUEUE_LOCK_HPP