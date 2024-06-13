/**
 * HardwareUtils allows fetching hardware capabilities.
 * 
 * @file HardwareUtils.cpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_HARDWARE_UTILS_HPP
#define SPI_HARDWARE_UTILS_HPP

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <ifaddrs.h>
#include <mutex>
#include <sched.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>


#ifdef NUMA
#include <numa.h>
#include <numaif.h>
#endif

namespace spi {

typedef pid_t ThreadID;


class HardwareUtils {
protected:

    #ifdef CUDA
    static void handleCUDAError(cudaError_t err){
        if(err != cudaSuccess){
            std::string errStr = cudaGetErrorString(err);
            throw std::runtime_error("CUDA error: " + errStr);
        }
    }
    #endif

    inline static std::unordered_map<size_t, size_t> CPU_TO_NUMA; // mapping of CPU ID to NUMA node
    inline static std::unordered_map<size_t, std::vector<size_t>> NUMA_TO_CPU; // mapping of NUMA node to CPUs

    static void loadMappings(){
        if(!CPU_TO_NUMA.empty()) return;

        #ifdef NUMA
        int cpuCount = getCpuCoreCount();
        for(int cpu=0; cpu < cpuCount; cpu++){
            int numa = numa_node_of_cpu(cpu);
            if(numa < 0) continue;
            CPU_TO_NUMA[cpu] = numa;
            NUMA_TO_CPU[numa].push_back(cpu);
        }
        #endif
    }

public:

    /**
     * Returns amount of CPU cores this worker has.
     * 
     * @return int Amount of CPU cores or -1 if could not be determined
     */
    static int getCpuCoreCount(){
        return std::thread::hardware_concurrency();
    }

    /**
     * Returns the amount of NUMA nodes.
     * 
     * @return size_t Amount of NUMA nodes or zero if could not be determined.
     */
    static size_t getNumaNodeCount(){
        loadMappings();
        return NUMA_TO_CPU.size();
    }

    /**
     * Returns the CPU IDs of a given NUMA node.
     * 
     * @param numaNode NUMA node of which the CPUs should be returned.
     * @return std::vector<size_t> Vector containing the IDs of the CPUs (can be empty)
     */
    static std::vector<size_t> getCpusOfNumaNode(int numaNode){
        loadMappings();
        auto cpus = NUMA_TO_CPU.find(numaNode);
        return cpus != NUMA_TO_CPU.end() ? std::vector<size_t>(cpus->second) : std::vector<size_t>();
    }

    /**
     * Returns the NUMA node of a given CPU core.
     * 
     * @param cpu CPU core the NUMA node should be returned of.
     * @return int NUMA node or -1 if could not be determined.
     */
    static int getNumaNode(int cpu){
        #ifdef NUMA
        if(cpu < 0) return -1;
        loadMappings();
        auto numa = CPU_TO_NUMA.find(cpu);
        return numa != CPU_TO_NUMA.end() ? (int)numa->second : -1;
        #else
        (void)cpu; // prevent unused parameter warning
        #endif
        return -1;
    }

    /**
     * Returns the ID of the CPU the calling thread is currently running at. 
     * 
     * @return int ID of CPU or -1 if could not get detected
     */
    static int currentCPU(){
        return sched_getcpu();
    }

    /**
     * Returns the numa node of the calling thread.
     * 
     * @return int ID of the numa node or -1 if could not get detected
     */
    static int currentNumaNode(){
        return getNumaNode(currentCPU());
    }

    /**
     * Returns the unique ID of the calling thread assigned by the OS.
     * ID are only unique within this worker instance (other workers even on the same machine may have the same ID).
     * 
     * @return ThreadID ID of the thread.
     */
    static ThreadID currentThreadID(){
        return (ThreadID) syscall(SYS_gettid); // same logic in Thread.hpp execute() !
    }

};

}
#endif // SPI_HARDWARE_UTILS_HPP