#include "./utils/QueueAtomic.hpp"
#include "./utils/QueueLock.hpp"
#include "./utils/QueueMoodyCamel.hpp"
#include "./utils/QueueRing.hpp"

#include "./utils/Thread.hpp"

#include <iostream>

using namespace spi;

int main(){
    const uint64_t ITERATIONS = 50000000;
    const size_t THREADS = 4;

    QueueAtomic<uint64_t> queueAtomic;
    QueueLock<uint64_t> queueLock;
    moodycamel::ConcurrentQueue<uint64_t> queueMoodyCamel;
    QueueRing<uint64_t> queueRing(20);

    uint64_t result;
    std::vector<Thread*> threads;


    // Sequential QueueAtomic push & pop:    ~ 13.1 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueAtomic.push(i);
        while(!queueAtomic.pop(result));
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueAtomic push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueLock push & pop:    ~ 13.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueLock.push(i);
        while(!queueLock.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueLock push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueMoodyCamel push & pop:    ~ 6.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueMoodyCamel.enqueue(i);
        while(!queueMoodyCamel.try_dequeue(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueMoodyCamel push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueRing push & pop:    ~ 10.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueRing.push(i);
        while(!queueRing.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueRing push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    



    // Parallel QueueAtomic push & pop:    ~ ???? Mio/sec
    /*
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueAtomic](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/4; i++){
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


    // Parallel QueueLock push & pop:    ~ 1.9 Mio/sec
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueLock](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/4; i++){
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


    // Parallel QueueMoodyCamel push & pop:    ~ 2.4 Mio/sec
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueMoodyCamel](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/4; i++){
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


    // Parallel QueueRing push & pop:    ~ 1.5 Mio/sec
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueRing](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/4; i++){
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

    return 0;
}