/**
 * High performance version of a std::atomic.
 * 
 * @file Atomic.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_ATOMIC_HPP
#define SPI_ATOMIC_HPP

#include "Lock.hpp"

#include <atomic>

namespace spi {


/**
 * Abstract class for an atomic variable.
 * 
 * @tparam T Type of value the atomic variable should store.
 */
template<typename T>
class AbstractAtomic {
public:

    virtual ~AbstractAtomic() = default;

    /**
     * Atomically stores a given value 
     * in the view of the thread group A.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to store.
     * @param order Memory order.
     */
    virtual void storeA(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically stores a given value 
     * in the view of the thread group B.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to store.
     * @param order Memory order.
     */
    virtual void storeB(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically loads the current value 
     * in the view of the thread group A.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param order Memory order.
     * @return Current value.
     */
    virtual T loadA(std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically loads the current value 
     * in the view of the thread group B.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param order Memory order.
     * @return Current value.
     */
    virtual T loadB(std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically fetches the current value and adds a given value to it 
     * in the view of the thread group A.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to add.
     * @param order Memory order.
     * @return Previous value.
     */
    virtual T fetchAddA(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically fetches the current value and adds a given value to it 
     * in the view of the thread group B.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to add.
     * @param order Memory order.
     * @return Previous value.
     */
    virtual T fetchAddB(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically fetches the current value and subtracts a given value from it 
     * in the view of the thread group A.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to subtract.
     * @param order Memory order.
     * @return Previous value.
     */
    virtual T fetchSubA(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically fetches the current value and subtracts a given value from it 
     * in the view of the thread group B.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to subtract.
     * @param order Memory order.
     * @return Previous value.
     */
    virtual T fetchSubB(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically exchanges the current value with a given value 
     * in the view of the thread group A.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to store.
     * @param order Memory order.
     * @return Previous value.
     */
    virtual T exchangeA(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically exchanges the current value with a given value 
     * in the view of the thread group B.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param value Value to store.
     * @param order Memory order.
     * @return Previous value.
     */
    virtual T exchangeB(T value, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically compares the current value with a given value and exchanges it if it matches 
     * in the view of the thread group A.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param expected Expected value.
     * @param desired Desired value to set if the current value matches the expected value.
     * @param order Memory order.
     * @return True if the exchange was successful, false otherwise.
     */
    virtual bool compareExchangeA(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) = 0;

    /**
     * Atomically compares the current value with a given value and exchanges it if it matches 
     * in the view of the thread group B.
     * 
     * Both thread groups A and B share the same underlying atomic value.
     * 
     * @param expected Expected value.
     * @param desired Desired value to set if the current value matches the expected value.
     * @param order Memory order.
     * @return True if the exchange was successful, false otherwise.
     */
    virtual bool compareExchangeB(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) = 0;
};



/**
 * Fully thread-safe high performance version of a std::atomic.
 * 
 * @tparam T Type of value the atomic variable should store.
 */
template<typename T>
class AtomicThreadSafe : public AbstractAtomic<T> {
protected:
    std::atomic<T> atomic;

public:

    /**
     * Create a new AtomicThreadSafe without an initial value.
     */
    AtomicThreadSafe() = default;

    /**
     * Create a new AtomicThreadSafe with an initial value.
     * 
     * @param value Initial value.
     */
    AtomicThreadSafe(T value) : atomic(value) {}

    inline void storeA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        atomic.store(value, order = std::memory_order_seq_cst);
    }

    inline void storeB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        atomic.store(value, order = std::memory_order_seq_cst);
    }

    inline T loadA(std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.load(order = std::memory_order_seq_cst);
    }

    inline T loadB(std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.load(order = std::memory_order_seq_cst);
    }

    inline T fetchAddA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.fetch_add(value, order = std::memory_order_seq_cst);
    }

    inline T fetchAddB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.fetch_add(value, order = std::memory_order_seq_cst);
    }

    inline T fetchSubA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.fetch_sub(value, order = std::memory_order_seq_cst);
    }

    inline T fetchSubB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.fetch_sub(value, order = std::memory_order_seq_cst);
    }

    inline T exchangeA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.exchange(value, order = std::memory_order_seq_cst);
    }

    inline T exchangeB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.exchange(value, order = std::memory_order_seq_cst);
    }

    inline bool compareExchangeA(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.compare_exchange_strong(expected, desired, order = std::memory_order_seq_cst);
    }

    inline bool compareExchangeB(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) override {
        return atomic.compare_exchange_strong(expected, desired, order = std::memory_order_seq_cst);
    }

};




/**
 * High performance version of a std::atomic that is optimized for two-party synchronization. 
 * This means that at most one thread accesses the atomic variable as group A 
 * and at most one thread accesses the atomic variable as group B.
 * 
 * @tparam T Type of value the atomic variable should store.
 */
template<typename T>
class AtomicTwoParty : public AbstractAtomic<T> {
protected:
    T value;
    ReadOrWriteAccess lock;

public:

    /**
     * Create a new AtomicTwoParty without an initial value.
     * 
     * At most one thread accesses the atomic variable as group A and 
     * at most one thread accesses the atomic variable as group B.
     * 
     * @param reduceCpuUsage If true the object will use less cpu resources but will be slower.
     */
    AtomicTwoParty(bool reduceCpuUsage) : lock(reduceCpuUsage, false, false) {

    }

    /**
     * Create a new AtomicTwoParty with an initial value.
     * 
     * At most one thread accesses the atomic variable as group A and 
     * at most one thread accesses the atomic variable as group B.
     * 
     * @param reduceCpuUsage If true the object will use less cpu resources but will be slower.
     * @param value Initial value.
     */
    AtomicTwoParty(bool reduceCpuUsage, T value) : value(value), lock(reduceCpuUsage, false, false) {

    }

    inline void storeA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessRead();
        this->value = value;
        lock.releaseRead();
    }

    inline void storeB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessWrite();
        this->value = value;
        lock.releaseWrite();
    }

