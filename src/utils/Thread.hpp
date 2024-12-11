/**
 * Create threads that are platform independent and allow for optimizations.
 * 
 * @file Thread.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_THREAD_HPP
#define SPI_THREAD_HPP

#include "./HardwareUtils.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unistd.h>
#include <vector>

namespace spi {
class HardwareUtils; // defined in HardwareUtils.hpp

typedef std::function<void()> Task;

class Cancellable {
protected:
    bool cancelled = false;

public:

    bool isCancelled(){
        return this->cancelled;
    }

    /**
     * Cancels the running task if it hasn't been executed yet.
     */
    void cancel(){
        this->cancelled = true;
    }
};



class ThreadState {
public:
    enum Type : uint8_t {

        /** Thread object got created but execution has not been started yet. */
        CREATED,

        /** Executing is starting (thread sets CPU affinity but hasn't started executing task yet). */
        STARTING,

        /** Thread is executing task. */
        RUNNING,

        /** Thread successfully completed execution of task. */
        TERMINATED,

        /** Thread died because an occurred during execution of task. */
        ERROR,

        /** Thread got killed externally (e.g. by calling kill() method). */
        INTERRUPTED,

        /** Thread got detached which causes it to continue running in background. */
        DETACHED
    };
private:
    Type type = CREATED;

public:
    ThreadState() = default;

    constexpr ThreadState(Type type) : type(type) { }

    constexpr operator Type() const { return type; } // switch(type)
    explicit operator bool() const = delete; // prevent if(type)
    bool operator==(ThreadState o) const { return o.type == this->type; }
    bool operator==(Type o) const { return o == this->type; }
    bool operator!=(ThreadState o) const { return o.type != this->type; }
    bool operator!=(Type o) const { return o != this->type; }
    size_t operator()(const ThreadState &o) const { return o.type; }

    std::string toString() const {
        switch(this->type){
            case CREATED: return "CREATED";
            case STARTING: return "STARTING";
            case RUNNING: return "RUNNING";
            case TERMINATED: return "TERMINATED";
            case ERROR: return "ERROR";
            case INTERRUPTED: return "INTERRUPTED";
            case DETACHED: return "DETACHED";
        } return "UNDEFINED";
    }

    uint8_t toId() const {
        return this->type;
    }

    static const ThreadState getById(uint8_t id){
        switch(id){
            case CREATED: return ThreadState::CREATED;
            case STARTING: return ThreadState::STARTING;
            case RUNNING: return ThreadState::RUNNING;
            case TERMINATED: return ThreadState::TERMINATED;
            case ERROR: return ThreadState::ERROR;
            case INTERRUPTED: return ThreadState::INTERRUPTED;
            case DETACHED: return ThreadState::DETACHED;
        }
        return ThreadState::TERMINATED;
    }
};


/**
 * Spawns a single thread that gets destructed after execution.
 */
class Thread {
protected:
    inline static std::mutex mThreadIdToPID;
    inline static std::unordered_map<std::thread::id, pid_t> threadIdToOsId;

    struct ThreadData {
        std::thread *thr = nullptr;
        Task task;
        std::atomic<ThreadState> state{ThreadState::STARTING};
        bool doDetach = false;

        std::vector<size_t> cpus;
        std::thread::id threadId; // thread id returned by thread API
        ThreadID tid = -1; // thread id assigned by OS

        std::atomic<size_t> references{2}; // one from Thread object and one from std::thread

        ThreadData(Task task, bool doDetach) : task(task), doDetach(doDetach) {
            this->thr = new std::thread([this]{ this->execute(); });
            if(doDetach) this->detach();
        }

        ~ThreadData(){
            this->_invalidate();
            ThreadState expected = ThreadState::RUNNING;
            this->state.compare_exchange_weak(expected, ThreadState::ERROR);
            this->kill();
        }

        void _dereference(){
            if(this->references.fetch_sub(1) == 1)
                delete this;
        }

        void _invalidate(){
            if(this->tid == -1) return;
            std::lock_guard<std::mutex> l(Thread::mThreadIdToPID);
            Thread::threadIdToOsId.erase(this->threadId);
            this->tid = -1;
        }

        void kill(){
            if(this->thr != nullptr && this->tid >= 0){
                //Logger::error("Killing a thread is considered bad practice", __FILE__, __LINE__);
                delete this->thr;
            }
            this->thr = nullptr;
            ThreadState expected = ThreadState::RUNNING;
            this->state.compare_exchange_weak(expected, ThreadState::INTERRUPTED);
        }

        void detach(){
            this->doDetach = true;
            if(this->thr != nullptr && this->thr->joinable()){
                this->thr->detach();
                delete this->thr;
                this->thr = nullptr;
                this->state.store(ThreadState::DETACHED);
            }
        }

