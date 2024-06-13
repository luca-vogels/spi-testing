#include <atomic>
#include <cstdint>

class Tuple {
protected:
    uint32_t value;
    //std::atomic<uint32_t> atomicValue;

public:

    Tuple(uint32_t value) : value(value){

    }

    Tuple(const Tuple &other){
        value = other.value;
    }

    Tuple(Tuple &&other){
        value = other.value;
    }


    void doSomething() {
        value++;
    }

};