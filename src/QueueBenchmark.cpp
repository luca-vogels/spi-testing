#include "./utils/QueueAtomic.hpp"
#include "./utils/QueueLock.hpp"
#include "./utils/QueueMoodyCamel.hpp"
#include "./utils/QueueRing.hpp"
#include "./utils/QueueTwoParty.hpp"

#include "./utils/Thread.hpp"

#include <chrono>
#include <iostream>

using namespace spi;

int main(){
    const uint64_t ITERATIONS = 50000000;
    const size_t THREADS = 2;

    QueueAtomic<uint64_t> queueAtomic;
    QueueLock<uint64_t> queueLock;
    moodycamel::ConcurrentQueue<uint64_t> queueMoodyCamel;
    QueueRing<uint64_t> queueRing(20);
    QueueTwoParty<uint64_t> queueTwoParty;

    uint64_t result;
    std::vector<Thread*> threads;


    // Sequential QueueAtomic push & pop:       ~ 12.4 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueAtomic.push(i);
        while(!queueAtomic.pop(result));
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueAtomic push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueLock push & pop:         ~ 13.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueLock.push(i);
        while(!queueLock.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueLock push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueMoodyCamel push & pop:   ~ 6.2 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueMoodyCamel.enqueue(i);
        while(!queueMoodyCamel.try_dequeue(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueMoodyCamel push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueRing push & pop:         ~ 10.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueRing.push(i);
        while(!queueRing.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueRing push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueTwoParty push & pop:     ~ 23.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueTwoParty.push(i);
        while(!queueTwoParty.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueTwoParty push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;
    



    // Parallel QueueAtomic push & pop:         ~ ???? Mio/sec
    /*
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueAtomic](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueAtomic.push(i);
                while(!queueAtomic.pop(result));
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < THREADS; i++) threads[i]->start();
    for(size_t i=0; i < THREADS; i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Parallel QueueAtomic push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    for(size_t i=0; i < THREADS; i++) delete threads[i];
    threads.clear();*/
    std::cout << "Parallel QueueAtomic push & pop:   not thread-safe" << std::endl;


    // Parallel QueueLock push & pop:           ~ 6.4 Mio/sec
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueLock](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueLock.push(i);
                while(!queueLock.pop(result));
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < THREADS; i++) threads[i]->start();
    for(size_t i=0; i < THREADS; i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Parallel QueueLock push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    for(size_t i=0; i < THREADS; i++) delete threads[i];
    threads.clear();


    // Parallel QueueMoodyCamel push & pop:     ~ 9.9 Mio/sec
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueMoodyCamel](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueMoodyCamel.enqueue(i);
                while(!queueMoodyCamel.try_dequeue(result));
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < THREADS; i++) threads[i]->start();
    for(size_t i=0; i < THREADS; i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Parallel QueueMoodyCamel push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    for(size_t i=0; i < THREADS; i++) delete threads[i];
    threads.clear();


    // Parallel QueueRing push & pop:           ~ 5.3 Mio/sec
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueRing](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueRing.push(i);
                while(!queueRing.pop(result));
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < THREADS; i++) threads[i]->start();
    for(size_t i=0; i < THREADS; i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Parallel QueueRing push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    for(size_t i=0; i < THREADS; i++) delete threads[i];
    threads.clear();


    // Parallel QueueTwoParty push & pop:       WAY TO SLOW UNDER HIGH CONTENTION
    if(THREADS == 2){
        threads.push_back(new Thread([&ITERATIONS, &queueTwoParty](){
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueTwoParty.push(i);
            }
        }));
        threads.push_back(new Thread([&ITERATIONS, &queueTwoParty](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                while(!queueTwoParty.pop(result));
            }
        }));
        startTime = std::chrono::high_resolution_clock::now();
        for(size_t i=0; i < THREADS; i++) threads[i]->start();
        for(size_t i=0; i < THREADS; i++) threads[i]->join();
        endTime = std::chrono::high_resolution_clock::now();
        std::cout << "Parallel QueueTwoParty push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
        for(size_t i=0; i < THREADS; i++) delete threads[i];
        threads.clear();
    } else {
        std::cout << "Parallel QueueTwoParty push & pop:   not thread-safe for more than two threads" << std::endl;
    }
    std::cout << std::endl;

    return 0;
}