        void join(){
            if(this->thr != nullptr && this->thr->joinable())
                this->thr->join();
        }

        void applyCpuRestrictions(bool checkIfValid){
            if(checkIfValid && (this->tid < 0 || this->thr == nullptr || this->state.load() != ThreadState::RUNNING)) return; // no restrictions
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            if(this->cpus.empty()){
                for(int cpu=0; cpu < HardwareUtils::getCpuCoreCount(); cpu++)
                    CPU_SET(cpu, &cpuset); // reset by setting all cpus
            } else {
                for(const size_t &cpu : this->cpus)
                    CPU_SET(cpu, &cpuset); // specific CPU
            }

            /*if(sched_setaffinity(this->tid, sizeof(cpuset), &cpuset) < 0)
                Logger::warn("Could not set CPU affinity using sched_setaffinity() for threadId="+std::to_string(this->tid), __FILE__, __LINE__);*/
        }

        void execute(){
            this->tid = HardwareUtils::currentThreadID();
            this->threadId = std::this_thread::get_id();
            {
                std::lock_guard<std::mutex> l(Thread::mThreadIdToPID);
                Thread::threadIdToOsId[this->threadId] = this->tid; // same logic in HardwareUtils.hpp currentThreadID() !
            }
            this->applyCpuRestrictions(false);
            ThreadState expected = ThreadState::STARTING;
            this->state.compare_exchange_weak(expected, ThreadState::RUNNING);

            this->task();
            
            this->state.store(ThreadState::TERMINATED);
            this->_invalidate();
            this->_dereference();
        }
    };

    ThreadData *current = nullptr; // shared because std::thread still needs access when detached
    Task defaultTask;
    std::vector<size_t> defaultCpus;
    bool defaultDoDetach = false;

public:
    
    /**
     * Sets up a new thread but does not run it immediately
     * 
     * @param fn Function that should be executed
     * @param args Arguments to pass to the function call
     */
    template <class Fn, class... Args> explicit
    Thread(Fn&& fn, Args&&... args){
        this->defaultTask = std::bind(fn, args...);
    }

    /** Will terminate the thread immediately if still running.
     * Use .join() before to wait until the thread has finished execution.
     * Use .detach() before to keep thread running in the background.  
     */
    ~Thread(){
        if(this->current != nullptr)
            this->current->kill();
    }

    /**
     * Compares thread ids
     */
    inline bool operator==(const Thread &thr){
        return (this->current == nullptr && thr.current == nullptr) || (this->current != nullptr && thr.current != nullptr && this->current->tid == thr.current->tid);
    }

    /**
     * Returns the current state of the thread.
     * 
     * @return ThreadState State of the thread.
     */
    ThreadState getState() const {
        const ThreadState created = ThreadState::CREATED;
        return this->current != nullptr ? this->current->state.load() : created;
    }

    /**
     * Returns the thread id of this thread assigned by the OS.
     * 
     * @return pid_t OS thread id
     * @throws std::runtime_error if thread is not running
     */
    pid_t getPID(){
        auto entry = (this->current != nullptr && this->current->thr != nullptr) ? threadIdToOsId.find(this->current->threadId) : threadIdToOsId.end();
        if(entry == threadIdToOsId.end()){
            std::string error = "Thread that isn't running does not have a PID";
            //Logger::error(error, __FILE__, __LINE__);
            throw std::runtime_error(error);
        }
        return entry->second;
    }

    /**
     * Returns a unique ID of this thread.
     * ID is only unique for this worker (threads on other workers may have the same id).
     * 
     * @return ThreadID ID of this thread
     */
    ThreadID getID(){
        return (this->current != nullptr && this->current->thr != nullptr) ? this->current->tid : -1;
    }

    /**
     * Lets the thread run on a specific CPU.
     * Can be set regardless of if thread is running or not.
     * Overwrites setNumaNode();
     * 
     * @param cpu CPU the thread should run on (if negative no longer bound to specific CPU)
     */
    void setCPU(int cpu){
        this->defaultCpus.clear();
        if(cpu >= 0) this->defaultCpus.push_back(cpu);
        if(this->current != nullptr){
            this->current->cpus = this->defaultCpus; // copy
            this->current->applyCpuRestrictions(true); // only applies if thread is actually running
        }
    }

    /**
     * Lets the thread run on specific CPUs.
     * Can be set regardless of if thread is running or not.
     * Overwrites setNumaNode();
     * 
     * @param cpus CPUs the thread should run on (if empty no longer bound to specific CPUs)
     */
    void setCPUs(const std::vector<size_t> cpus){
        this->defaultCpus = cpus; // copy
        if(this->current != nullptr){
            this->current->cpus = this->defaultCpus; // copy
            this->current->applyCpuRestrictions(true); // only applies if thread is actually running
        }
    }

