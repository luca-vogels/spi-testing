#include "./utils/CountingLock.hpp"
#include "./utils/Thread.hpp"

#include <atomic>
#include <cmath> // round
#include <iostream>
#include <stdexcept>

using namespace spi;


void executeSimpleTest(AbstractCountingLock* lock){
    std::atomic<int32_t> accessCounter{0};
    Thread thr1([&accessCounter, lock]{
        for(uint32_t i=0; i < 10; i++){
            lock->acquire();
            accessCounter.fetch_add(1);
            uint32_t check = accessCounter.load();
            if(check != 1) throw std::runtime_error("Counter should be 1 but it is "+std::to_string(check));

            Thread::sleepMs(40);

            accessCounter.fetch_sub(1);
            check = accessCounter.load();
            if(check != 0) throw std::runtime_error("Counter should be 0 but it is "+std::to_string(check));
            lock->release();

            Thread::sleepMs(40);
        }
    });
    thr1.start();

    for(uint32_t i=0; i < 10; i++){
        lock->acquire();
        uint32_t check = accessCounter.fetch_add(1) + 1;
        if(check != 1) throw std::runtime_error("Counter should be 1 but it is "+std::to_string(check));

        Thread::sleepMs(20);

        check = accessCounter.fetch_sub(1) - 1;
        if(check != 0) throw std::runtime_error("Counter should be 0 but it is "+std::to_string(check));
        lock->release();

        Thread::sleepMs(20);
    }

    thr1.join();
}


void runSimpleTest(){
    std::cout << "Starting Simple Test: CountingLockCompSwap" << std::endl;
    AbstractCountingLock* lockCompSwap = new CountingLockCompSwap(1, false, false);
    executeSimpleTest(lockCompSwap);
    delete lockCompSwap;
    std::cout << "Completed Simple Test: CountingLockCompSwap" << std::endl;
    std::cout << std::endl;


    std::cout << "Starting Simple Test: CountingLockFetch" << std::endl;
    AbstractCountingLock* lockFetch = new CountingLockFetch(1, false, true);
    executeSimpleTest(lockFetch);
    delete lockFetch;
    std::cout << "Completed Simple Test: CountingLockFetch" << std::endl;
    std::cout << std::endl;


    std::cout << "Starting Simple Test: CountingLockSemaphore" << std::endl;
    AbstractCountingLock* lockSemaphore = new CountingLockSemaphore(1);
    executeSimpleTest(lockSemaphore);
    delete lockSemaphore;
    std::cout << "Completed Simple Test: CountingLockSemaphore" << std::endl;
    std::cout << std::endl;
}





