#include "./utils/Atomic.hpp"
#include "./utils/Thread.hpp"

#include <atomic>
#include <cmath> // round
#include <iostream>
#include <stdexcept>

using namespace spi;



void runFetchAddTest(AbstractAtomic<int32_t>* atomic){
    const uint32_t ITERATIONS = 10000000; // 10000000

    Thread thr1([atomic, ITERATIONS]{
        for(uint32_t i=0; i < ITERATIONS; i++){
            atomic->fetchAddA(1);
        }
    });
    thr1.start();

    for(uint32_t i=0; i < ITERATIONS; i++){
        atomic->fetchAddB(1);
    }
    thr1.join();

    if(atomic->loadA() != ITERATIONS*2)
        throw std::runtime_error("Atomic value should be "+std::to_string(ITERATIONS*2)+" but it is "+std::to_string(atomic->loadA()));
    std::cout << "Completed FetchAddTest successfully" << std::endl;
}


int main(){
    AbstractAtomic<int32_t>* atomicTwoParty = new AtomicTwoParty<int32_t>(false, 0);
    //AbstractAtomic<int32_t>* atomicTwoParty = new AtomicThreadSafe<int32_t>(0);

    runFetchAddTest(atomicTwoParty);



    return 0;
}