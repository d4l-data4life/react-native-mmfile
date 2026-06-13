#ifndef AES_H
#define AES_H

// Select the AES backend.
//
// The ARM Cryptography Extensions (the AES instructions) exist in both the
// AArch64 and AArch32 execution states of ARMv8-A. Some 64-bit ARMv8 CPUs ship
// in devices that run a 32-bit OS (e.g. the Samsung Galaxy A13 / Helio G80);
// there the app is built for armeabi-v7a / AArch32, yet the AES instructions are
// still available. We therefore enable the ARM backend for 32-bit ARM too, as
// long as the toolchain reports the crypto/AES feature (built with
// -march=armv8-a+crypto). See AES_arm.h for the AArch32-specific adaptations.
#if (defined(__aarch64__) && defined(__ARM_NEON)) || \
    (defined(__arm__) && defined(__ARM_NEON) && (defined(__ARM_FEATURE_AES) || defined(__ARM_FEATURE_CRYPTO)))
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

// Returns true if the CPU actually provides the AES instructions the cipher was
// compiled with. On arm64 every shipping Android/iOS device has them, but a
// 32-bit (armeabi-v7a) build can also land on an older ARMv7 CPU that predates
// the ARMv8 Cryptography Extensions, so we check at runtime and fail with a clear
// error instead of crashing with SIGILL on the first AES instruction.
#if defined(USE_ARM_AES) && defined(__ANDROID__)
    #include <sys/auxv.h>
    #include <asm/hwcap.h>
    inline bool cpuHasAES() {
        static const bool supported = []() {
    #if defined(__aarch64__)
            return (getauxval(AT_HWCAP) & HWCAP_AES) != 0;
    #else
            return (getauxval(AT_HWCAP2) & HWCAP2_AES) != 0;
    #endif
        }();
        return supported;
    }
#else
    // iOS arm64 always has the extensions; the Intel backend is chosen at compile
    // time from the build flags, so there is nothing to verify at runtime.
    inline bool cpuHasAES() { return true; }
#endif

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