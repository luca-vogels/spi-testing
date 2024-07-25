/**
 * Thread-safe queue implementation that uses a linked list scheme.
 * Best performance under low contention.
 * 
 * @file Queue.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_QUEUE_TWOPARTY_HPP
#define SPI_QUEUE_TWOPARTY_HPP

#include <atomic>

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
class QueueTwoPartyAtomic {
protected:

    struct Node {
        T data;
        Node* next = nullptr;
    };

    Node* head; // always keep one element at head
    Node* tail;
    std::atomic<size_t> count{1}; // one actually represents zero! (used to optimize pop)

    Node* recycleHead = nullptr; // optimization to recycle Node instances (used by push)
    Node* recycleTail = nullptr; // optimization to recycle Node instances (used by pop)
    std::atomic<size_t> recycleCount{1}; // one actually represents zero! (used to optimize pop)

public:

    QueueTwoPartyAtomic(){
        Node* dummy = new Node();
        head = dummy;
        tail = dummy;
        Node* dummy2 = new Node();
        recycleHead = dummy2;
        recycleTail = dummy2;
    }

    ~QueueTwoPartyAtomic() {
        cancelAll();
    }

    void cancelAll() noexcept {
        while(head != nullptr){
            Node* oldHead = head;
            head = oldHead->next;
            delete oldHead;
        }
        while(recycleHead != nullptr){
            Node* oldHead = recycleHead;
            recycleHead = oldHead->next;
            delete oldHead;
        }
    }

    void push(T data) noexcept {
        Node* newNode;
        if(recycleCount.fetch_sub(1) == 1){
            recycleCount.fetch_add(1); // restore count because empty
            newNode = new Node();
        } else {
            newNode = recycleHead;
            recycleHead = recycleHead->next;
        }

        tail->next = newNode;
        tail->data = data;
        tail = tail->next;
        count.fetch_add(1);
    }

    bool pop(T& data) noexcept {
        if(count.fetch_sub(1) == 1){
            count.fetch_add(1); // restore count because empty
            std::this_thread::yield(); // actually performs better with this!
            return false;
        }
        Node* oldHead = head;
        head = head->next;

        data = oldHead->data;
        oldHead->next = nullptr;
        recycleTail->next = oldHead;
        recycleTail = oldHead;
        recycleCount.fetch_add(1);
        return true;
    }

    bool empty() noexcept {
        return count.load() == 1; // one actually represents zero! (because of dummy node)
    }

};



}

#endif // SPI_QUEUE_TWOPARTY_HPP