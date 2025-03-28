#ifndef AES_H
#define AES_H

#if defined(__aarch64__) && defined(__ARM_NEON)
    #include <arm_neon.h>
    #define USE_ARM_AES
    #include "AES_arm.h"
#elif defined(__AES__) || defined(__AVX__)
    #include <wmmintrin.h>
    #define USE_INTEL_AES
    #include "AES_intel.h"
#else
    #error "No AES intrinsics available on this platform"
#endif

#include <random>

inline void fillRandom(uint8_t *buffer, size_t length) {
    static std::random_device rd;
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_int_distribution<uint8_t> dist(0, 255);

    for (size_t i = 0; i < length; ++i) {
        buffer[i] = dist(gen);
    }
}

inline void fillRandom(std::string &buffer) {
    fillRandom(reinterpret_cast<uint8_t*>(const_cast<char *>(buffer.data())), buffer.size());
}

#endif // AES_H