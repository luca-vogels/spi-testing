#include <chrono>
#include <cstdint>
#include <cstdlib> // malloc
#include <cstring> // memcpy
#include <iostream>

int main(){
    const uint64_t ITERATIONS_SMALL = 500000000;
    const uint64_t ITERATIONS_MEDIUM = 50000000;
    const uint64_t ITERATIONS_LARGE = 50000;

    const uint64_t SMALL_BUF_SIZE = 4; // 4B
    const uint64_t MEDIUM_BUF_SIZE = 1024; // 1KB
    const uint64_t LARGE_BUF_SIZE = 1024*1024; // 1MB

    uint8_t* smallBuf1 = (uint8_t*)std::malloc(SMALL_BUF_SIZE);
    uint8_t* smallBuf2 = (uint8_t*)std::malloc(SMALL_BUF_SIZE);
    uint8_t* mediumBuf1 = (uint8_t*)std::malloc(MEDIUM_BUF_SIZE);
    uint8_t* mediumBuf2 = (uint8_t*)std::malloc(MEDIUM_BUF_SIZE);
    uint8_t* largeBuf1 = (uint8_t*)std::malloc(LARGE_BUF_SIZE);
    uint8_t* largeBuf2 = (uint8_t*)std::malloc(LARGE_BUF_SIZE);

    for(uint64_t i=0; i < SMALL_BUF_SIZE; i++){
        smallBuf1[i] = (uint8_t)i;
        smallBuf2[i] = 0;
    }
    for(uint64_t i=0; i < MEDIUM_BUF_SIZE; i++){
        mediumBuf1[i] = (uint8_t)i;
        mediumBuf2[i] = 0;
    }
    for(uint64_t i=0; i < LARGE_BUF_SIZE; i++){
        largeBuf1[i] = (uint8_t)i;
        largeBuf2[i] = 0;
    }



    //                              RELEASE         |   DEBUG

    // copy-edit-copy small:        ~ 305 Mio/sec   |   ~ 139 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_SMALL; i++){
        std::memcpy(smallBuf2, smallBuf1, SMALL_BUF_SIZE);
        volatile uint32_t tmp = *(uint32_t*)(smallBuf2 + 0);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(smallBuf2 + 0) = tmp;
        std::memcpy(smallBuf1, smallBuf2, SMALL_BUF_SIZE);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "copy-edit-copy small: " << (ITERATIONS_SMALL * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // copy-edit-copy medium:       ~ 24 Mio/sec    |   ~ 58 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEDIUM; i++){
        std::memcpy(mediumBuf2, mediumBuf1, MEDIUM_BUF_SIZE);
        volatile uint32_t tmp = *(uint32_t*)(mediumBuf2 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(mediumBuf2 + 6) = tmp;
        std::memcpy(mediumBuf1, mediumBuf2, MEDIUM_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "copy-edit-copy medium: " << (ITERATIONS_MEDIUM * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // copy-edit-copy large:        ~ 3583 /sec     |   ~ 3509 /sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_LARGE; i++){
        std::memcpy(largeBuf2, largeBuf1, LARGE_BUF_SIZE);
        volatile uint32_t tmp = *(uint32_t*)(largeBuf2 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(largeBuf2 + 6) = tmp;
        std::memcpy(largeBuf1, largeBuf2, LARGE_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "copy-edit-copy large: " << (ITERATIONS_LARGE * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;








    // zero-copy-edit small:        ~ 312 Mio/sec   |   ~ 217 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_SMALL; i++){
        volatile uint32_t tmp = *(uint32_t*)(smallBuf1 + 0);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(smallBuf1 + 0) = tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "zero-copy-edit small: " << (ITERATIONS_SMALL * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // zero-copy-edit medium:       ~ 312 Mio/sec   |   ~ 193 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEDIUM; i++){
        volatile uint32_t tmp = *(uint32_t*)(mediumBuf1 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(mediumBuf1 + 6) = tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "zero-copy-edit medium: " << (ITERATIONS_MEDIUM * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // zero-copy-edit large:        ~ 279 Mio/sec |   ~ 213 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_LARGE; i++){
        volatile uint32_t tmp = *(uint32_t*)(largeBuf1 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(largeBuf1 + 6) = tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "zero-copy-edit large: " << (ITERATIONS_LARGE * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;



    free(smallBuf1);
    free(smallBuf2);
    free(mediumBuf1);
    free(mediumBuf2);
    free(largeBuf1);
    free(largeBuf2);
    return 0;
}