#include "./utils/Tuple.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>


class MyClass {
protected:
    int32_t a;

public:

    MyClass(){

    }

    void doSomething(){
        this->a++;
    }
};


int main(){
    const uint64_t ITERATIONS = 50000000;


    // MyClass():               ~ 200.9 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        MyClass myClass;
        myClass.doSomething();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "MyClass(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // new MyClass():           ~ 54.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        MyClass *myClass = new MyClass();
        myClass->doSomething();
        delete myClass;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "new MyClass(): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // unique_ptr<MyClass>:     ~ 9.2 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::unique_ptr<MyClass> myClass = std::make_unique<MyClass>();
        myClass->doSomething();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "unique_ptr<MyClass>: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // shared_ptr<MyClass>:     ~ 4.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::shared_ptr<MyClass> myClass = std::make_shared<MyClass>();
        myClass->doSomething();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "shared_ptr<MyClass>: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}