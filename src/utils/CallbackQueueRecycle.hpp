/**
 * Concurrent non-blocking queue specifically designed for use with callbacks.
 * 
 * @file CallbackQueueRecycle.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef CALLBACK_QUEUE_RECYCLE_HPP
#define CALLBACK_QUEUE_RECYCLE_HPP

#include <atomic>


namespace spi {


using QueueableCallback = bool(*)();


class CallbackQueueRecycle {
protected:

    struct Entry {
        QueueableCallback callback;
        Entry* next = nullptr;
    };

    // queue
    Entry* head = nullptr;
    std::atomic<Entry*> tail{nullptr};
    std::atomic<bool> executing{false};

    // recycling of entries
    std::atomic<Entry*> poolHead{nullptr};
    std::atomic<Entry*> poolTail{nullptr};

public:

    ~CallbackQueueRecycle(){
        Entry* curr = this->head;
        while(curr != nullptr){
            Entry* next = curr->next;
            delete curr;
            curr = next;
        }
        curr = this->poolHead.load();
        while(curr != nullptr){
            Entry* next = curr->next;
            delete curr;
            curr = next;
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
    void push(QueueableCallback callback){
        
        // get entry from pool or create new one
        Entry *entry = nullptr, *tmp = nullptr;
        do {
            entry = this->poolHead.load();
            if(entry != nullptr){
                // set poolHead to poolHead->next if still the same
                tmp = entry; // needed because compare_exchange_strong() assigns current value to it
                if(this->poolHead.compare_exchange_strong(tmp, entry->next)){
                    // successfully got head from pool, update poolTail
                    entry->callback = callback;
                    entry->next = nullptr;
                    break;
                }
            } else {
                entry = new Entry();
                entry->callback = callback;
                break;
            }
        } while(true);
        
        // add entry to queue
        Entry* oldTail = this->tail.exchange(entry);
        if (oldTail == nullptr) {
            this->head = entry;
            return;
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
                
                // recyle oldHead
                oldHead->callback = nullptr;
                oldHead->next = nullptr;
                Entry* oldPoolTail = this->poolTail.exchange(oldHead);
                if(oldPoolTail != nullptr){
                    oldPoolTail->next = oldHead;
                } else {
                    this->poolHead.store(oldHead);
                }

                hasMore = this->head != nullptr;
            } else {
                break;
            }
        }
        executing.store(false);
        return !hasMore;
    }

};


}

#endif // CALLBACK_QUEUE_RECYCLE_HPP