    /**
     * Lets the thread run on a specific NUMA node.
     * Can be set regardless of if thread is running or not.
     * Overwrites setCPU();
     * 
     * @param numaNode NUMA node the thread should run on (if negative no longer bound to specific CPUs)
     */
    void setNumaNode(int numaNode){
        if(numaNode >= 0){
            this->setCPUs(HardwareUtils::getCpusOfNumaNode(numaNode));
        } else {
            this->defaultCpus.clear();
            if(this->current != nullptr){
                this->current->cpus.clear();
                this->current->applyCpuRestrictions(true); // only applies if thread is actually running
            }
        }
    }

    
    /**
     * Starts the execution of this thread
     */
    void start(){
        if(this->current != nullptr && this->current->thr != nullptr) return;
        if(this->current != nullptr) this->current->_dereference();
        this->current = new ThreadData(this->defaultTask, this->defaultDoDetach);
    }

    
    /**
     * Stopps execution of the thread immediately.
     */
    /*void kill(){                      // killing a running thread is considered bad practice
        if(this->current != nullptr)
            this->current->kill();
    }*/


    /**
     * Blocks the thread calling this function until this thread 
     * has finished its execution.
     */
    void join(){
        if(this->current != nullptr)
            this->current->join();
    }


    /**
     * Detaches the thread from the calling thread.
     * The thread will continue running in the background 
     * even if this thread object is destructed.
     * However the thread cannot be joined nor controlled anymore.
     */
    void detach(){
        this->defaultDoDetach = true;
        if(this->current != nullptr)
            this->current->detach();
    }


    /**
     * Returns if the thread is still running
     * 
     * @return true If thread is running
     * @return false If thread has finished execution
     */
    bool isRunning(){
        return this->current != nullptr && this->current->state.load() == ThreadState::RUNNING;
    }


    /**
     * Runs the given function after a given amount of milliseconds 
     * without blocking the calling thread.
     * 
     * @param milliseconds Amount of milliseconds after which the function should be executed.
     */
    static std::shared_ptr<Cancellable> runAfter(uint64_t milliseconds, std::function<void()> fn){
        std::shared_ptr<Cancellable> cancellable = std::make_shared<Cancellable>();
        Thread thr([milliseconds, fn, cancellable]{
            if(milliseconds > 0) Thread::sleepMs(milliseconds);
            if(cancellable->isCancelled()) return;
            fn();
        });
        thr.start();
        thr.detach(); // so thread object can be deleted while actual thread keeps running
        return cancellable;
    }

    /**
     * Lets the calling thread sleep for a given amount of seconds.
     * 
     * @param seconds Amount of seconds to sleep
     */
    static void sleepSec(uint64_t seconds){
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
    }

    /**
     * Lets the calling thread sleep for a given amount of milliseconds.
     * 
     * @param milliseconds Amount of milliseconds to sleep
     */
    static void sleepMs(uint64_t milliseconds){
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    /**
     * Lets the calling thread sleep for a given amount of microseconds.
     * 
     * @param microseconds Amount of microseconds to sleep
     */
    static void sleepUs(uint64_t microseconds){
        std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
    }

};



/**
 * Creates a pool of worker threads to which 
 * an unlimited amount of tasks can be given.
 */
class ThreadPool {
protected:
    struct WorkerThread {
        Thread* thr;

        inline bool operator ==(const WorkerThread &w){
            return this->thr == w.thr;
        }
    };

    int minThreadsAlive;
    int maxThreads;
    std::chrono::milliseconds keepAliveMs;
    int numaNode;

    std::vector<WorkerThread> workers;
    std::mutex mWorkerThreads;
    std::atomic<size_t> staleWorkerThreads{0};

    std::queue<Task> tasks;
    std::mutex mTasks;
    std::condition_variable cvTasks; // used to signal that a new task has been added
    std::condition_variable cvIdle; // used to signal that all tasks have been executed

