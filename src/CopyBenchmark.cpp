#include "./utils/MetricsUtils.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib> // malloc
#include <cstring> // memcpy
#include <iostream>

using namespace spi;



// COPY-EDIT-COPY vs. ZERO-COPY-EDIT


int main(){
    const uint64_t ITERATIONS_SMALL = 500000000;
    const uint64_t ITERATIONS_MEDIUM = 50000000;
    const uint64_t ITERATIONS_LARGE = 500000;
    const uint64_t ITERATIONS_MEGA_LARGE = 5000;

    const uint64_t SMALL_BUF_SIZE = 32; // 32B
    const uint64_t MEDIUM_BUF_SIZE = 2048; // 2KB
    const uint64_t LARGE_BUF_SIZE = 1024*128; // 128KB
    const uint64_t MEGA_LARGE_BUF_SIZE = 1024*1024*8; // 8MB

    uint8_t* smallBuf1 = (uint8_t*)std::malloc(SMALL_BUF_SIZE);
    uint8_t* smallBuf2 = (uint8_t*)std::malloc(SMALL_BUF_SIZE);
    uint8_t* mediumBuf1 = (uint8_t*)std::malloc(MEDIUM_BUF_SIZE);
    uint8_t* mediumBuf2 = (uint8_t*)std::malloc(MEDIUM_BUF_SIZE);
    uint8_t* largeBuf1 = (uint8_t*)std::malloc(LARGE_BUF_SIZE);
    uint8_t* largeBuf2 = (uint8_t*)std::malloc(LARGE_BUF_SIZE);
    uint8_t* megaLargeBuf1 = (uint8_t*)std::malloc(MEGA_LARGE_BUF_SIZE);
    uint8_t* megaLargeBuf2 = (uint8_t*)std::malloc(MEGA_LARGE_BUF_SIZE);

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
    for(uint64_t i=0; i < MEGA_LARGE_BUF_SIZE; i++){
        megaLargeBuf1[i] = (uint8_t)i;
        megaLargeBuf2[i] = 0;
    }



    //                              RELEASE         |   DEBUG               RELEASE     |   DEBUG



    // copy small:                                  |   ~ 513 Mio/sec        ~ 17 GB/s   |   ~ 17 GB/s           
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_SMALL; i++){
        std::memcpy(smallBuf2, smallBuf1, SMALL_BUF_SIZE);
        std::memcpy(smallBuf1, smallBuf2, SMALL_BUF_SIZE);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    int64_t iterationsPerSec = 2 * ITERATIONS_SMALL * 1000000 / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "copy small: \t" << MetricsUtils::bytesPerSecToString(iterationsPerSec * SMALL_BUF_SIZE) << " | \t" << iterationsPerSec << "op/s" << std::endl;

    // copy medium:                 ~ 56 Mio/sec    ~ 116 GB/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEDIUM; i++){
        std::memcpy(mediumBuf2, mediumBuf1, MEDIUM_BUF_SIZE);
        std::memcpy(mediumBuf1, mediumBuf2, MEDIUM_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    iterationsPerSec = 2 * ITERATIONS_MEDIUM * 1000000 / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "copy medium: \t" << MetricsUtils::bytesPerSecToString(iterationsPerSec * MEDIUM_BUF_SIZE) << " | \t" << iterationsPerSec << "op/s" << std::endl;

    // copy large:                  ~ 464 Kilo/sec  |   ~61 GB/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_LARGE; i++){
        std::memcpy(largeBuf2, largeBuf1, LARGE_BUF_SIZE);
        std::memcpy(largeBuf1, largeBuf2, LARGE_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    iterationsPerSec = 2 * ITERATIONS_LARGE * 1000000 / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "copy large: \t" << MetricsUtils::bytesPerSecToString(iterationsPerSec * LARGE_BUF_SIZE) << " | \t" << iterationsPerSec << "op/s" << std::endl;

    // copy mega:                   ~ 1416 /sec     |   ~ 12 GB/s
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEGA_LARGE; i++){
        std::memcpy(megaLargeBuf2, megaLargeBuf1, MEGA_LARGE_BUF_SIZE);
        std::memcpy(megaLargeBuf1, megaLargeBuf2, MEGA_LARGE_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    iterationsPerSec = 2 * ITERATIONS_MEGA_LARGE * 1000000 / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "copy mega: \t" << MetricsUtils::bytesPerSecToString(iterationsPerSec * MEGA_LARGE_BUF_SIZE) << " | \t" << iterationsPerSec << "op/s" << std::endl;
    std::cout << std::endl;





    // copy-edit-copy small:        ~ 305 Mio/sec   |   ~ 98 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_SMALL; i++){
        std::memcpy(smallBuf2, smallBuf1, SMALL_BUF_SIZE);
        volatile uint32_t tmp = *(uint32_t*)(smallBuf2 + 0);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(smallBuf2 + 0) = tmp;
        std::memcpy(smallBuf1, smallBuf2, SMALL_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "copy-edit-copy small: \t" << (ITERATIONS_SMALL * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // copy-edit-copy medium:       ~ 24 Mio/sec    |   ~ 26 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEDIUM; i++){
        std::memcpy(mediumBuf2, mediumBuf1, MEDIUM_BUF_SIZE);
        volatile uint32_t tmp = *(uint32_t*)(mediumBuf2 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(mediumBuf2 + 6) = tmp;
        std::memcpy(mediumBuf1, mediumBuf2, MEDIUM_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "copy-edit-copy medium: \t" << (ITERATIONS_MEDIUM * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // copy-edit-copy large:        ~ 3583 /sec     |   ~ 231 Kilo/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_LARGE; i++){
        std::memcpy(largeBuf2, largeBuf1, LARGE_BUF_SIZE);
        volatile uint32_t tmp = *(uint32_t*)(largeBuf2 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(largeBuf2 + 6) = tmp;
        std::memcpy(largeBuf1, largeBuf2, LARGE_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "copy-edit-copy large: \t" << (ITERATIONS_LARGE * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    
    // copy-edit-copy mega:         ~ 0.1 /sec      |   ~ 720 /sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEGA_LARGE; i++){
        std::memcpy(megaLargeBuf2, megaLargeBuf1, MEGA_LARGE_BUF_SIZE);
        volatile uint32_t tmp = *(uint32_t*)(megaLargeBuf2 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(megaLargeBuf2 + 6) = tmp;
        std::memcpy(megaLargeBuf1, megaLargeBuf2, MEGA_LARGE_BUF_SIZE);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "copy-edit-copy mega: \t" << (ITERATIONS_MEGA_LARGE * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;







    // zero-copy-edit small:        ~ 312 Mio/sec   |   ~ 288 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_SMALL; i++){
        volatile uint32_t tmp = *(uint32_t*)(smallBuf1 + 0);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(smallBuf1 + 0) = tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "zero-copy-edit small: " << (ITERATIONS_SMALL * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // zero-copy-edit medium:       ~ 312 Mio/sec   |   ~ 277 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEDIUM; i++){
        volatile uint32_t tmp = *(uint32_t*)(mediumBuf1 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(mediumBuf1 + 6) = tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "zero-copy-edit medium: " << (ITERATIONS_MEDIUM * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // zero-copy-edit large:        ~ 279 Mio/sec |   ~ 276 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_LARGE; i++){
        volatile uint32_t tmp = *(uint32_t*)(largeBuf1 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(largeBuf1 + 6) = tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "zero-copy-edit large: " << (ITERATIONS_LARGE * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;

    // zero-copy-edit mega:         ~ 0.1 /sec      |   ~ 263 /sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS_MEGA_LARGE; i++){
        volatile uint32_t tmp = *(uint32_t*)(megaLargeBuf1 + 6);
        tmp = tmp + (uint32_t)i;
        *(uint32_t*)(megaLargeBuf1 + 6) = tmp;
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "zero-copy-edit mega: " << (ITERATIONS_MEGA_LARGE * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;



    free(smallBuf1);
    free(smallBuf2);
    free(mediumBuf1);
    free(mediumBuf2);
    free(largeBuf1);
    free(largeBuf2);
    free(megaLargeBuf1);
    free(megaLargeBuf2);
    return 0;
}