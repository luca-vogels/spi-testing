#include "./utils/QueueAtomic.hpp"
#include "./utils/QueueLock.hpp"
#include "./utils/QueueMoodyCamel.hpp"
#include "./utils/QueueRing.hpp"
#include "./utils/QueueTwoPartyAtomic.hpp"
#include "./utils/QueueTwoPartyHighContention.hpp"
#include "./utils/QueueTwoPartyNoCritical.hpp"

#include "./utils/Thread.hpp"

#include <chrono>
#include <iostream>

using namespace spi;

int main(){
    const uint64_t ITERATIONS = 50000000; // before 50000000
    const size_t THREADS = 2;

    QueueAtomic<uint64_t> queueAtomic;
    QueueLock<uint64_t> queueLock;
    QueueLockCustom<uint64_t> queueLockCustom;
    moodycamel::ConcurrentQueue<uint64_t> queueMoodyCamel;
    QueueRing<uint64_t> queueRing(20);
    QueueTwoPartyAtomic<uint64_t> queueTwoPartyAtomic;
    QueueTwoPartyHighContention<uint64_t> queueTwoPartyHC;
    QueueTwoPartyNoCritical<uint64_t> queueTwoPartyNoCritical;

    uint64_t result;
    std::vector<Thread*> threads;

    //                                                      RELEASE         |   DEBUG

    // Sequential QueueAtomic push & pop:                   ~ 32.9 Mio/sec  |   ~ 12.7 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueAtomic.push(i);
        while(!queueAtomic.pop(result));
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueAtomic push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueLock push & pop:                     ~ 53.5 Mio/sec  |   ~ 9.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueLock.push(i);
        while(!queueLock.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueLock push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueLockCustom push & pop:               ~ 31.0 Mio/sec  |   ~ 13.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueLockCustom.push(i);
        while(!queueLockCustom.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueLockCustom push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueMoodyCamel push & pop:               ~ 27.6 Mio/sec  |   ~ 6.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueMoodyCamel.enqueue(i);
        while(!queueMoodyCamel.try_dequeue(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueMoodyCamel push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueRing push & pop:                     ~ 31.7 Mio/sec  |   ~ 10.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueRing.push(i);
        while(!queueRing.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueRing push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueTwoPartyAtomic push & pop:           ~ 36.1 Mio/sec  |   ~ 28.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueTwoPartyAtomic.push(i);
        while(!queueTwoPartyAtomic.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueTwoPartyAtomic push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // Sequential QueueTwoPartyHighContention push & pop:   ~ 18.4 Mio/sec
    /*startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueTwoPartyHC.push(i);
        while(!queueTwoPartyHC.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueTwoPartyHighContention push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    */
    

    // Sequential QueueTwoPartyNoCritical push & pop:       ~ 287.9 Mio/sec  |   ~ 84.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        queueTwoPartyNoCritical.push(i);
        while(!queueTwoPartyNoCritical.pop(result));
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential QueueTwoPartyNoCritical push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;
    








    // Parallel QueueAtomic push & pop:                     ~ ???? Mio/sec
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


    // Parallel QueueLock push & pop:                       ~ 6.9 Mio/sec  |   ~ 2.3 Mio/sec
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


    // Parallel QueueLockCustom push & pop:                 ~ 4.7 Mio/sec  |   ~ 3.3 Mio/sec
    for(size_t i=0; i < THREADS; i++){
        threads.push_back(new Thread([&ITERATIONS, &queueLockCustom](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueLockCustom.push(i);
                while(!queueLockCustom.pop(result));
            }
        }));
    }
    startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < THREADS; i++) threads[i]->start();
    for(size_t i=0; i < THREADS; i++) threads[i]->join();
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Parallel QueueLockCustom push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    for(size_t i=0; i < THREADS; i++) delete threads[i];
    threads.clear();


    // Parallel QueueMoodyCamel push & pop:                 ~ 9.1 Mio/sec  |   ~ 5.0 Mio/sec
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


    // Parallel QueueRing push & pop:                       ~ 3.6 Mio/sec  |   ~ 2.8 Mio/sec
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


    // Parallel QueueTwoPartyAtomic push & pop:             ~ 11.1 Mio/sec  |   ~ 9.1 Mio/sec
    if(THREADS == 2){
        threads.push_back(new Thread([&ITERATIONS, &queueTwoPartyAtomic](){
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueTwoPartyAtomic.push(i);
            }
        }));
        threads.push_back(new Thread([&ITERATIONS, &queueTwoPartyAtomic](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                while(!queueTwoPartyAtomic.pop(result));
            }
        }));
        startTime = std::chrono::high_resolution_clock::now();
        for(size_t i=0; i < THREADS; i++) threads[i]->start();
        for(size_t i=0; i < THREADS; i++) threads[i]->join();
        endTime = std::chrono::high_resolution_clock::now();
        std::cout << "Parallel QueueTwoPartyAtomic push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
        for(size_t i=0; i < THREADS; i++) delete threads[i];
        threads.clear();
    } else {
        std::cout << "Parallel QueueTwoPartyAtomic push & pop:   not thread-safe for more than two threads" << std::endl;
    }


    // Parallel QueueTwoPartyHighContention push & pop:     ~
    /*if(THREADS == 2){
        threads.push_back(new Thread([&ITERATIONS, &queueTwoPartyHC](){
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                queueTwoPartyHC.push(i);
            }
        }));
        threads.push_back(new Thread([&ITERATIONS, &queueTwoPartyHC](){
            uint64_t result;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                while(!queueTwoPartyHC.pop(result));
            }
        }));
        startTime = std::chrono::high_resolution_clock::now();
        for(size_t i=0; i < THREADS; i++) threads[i]->start();
        for(size_t i=0; i < THREADS; i++) threads[i]->join();
        endTime = std::chrono::high_resolution_clock::now();
        std::cout << "Parallel QueueTwoPartyHighContention push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
        for(size_t i=0; i < THREADS; i++) delete threads[i];
        threads.clear();
    } else {
        std::cout << "Parallel QueueTwoPartyHighContention push & pop:   not thread-safe for more than two threads" << std::endl;
    }*/


    // Parallel QueueTwoPartyNoCritical push & pop:         ~ 17.3 Mio/sec  |   ~ 37.2 Mio/sec  
    if(THREADS == 2){
        threads.push_back(new Thread([&ITERATIONS, &queueTwoPartyNoCritical](){
            uint64_t i=0;
            for(i=0; i < ITERATIONS/THREADS; i++){
                queueTwoPartyNoCritical.push(i);
            }
        }));
        threads.push_back(new Thread([&ITERATIONS, &queueTwoPartyNoCritical](){
            uint64_t result; uint64_t retries;
            for(uint64_t i=0; i < ITERATIONS/THREADS; i++){
                retries = 1000000000;
                while(!queueTwoPartyNoCritical.pop(result)){
                    if(retries-- == 0){
                        std::cout << "Pop failed at: " << i << " empty=" << queueTwoPartyNoCritical.empty() << " queue=" << queueTwoPartyNoCritical.toString() << std::endl;
                        break;
                    }
                }
            }
        }));
        startTime = std::chrono::high_resolution_clock::now();
        for(size_t i=0; i < THREADS; i++) threads[i]->start();
        for(size_t i=0; i < THREADS; i++) threads[i]->join();
        endTime = std::chrono::high_resolution_clock::now();
        std::cout << "Parallel QueueTwoPartyNoCritical push & pop: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
        for(size_t i=0; i < THREADS; i++) delete threads[i];
        threads.clear();
    } else {
        std::cout << "Parallel QueueTwoPartyNoCritical push & pop:   not thread-safe for more than two threads" << std::endl;
    }
    std::cout << std::endl;

    return 0;
}