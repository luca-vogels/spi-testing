#include <chrono>
#include <cstdint>
#include <iostream>



class Base {
public:
    volatile uint64_t a;

    void doBase(){
        a = a + a + 1;
    }

    virtual void doVirtual(){
        a = a + a + 2;
    }

    virtual void doPureVirtual() = 0;
};

class Derived : public Base {
public:

    void doDerived(){
        a = a + a + 3;
    }

    void doVirtual() override {
        a = a + a + 4;
    }

    void doPureVirtual() override {
        a = a + a + 5;
    }
};




const uint64_t ITERATIONS = 1000000000;


int main(){
    Derived obj;



    //                                          RELEASE         |   DEBUG

    // doBase():            ~ 409 Mio/s     |   ~ 191 Mio/s
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj.doBase();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "doBase(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // doDerived():         ~ 412 Mio/s     |   ~ 190 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj.doDerived();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "doDerived(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // doVirtual():         ~ 404 Mio/s     |   ~ 191 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj.doVirtual();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "doVirtual(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // doPureVirtual():     ~ 383 Mio/s     |   ~ 191 Mio/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj.doPureVirtual();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "doPureVirtual(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    return 0;
}