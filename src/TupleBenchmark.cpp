#include "./utils/FlowRepresentation.hpp"
#include "./utils/Tuple.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>

using namespace spi;


Tuple createTuple(uint64_t value){
    return Tuple((uint32_t)value);
}

void processTuple(Tuple obj){
    obj.doSomething();
}


Tuple createTupleMove(uint64_t value){
    return std::move(Tuple((uint32_t)value));
}

void processTupleMove(Tuple obj){
    obj.doSomething();
}


Tuple* createTuplePtr(uint64_t value){
    return new Tuple((uint32_t)value);
}

void processTuplePtr(Tuple *obj){
    obj->doSomething();
}


std::unique_ptr<Tuple> createTupleUnique(uint64_t value){
    return std::make_unique<Tuple>((uint32_t)value);
}

void processTupleUnique(std::unique_ptr<Tuple> obj){
    obj->doSomething();
}


std::shared_ptr<Tuple> createTupleShared(uint64_t value){
    return std::make_shared<Tuple>((uint32_t)value);
}

void processTupleShared(std::shared_ptr<Tuple> obj){
    obj->doSomething();
}



int main(){
    const uint64_t ITERATIONS = 100000000;


    auto input = std::make_unique<FlowInput>();
    auto output = input->output();


    // Tuple(&):    ~ 62 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Tuple tup = createTuple(i);
        processTuple(tup); // full copy tuple
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Tuple(&): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Tuple(&&):   ~ 36 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Tuple tup = createTupleMove(i);
        processTuple(std::move(tup)); // full copy tuple
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Tuple(&&): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Tuple*:      ~ 43 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        Tuple *tup = createTuplePtr(i);
        processTuplePtr(tup); // full copy tuple
        delete tup;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Tuple*: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // std::unique_ptr<Tuple>: ~ 5.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::unique_ptr<Tuple> tup = createTupleUnique(i);
        processTupleUnique(std::move(tup)); // full copy tuple
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::unique_ptr<Tuple>: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // std::shared_ptr<Tuple>: ~ 3.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        std::shared_ptr<Tuple> tup = createTupleShared(i);
        processTupleShared(tup); // full copy tuple
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::shared_ptr<Tuple>: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    return 0;
}