void executeMultiThreadedTest(AbstractCountingLock* lock, const int32_t MAX, const uint32_t THREADS, const uint32_t ITERATIONS){
    std::atomic<int32_t> accessCounter{0};
    std::vector<Thread*> threads;
    std::vector<uint32_t> progress; // for each thread
    bool done = false;

    for(uint32_t threadId=0; threadId < THREADS; threadId++){
        progress.push_back(0);
        threads.push_back(new Thread([&accessCounter, &progress, threadId, lock, MAX, ITERATIONS]{
            for(uint32_t i=0; i < ITERATIONS; i++){
                lock->acquire();
                int32_t check = accessCounter.fetch_add(1) + 1;
                if(check > MAX){
                    std::cout << "[ERROR] Counter should be smaller than "+std::to_string(MAX)+" but it is "+std::to_string(check) << std::endl;
                    // throw std::runtime_error("Counter should be smaller than "+std::to_string(MAX)+" but it is "+std::to_string(check));
                } else if(check < 0){
                    std::cout << "[ERROR] Counter should be at least 0 but it is "+std::to_string(check) << std::endl;
                    // throw std::runtime_error("Counter should be at least 0 but it is "+std::to_string(check));
                }

                Thread::sleepMs(1+threadId);

                check = accessCounter.fetch_sub(1) - 1;
                if(check > MAX){
                    std::cout << "[ERROR] Counter should be smaller than "+std::to_string(MAX)+" but it is "+std::to_string(check) << std::endl;
                    // throw std::runtime_error("Counter should be smaller than "+std::to_string(MAX)+" but it is "+std::to_string(check));
                } else if(check < 0){
                    std::cout << "[ERROR] Counter should be at least 0 but it is "+std::to_string(check) << std::endl;
                    // throw std::runtime_error("Counter should be at least 0 but it is "+std::to_string(check));
                }
                lock->release();

                progress[threadId]++;
                Thread::sleepMs(1+threadId);
            }
        }));
    }
    for(Thread* thr : threads) thr->start();

    Thread monitor([&progress, &done, THREADS, ITERATIONS]{
        const uint32_t REPORT_INTERVAL = 1000; // 1s
        const uint32_t PAUSE_TIME = 10;

        std::vector<uint32_t> lastProgress = progress;
        std::vector<bool> isStuck(progress.size(), false);

        uint32_t msPassed = 0;
        bool lastMessage = false;
        while(!done || !lastMessage){
            Thread::sleepMs(PAUSE_TIME);
            msPassed += PAUSE_TIME;

            if(msPassed >= REPORT_INTERVAL){
                msPassed = 0;
                uint32_t total = 0, stuck = 0;
                for(size_t threadId = 0; threadId < progress.size(); threadId++){
                    total += progress[threadId];
                    if(lastProgress[threadId] == progress[threadId] && progress[threadId] < ITERATIONS){
                        if(!isStuck[threadId]){
                            isStuck[threadId] = true;
                            stuck++;
                            /*std::cout << "- thread " << (threadId+1) << " is stuck at " << progress[threadId] << "/"
                                        << ITERATIONS << " (" << std::round(progress[threadId]*100.0/(double)ITERATIONS) << "%)." << std::endl;*/
                        }
                    } else if(isStuck[threadId]){
                        isStuck[threadId] = false;
                        //std::cout << "- thread " << (threadId+1) << " resumed execution." << std::endl;
                    }
                }
                lastProgress = progress;
                lastMessage = done;

                std::cout << "Progress: " << total << "/" << progress.size() * ITERATIONS << " ("
                            << std::round(total*100.0/(double)progress.size()/(double)ITERATIONS) << "%)  |  Stuck Threads: "
                            << stuck << "/" << THREADS << " (" << std::floor(stuck*100.0/(double)THREADS) << "%)" << std::endl;
            }
        }
    });
    monitor.start();

    for(Thread* thr : threads){
        thr->join();
        delete thr;
    }
    done = true;
    monitor.join();
}




void runHighContentionTest(){
    const uint32_t MAX = 10;
    const uint32_t THREADS = 100;
    const uint32_t ITERATIONS = 20;

    std::cout << "Starting High Contention Test: CountingLockCompSwap" << std::endl;
    AbstractCountingLock* lockCompSwap = new CountingLockCompSwap(MAX, false, true);
    executeMultiThreadedTest(lockCompSwap, (int32_t)MAX, THREADS, ITERATIONS);
    delete lockCompSwap;
    std::cout << "Completed High Contention Test: CountingLockCompSwap" << std::endl;
    std::cout << std::endl;


    std::cout << "Starting High Contention Test: CountingLockFetch" << std::endl;
    AbstractCountingLock* lockFetch = new CountingLockFetch(MAX, false, true);
    executeMultiThreadedTest(lockFetch, (int32_t)MAX, THREADS, ITERATIONS);
    delete lockFetch;
    std::cout << "Completed High Contention Test: CountingLockFetch" << std::endl;
    std::cout << std::endl;


    std::cout << "Starting High Contention Test: CountingLockSemaphore" << std::endl;
    AbstractCountingLock* lockSemaphore= new CountingLockSemaphore(MAX);
    executeMultiThreadedTest(lockSemaphore, (int32_t)MAX, THREADS, ITERATIONS);
    delete lockSemaphore;
    std::cout << "Completed High Contention Test: CountingLockSemaphore" << std::endl;
    std::cout << std::endl;
}



int main(){

    runSimpleTest();

    runHighContentionTest();

    return 0;
}