    inline T loadA(std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessRead();
        T tmp = value;
        lock.releaseRead();
        return tmp;
    }

    inline T loadB(std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessWrite();
        T tmp = value;
        lock.releaseWrite();
        return tmp;
    }

    inline T fetchAddA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessRead();
        T tmp = this->value;
        this->value += value;
        lock.releaseRead();
        return tmp;
    }

    inline T fetchAddB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessWrite();
        T tmp = this->value;
        this->value += value;
        lock.releaseWrite();
        return tmp;
    }

    inline T fetchSubA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessRead();
        T tmp = this->value;
        this->value -= value;
        lock.releaseRead();
        return tmp;
    }

    inline T fetchSubB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessWrite();
        T tmp = this->value;
        this->value -= value;
        lock.releaseWrite();
        return tmp;
    }

    inline T exchangeA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessRead();
        T tmp = this->value;
        this->value = value;
        lock.releaseRead();
        return tmp;
    }

    inline T exchangeB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessWrite();
        T tmp = this->value;
        this->value = value;
        lock.releaseWrite();
        return tmp;
    }

    inline bool compareExchangeA(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessRead();
        if(this->value == expected){
            this->value = desired;
            lock.releaseRead();
            return true;
        }
        lock.releaseRead();
        return false;
    }

    inline bool compareExchangeB(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) override {
        (void)order;
        lock.accessWrite();
        if(this->value == expected){
            this->value = desired;
            lock.releaseWrite();
            return true;
        }
        lock.releaseWrite();
        return false;
    }
};





/**
 * High performance version of a std::atomic that combines the AtomicThreadSafe and AtomicTwoParty into one class.
 * 
 * @tparam T Type of value the atomic variable should store.
 */
template<typename T>
class Atomic : public AbstractAtomic<T> {
protected:
    // AbstractAtomic<T> *atomic; // massive performance drop!
    AtomicThreadSafe<T> atomicThreadSafe;
    AtomicTwoParty<T> atomicTwoParty;
    const bool multithreaded;

public:

    /**
     * Create a new Atomic without an initial value.
     * 
     * @param reduceCpuUsage If true the object will use less cpu resources but will be slower.
     * @param multithreaded Set to true if there are multiple threads in group A or multiple threads in group B (if only one per group set to false).
     */
    Atomic(bool reduceCpuUsage, bool multithreaded) : atomicThreadSafe(), atomicTwoParty(reduceCpuUsage), multithreaded(multithreaded) {
        
    }

    /**
     * Create a new Atomic with an initial value.
     * 
     * @param reduceCpuUsage If true the object will use less cpu resources but will be slower.
     * @param multithreaded Set to true if there are multiple threads in group A or multiple threads in group B (if only one per group set to false).
     * @param value Initial value.
     */
    Atomic(bool reduceCpuUsage, bool multithreaded, T value) : atomicThreadSafe(value), atomicTwoParty(reduceCpuUsage, value), multithreaded(multithreaded) {
        
    }

    inline void storeA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            atomicThreadSafe.storeA(value, order);
        } else {
            atomicTwoParty.storeA(value, order);
        }
    }

    inline void storeB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            atomicThreadSafe.storeB(value, order);
        } else {
            atomicTwoParty.storeB(value, order);
        }
    }

    inline T loadA(std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.loadA(order);
        } else {
            return atomicTwoParty.loadA(order);
        }
    }

    inline T loadB(std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.loadB(order);
        } else {
            return atomicTwoParty.loadB(order);
        }
    }

    inline T fetchAddA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.fetchAddA(value, order);
        } else {
            return atomicTwoParty.fetchAddA(value, order);
        }
    }

    inline T fetchAddB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.fetchAddB(value, order);
        } else {
            return atomicTwoParty.fetchAddB(value, order);
        }
    }

    inline T fetchSubA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.fetchSubA(value, order);
        } else {
            return atomicTwoParty.fetchSubA(value, order);
        }
    }

    inline T fetchSubB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.fetchSubB(value, order);
        } else {
            return atomicTwoParty.fetchSubB(value, order);
        }
    }

    inline T exchangeA(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.exchangeA(value, order);
        } else {
            return atomicTwoParty.exchangeA(value, order);
        }
    }

    inline T exchangeB(T value, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.exchangeB(value, order);
        } else {
            return atomicTwoParty.exchangeB(value, order);
        }
    }

    inline bool compareExchangeA(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.compareExchangeA(expected, desired, order);
        } else {
            return atomicTwoParty.compareExchangeA(expected, desired, order);
        }
    }

    inline bool compareExchangeB(T expected, T desired, std::memory_order order = std::memory_order_seq_cst) override {
        if(multithreaded){
            return atomicThreadSafe.compareExchangeB(expected, desired, order);
        } else {
            return atomicTwoParty.compareExchangeB(expected, desired, order);
        }
    }
};


}
#endif // SPI_ATOMIC_HPP