    /**
     * Function that workers execute
     */
    void workerExecute(WorkerThread me){
        this->staleWorkerThreads.fetch_add(1);
        while(true){

            // locks tasks
            std::unique_lock<std::mutex> lTasks(this->mTasks);

            if(this->tasks.empty()) this->cvIdle.notify_all(); // signal that all tasks have been executed
            
            // wait for new tasks
            if(this->cvTasks.wait_for(lTasks, this->keepAliveMs) != std::cv_status::no_timeout){

                // no timeout, task maybe ready
                if(!this->tasks.empty()){
                    this->staleWorkerThreads.fetch_add(-1); // signal that this worker is working (no longer stale)
                    Task task = this->tasks.front(); // retrieve first element
                    this->tasks.pop(); // remove first element
                    lTasks.unlock(); // unlock tasks

                    // execute tasks
                    /* TODO REDO try {
                        task();
                    } catch (const std::exception &ex){
                        Logger::error(ex, "Exception occured while executing task", __FILE__, __LINE__);
                    }*/
                    task(); // TODO REMOVE

                    this->staleWorkerThreads.fetch_add(1); // signal that this worker is stale

                } else lTasks.unlock();

            } else {
                // timeout reached, check if this worker can get destructed
                std::unique_lock<std::mutex> lWorkerThreads(this->mWorkerThreads);

                // check if this worker can get destructed
                if((int)this->workers.size() > this->minThreadsAlive){
                    // destruct this worker
                    this->staleWorkerThreads.fetch_add(-1);
                    this->workers.erase(std::remove(this->workers.begin(), this->workers.end(), me), this->workers.end());
                    delete me.thr;
                    break;
                }
            }
        }
    }

    /**
     * Decides if new workers need to be spawned
     */
    void ensureWorkerThreads(){
        std::unique_lock<std::mutex> lWorkerThreads(this->mWorkerThreads);
        if((this->maxThreads > 0 && (int)this->workers.size() >= this->maxThreads) || this->staleWorkerThreads.load() != 0)
            return; // do not spawn new threads if:  no more threads can be spawned OR there are stale workers

        WorkerThread worker;
        worker.thr = new Thread([this, worker]{ this->workerExecute(worker); });
        this->workers.push_back(worker);
        worker.thr->start();
    }

public:

    /**
     * Creates a thread pool which a variable amount of worker threads.
     * 
     * @param minThreadsAlive Minimum amount of threads to always keep alive (can be zero)
     * @param maxThreads Maximum amount of threads to run simultaneously.
     *                   If 0 then infinitely many threads can run in parallel.
     *                   If -1 the maximum hardware concurrency wil be set as limit or 
     *                      if numaNode is defined then the hardware concurrency of the NUMA node is set.
     * @param keepAliveMs Milliseconds how long to keep stale threads alive (if zero then instantly destructed if no more tasks)
     * @param numaNode NUMA node to assign the worker threads to. If -1 no NUMA optimization.
     */
    ThreadPool(int minThreadsAlive=0, int maxThreads=-1, size_t keepAliveMs=5000, int numaNode=-1){
        this->staleWorkerThreads.store(0);
        this->minThreadsAlive = minThreadsAlive;
        this->maxThreads = maxThreads < 0 ? HardwareUtils::getCpuCoreCount() : maxThreads;
        this->keepAliveMs = std::chrono::milliseconds(keepAliveMs);
        this->numaNode = numaNode;
    }

    ~ThreadPool(){
        cancelAllTasks();
    }

    /**
     * Returns the amount of currently active worker threads.
     * 
     * @return size_t Amount of running worker threads.
     */
    size_t getCurrentThreadCount(){
        return this->workers.size();
    }

    /**
     * Removes pending tasks and closes all threads.
     * Does not block calling thread (use join afterwards to block).
     * 
     * @param immediately If true all threads are interruped and destructed immediately (may cause errors and undefined behavior).
     *                      If false (default) currently running tasks will be finished before threads are destructed.
     * 
     */
    void cancelAllTasks(bool immediately = false){
        std::unique_lock<std::mutex> lTasks(this->mTasks);
        while(!this->tasks.empty()) this->tasks.pop();

        if(immediately){
            // interrupt all threads (may cause errors and undefined behavior)
            std::unique_lock<std::mutex> lWorkerThreads(this->mWorkerThreads);
            for(WorkerThread &worker : this->workers)
                delete worker.thr;
            this->workers.clear();
            this->staleWorkerThreads.store(0);
        }
    }

    /**
     * Blocks calling thread until all tasks have been executed and
     * therefore thread pool is idle.
     */
    void join(){
        std::unique_lock<std::mutex> lTasks(this->mTasks);
        while(!this->tasks.empty())
            this->cvIdle.wait(lTasks);
    }

    /**
     * Submits a tasks to the thread pool to be executed.
     * 
     * @tparam Fn Function that should be called
     * @tparam Args Arguments to pass to the function
     * @param fn Function that should be called
     * @param args Arguments to pass to the function
     */
    template <class Fn, class... Args>
    void submitTask(Fn&& fn, Args&& ...args){
        std::unique_lock<std::mutex> lTasks(this->mTasks);
        this->tasks.push(std::bind(fn, args...));
        lTasks.unlock(); // unlock tasks

        this->ensureWorkerThreads();
        this->cvTasks.notify_all();
    }

};


}
#endif // SPI_THREAD_HPP