#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>

#include <arpa/inet.h>
#include <cstdlib>
#include <endian.h>


const uint64_t ITERATIONS = 5000000000;



static inline int32_t getInt32Naive(uint8_t *arr){
    // big endian
    return (arr[0] << 24) | (arr[1] << 16) | (arr[2] << 8) | arr[3];
}

static inline void setInt32Naive(uint8_t *arr, int32_t value){
    // big endian
    arr[0] = (uint8_t)(value >> 24);
    arr[1] = (uint8_t)(value >> 16);
    arr[2] = (uint8_t)(value >> 8);
    arr[3] = (uint8_t)value;
}



static inline int32_t getInt32Optimized1(uint8_t *arr){
    uint16_t detect = 1;
    if(*(uint8_t*)&detect == 1){ // check if first byte contains the 1
        // little endian -> convert to big endian
        return (arr[0] << 24) | (arr[1] << 16) | (arr[2] << 8) | arr[3];
    } else {
        // big endian -> host already big endian so no conversion needed
        return *(int32_t*)arr;
    } 
}

static inline void setInt32Optimized1(uint8_t *arr, int32_t value){
    uint16_t detect = 1;
    if(*(uint8_t*)&detect == 1){ // check if first byte contains the 1
        // little endian -> convert to big endian
        arr[0] = (uint8_t)(value >> 24);
        arr[1] = (uint8_t)(value >> 16);
        arr[2] = (uint8_t)(value >> 8);
        arr[3] = (uint8_t)value;
    } else {
        // big endian -> host already big endian so no conversion needed
        *(int32_t*)arr = value;
    }
}



static inline int32_t getInt32Optimized2(uint8_t *arr){
    uint16_t detect = 1;
    if(*(uint8_t*)&detect == 1){ // check if first byte contains the 1
        // little endian -> convert to big endian
        uint8_t b0 = arr[0];
        uint8_t b1 = arr[1];
        arr[0] = arr[3];
        arr[1] = arr[2];
        arr[2] = b1;
        arr[3] = b0;
    }
    return *(int32_t*)arr;
}

static inline void setInt32Optimized2(uint8_t *arr, int32_t value){
    *(int32_t*)arr = value;
    uint16_t detect = 1;
    if(*(uint8_t*)&detect == 1){ // check if first byte contains the 1
        // little endian -> convert to big endian
        uint8_t b0 = arr[0];
        uint8_t b1 = arr[1];
        arr[0] = arr[3];
        arr[1] = arr[2];
        arr[2] = b1;
        arr[3] = b0;
    }
}


// WINNER !!!
static inline int32_t getInt32HTONL(uint8_t *arr){
    return ntohl(*(int32_t*)arr);
}

static inline void setInt32HTONL(uint8_t *arr, int32_t value){
    *(int32_t*)arr = htonl(value);
}


static inline int32_t getInt32LE(uint8_t *arr){
    return le32toh(*(int32_t*)arr);
}

static inline void setInt32LE(uint8_t *arr, int32_t value){
    *(int32_t*)arr = htole32(value);
}

static inline int32_t getInt32BE(uint8_t *arr){
    return be32toh(*(int32_t*)arr);
}

static inline void setInt32BE(uint8_t *arr, int32_t value){
    *(int32_t*)arr = htobe32(value);
}





int main(){
    const uint64_t HALF_ITERATIONS = ITERATIONS >> 1;
    uint8_t *arr = (uint8_t*)malloc(4);

    uint16_t detect = 1;
    if(*(uint8_t*)&detect == 1){
        std::cout << "Little endian" << std::endl;
    } else {
        std::cout << "Big endian" << std::endl;
    }


    //                  RELEASE         |   DEBUG


    // Naive():         ~ 523 Mio/sec   |   ~ 108 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        int32_t value = (int32_t)(i - HALF_ITERATIONS);
        setInt32Naive(arr, value);
        int32_t result = getInt32Naive(arr);
        if(value != result){
            std::cout << "Naive error with " << value << " -> " << result << std::endl;
        }
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Naive: " << (ITERATIONS * 1000000) / std::max(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count(), (int64_t)1) << "/s" << std::endl;


    // Optimized1():    ~ 525 Mio/sec   |   ~ 98 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        int32_t value = (int32_t)(i - HALF_ITERATIONS);    
        setInt32Optimized1(arr, value);
        int32_t result = getInt32Optimized1(arr);
        if(value != result){
            std::cout << "Optimized1 error with " << value << " -> " << result << std::endl;
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Optimized1(): " << (ITERATIONS * 1000000) / std::max(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count(), (int64_t)1) << "/s" << std::endl;


    // Optimized2():    ~ 3123 Mio/sec  |   ~ 65 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        int32_t value = (int32_t)(i - HALF_ITERATIONS);    
        setInt32Optimized2(arr, value);
        int32_t result = getInt32Optimized2(arr);
        if(value != result){
            std::cout << "Optimized2 error with " << value << " -> " << result << std::endl;
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "Optimized2(): " << (ITERATIONS * 1000000) / std::max(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count(), (int64_t)1) << "/s" << std::endl;


    // HtoN():          ~ 3135 Mio/sec  |   ~ 144 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        int32_t value = (int32_t)(i - HALF_ITERATIONS);    
        setInt32HTONL(arr, value);
        int32_t result = getInt32HTONL(arr);
        if(value != result){
            std::cout << "HtoN error with " << value << " -> " << result << std::endl;
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "HtoN(): " << (ITERATIONS * 1000000) / std::max(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count(), (int64_t)1) << "/s" << std::endl;


    // HtoLE():         ~ ???? Mio/sec  |   ~ 91 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        int32_t value = (int32_t)(i - HALF_ITERATIONS);    
        setInt32LE(arr, value);
        int32_t result = getInt32LE(arr);
        if(value != result){
            std::cout << "HtoLE error with " << value << " -> " << result << std::endl;
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "HtoLE(): " << (ITERATIONS * 1000000) / std::max(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count(), (int64_t)1) << "/s" << std::endl;


    // HtoBE():         ~ 1570 Mio/sec  |   ~ 92 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        int32_t value = (int32_t)(i - HALF_ITERATIONS);    
        setInt32BE(arr, value);
        int32_t result = getInt32BE(arr);
        if(value != result){
            std::cout << "HtoBE error with " << value << " -> " << result << std::endl;
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "HtoBE(): " << (ITERATIONS * 1000000) / std::max(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count(), (int64_t)1) << "/s" << std::endl;


    free(arr);
    return 0;
}
