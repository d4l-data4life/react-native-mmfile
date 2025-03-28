#ifndef AES_INTEL_H
#define AES_INTEL_H

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <random>

#include <wmmintrin.h>
#include <immintrin.h>
#include <stdlib.h>


template <typename CIPHER>
struct CIPHER_BLOCK;

template <const unsigned KEY_LENGTH = 128>
class AES {
    static constexpr unsigned KEY_WORDS = KEY_LENGTH / 32;          // Nk
    static constexpr unsigned NUM_ROUNDS = 6 + KEY_WORDS;           // Nr

public:
    __attribute__((aligned(4)))
    __m128i roundKeys[2 * NUM_ROUNDS];                           // round keys

    template <int rcon>
    static inline __m128i expandKey128(__m128i prevKey) {
        __m128i temp = _mm_aeskeygenassist_si128(prevKey, rcon);
        temp = _mm_shuffle_epi32(temp, _MM_SHUFFLE(3, 3, 3, 3)); // Extract relevant bytes

        __m128i key;
        key = _mm_xor_si128(prevKey, _mm_slli_si128(prevKey, 4));
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
        return _mm_xor_si128(key, temp);
    }

    template <int rcon>
    static inline void expandKey192(__m128i &temp1, __m128i &temp3) {
        __m128i temp2 = _mm_aeskeygenassist_si128(temp3, rcon);
        temp2 = _mm_shuffle_epi32(temp2, 0x55);
        
        __m128i temp4 = _mm_slli_si128(temp1, 0x4);
        temp1 = _mm_xor_si128(temp1, temp4);
        temp4 = _mm_slli_si128(temp4, 0x4);
        temp1 = _mm_xor_si128(temp1, temp4);
        temp4 = _mm_slli_si128(temp4, 0x4);
        temp1 = _mm_xor_si128(temp1, temp4);
        
        temp1 = _mm_xor_si128(temp1, temp2);
        
        temp2 = _mm_shuffle_epi32(temp1, 0xff);
        temp4 = _mm_slli_si128(temp3, 0x4);
        temp3 = _mm_xor_si128(temp3, temp4);
        temp3 = _mm_xor_si128(temp3, temp2);
    }

    template <int rcon>
    inline void expandKey256_part1(__m128i &temp1, __m128i &temp3) {
        __m128i temp2 = _mm_aeskeygenassist_si128(temp3, rcon);
        __m128i temp4;
        temp2 = _mm_shuffle_epi32(temp2, 0xff);
        temp4 = _mm_slli_si128(temp1, 0x4);
        temp1 = _mm_xor_si128(temp1, temp4);
        temp4 = _mm_slli_si128(temp4, 0x4);
        temp1 = _mm_xor_si128(temp1, temp4);
        temp4 = _mm_slli_si128(temp4, 0x4);
        temp1 = _mm_xor_si128(temp1, temp4);
        temp1 = _mm_xor_si128(temp1, temp2);
    }
    
    inline void expandKey256_part2(__m128i &temp1, __m128i &temp3) {
        __m128i temp2, temp4;
        temp4 = _mm_aeskeygenassist_si128(temp1, 0x00);
        temp2 = _mm_shuffle_epi32(temp4, 0xaa);
        temp4 = _mm_slli_si128(temp3, 0x4);
        temp3 = _mm_xor_si128(temp3, temp4);
        temp4 = _mm_slli_si128(temp4, 0x4);
        temp3 = _mm_xor_si128(temp3, temp4);
        temp4 = _mm_slli_si128(temp4, 0x4);
        temp3 = _mm_xor_si128(temp3, temp4);
        temp3 = _mm_xor_si128(temp3, temp2);
    }

    // AES Key Expansion for AES-128
    void setKey(const uint8_t *key) {
        switch (KEY_LENGTH) {
            case 128:
                roundKeys[0]  = _mm_loadu_si128((__m128i*)key);
                roundKeys[1]  = expandKey128<0x01>(roundKeys[0]);
                roundKeys[2]  = expandKey128<0x02>(roundKeys[1]);
                roundKeys[3]  = expandKey128<0x04>(roundKeys[2]);
                roundKeys[4]  = expandKey128<0x08>(roundKeys[3]);
                roundKeys[5]  = expandKey128<0x10>(roundKeys[4]);
                roundKeys[6]  = expandKey128<0x20>(roundKeys[5]);
                roundKeys[7]  = expandKey128<0x40>(roundKeys[6]);
                roundKeys[8]  = expandKey128<0x80>(roundKeys[7]);
                roundKeys[9]  = expandKey128<0x1B>(roundKeys[8]);
                roundKeys[10] = expandKey128<0x36>(roundKeys[9]);
                break;
            
            case 192:
            {
                __m128i temp1 = roundKeys[0] = _mm_loadu_si128((__m128i*)key);
                __m128i temp3 = roundKeys[1] = _mm_loadu_si128((__m128i*)(key + 16));
            
                expandKey192<0x01>(temp1, temp3);
                roundKeys[1] = (__m128i)_mm_shuffle_pd((__m128d)roundKeys[1], (__m128d)temp1, 0);
                roundKeys[2] = (__m128i)_mm_shuffle_pd((__m128d)temp1, (__m128d)temp3, 1);
            
                expandKey192<0x02>(temp1, temp3);
                roundKeys[3] = temp1;
                roundKeys[4] = temp3;
            
                expandKey192<0x04>(temp1, temp3);
                roundKeys[4] = (__m128i)_mm_shuffle_pd((__m128d)roundKeys[4], (__m128d)temp1, 0);
                roundKeys[5] = (__m128i)_mm_shuffle_pd((__m128d)temp1, (__m128d)temp3, 1);
            
                expandKey192<0x08>(temp1, temp3);
                roundKeys[6] = temp1;
                roundKeys[7] = temp3;

                expandKey192<0x10>(temp1, temp3);
                roundKeys[7] = (__m128i)_mm_shuffle_pd((__m128d)roundKeys[7], (__m128d)temp1, 0);
                roundKeys[8] = (__m128i)_mm_shuffle_pd((__m128d)temp1, (__m128d)temp3, 1);
            
                expandKey192<0x20>(temp1, temp3);
                roundKeys[9] = temp1;
                roundKeys[10] = temp3;
            
                expandKey192<0x40>(temp1, temp3);
                roundKeys[10] = (__m128i)_mm_shuffle_pd((__m128d)roundKeys[10], (__m128d)temp1, 0);
                roundKeys[11] = (__m128i)_mm_shuffle_pd((__m128d)temp1, (__m128d)temp3, 1);
            
                expandKey192<0x80>(temp1, temp3);
                roundKeys[12] = temp1;
                break;
            }

            case 256:
            {
                __m128i temp1 = roundKeys[0] = _mm_loadu_si128((__m128i*)key);
                __m128i temp3 = roundKeys[1] = _mm_loadu_si128((__m128i*)(key + 16));
            
                expandKey256_part1<0x01>(temp1, temp3);
                roundKeys[2] = temp1;
                expandKey256_part2(temp1, temp3);
                roundKeys[3] = temp3;
            
                expandKey256_part1<0x02>(temp1, temp3);
                roundKeys[4] = temp1;
                expandKey256_part2(temp1, temp3);
                roundKeys[5] = temp3;
            
                expandKey256_part1<0x04>(temp1, temp3);
                roundKeys[6] = temp1;
                expandKey256_part2(temp1, temp3);
                roundKeys[7] = temp3;
            
                expandKey256_part1<0x08>(temp1, temp3);
                roundKeys[8] = temp1;
                expandKey256_part2(temp1, temp3);
                roundKeys[9] = temp3;
            
                expandKey256_part1<0x10>(temp1, temp3);
                roundKeys[10] = temp1;
                expandKey256_part2(temp1, temp3);
                roundKeys[11] = temp3;
            
                expandKey256_part1<0x20>(temp1, temp3);
                roundKeys[12] = temp1;
                expandKey256_part2(temp1, temp3);
                roundKeys[13] = temp3;
            
                expandKey256_part1<0x40>(temp1, temp3);
                roundKeys[14] = temp1;
                break;
            }
        }
        for (unsigned i = 1; i < NUM_ROUNDS; ++i) {
            roundKeys[NUM_ROUNDS + i] = _mm_aesimc_si128(roundKeys[NUM_ROUNDS - i]);
        }
    }

    inline __m128i encryptBlock(__m128i block) const {
        // AddRoundKey
        block = _mm_xor_si128(block, roundKeys[0]);
        for (unsigned i = 1; i < NUM_ROUNDS; ++i) {
            // Perform SubBytes, ShiftRows, MixColumns and AddRoundKey in one step
            block = _mm_aesenc_si128(block, roundKeys[i]);
        }
        // Final round: SubBytes, ShiftRows and AddRoundKey without MixColumns
        return _mm_aesenclast_si128(block, roundKeys[NUM_ROUNDS]);
    }

    inline void encryptBlock(const uint8_t *input, uint8_t *output) const {
        // Load 16 bytes from input
        __m128i block = _mm_loadu_si128((__m128i*)input);
        
        // Encrypt the block
        block = encryptBlock(block);
        
        // Store the result in output
        _mm_storeu_si128(reinterpret_cast<__m128i*>(output), block);
    }

    inline __m128i decryptBlock(__m128i block) const {
        // AddRoundKey
        block = _mm_xor_si128(block, roundKeys[NUM_ROUNDS]);
        for (unsigned i = NUM_ROUNDS + 1; i < 2 * NUM_ROUNDS; ++i) {
            // Perform SubBytes, ShiftRows, MixColumns and AddRoundKey in one step
            block = _mm_aesdec_si128(block, roundKeys[i]);
        }
        // Final round: SubBytes, ShiftRows and AddRoundKey without MixColumns
        return _mm_aesdeclast_si128(block, roundKeys[0]);
    }

    inline void decryptBlock(const uint8_t *input, uint8_t *output) const {
        // Load 16 bytes from input
        __m128i block = _mm_loadu_si128((__m128i*)input);
        
        // Encrypt the block
        block = decryptBlock(block);
        
        // Store the result in output
        _mm_storeu_si128(reinterpret_cast<__m128i*>(output), block);
    }
};

template <const unsigned KEY_LENGTH>
struct CIPHER_BLOCK<AES<KEY_LENGTH>> {
    using type = __m128i;
    static constexpr unsigned LENGTH_BITS = 128;
    static constexpr unsigned LENGTH_BYTES = LENGTH_BITS / 8;
    static constexpr unsigned LENGTH_WORDS = LENGTH_BYTES / 4;
};


// Increment a 128-bit counter stored in a NEON register
static inline __m128i counterInc(__m128i counter) {
    const __m128i mask = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    counter = _mm_shuffle_epi8(counter, mask);
    const __m128i ONE = _mm_setr_epi32(1, 0, 0, 0);
    const __m128i ZERO = _mm_setzero_si128();

    counter = _mm_add_epi64(counter, ONE);
    __m128i t = _mm_cmpeq_epi64(counter, ZERO);
    t = _mm_and_si128(t, ONE);
    t = _mm_unpacklo_epi64(ZERO, t);
    counter = _mm_add_epi64(counter, t);
    return _mm_shuffle_epi8(counter, mask);
}

static inline void counterInc(uint8_t *counter) {
    _mm_storeu_si128(reinterpret_cast<__m128i*>(counter), 
        counterInc(_mm_loadu_si128(reinterpret_cast<__m128i*>(counter))));
}

static inline __m128i counterAdd(__m128i counter, uint64_t delta) {
    const __m128i mask = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    counter = _mm_shuffle_epi8(counter, mask);

    // Vector to add [1, 0] (increment lower 64 bits)
    __m128i delta64x2 = _mm_set_epi64x(0, delta);

    // Add 1 to the lower 64 bits
    __m128i incremented = _mm_add_epi64(counter, delta64x2);

    // Detect carry by comparing the counter and incremented values
    __m128i carry = _mm_cmpgt_epi64(counter, incremented); // Will set to all ones if carry occurred

    // Add carry to upper 64-bit half
    counter = _mm_sub_epi64(incremented, _mm_srli_si128(carry, 8)); // Shift carry and subtract it

    return _mm_shuffle_epi8(counter, mask);
}


// encrypt an array of data using AES CTR mode, where offset is the byte offset in the stream to start encryption and length the array length in bytes
template <typename BLOCK_CIPHER, size_t BLOCK_OFFSET>
inline void encryptCTRWrapper(BLOCK_CIPHER const &cipher, const uint8_t *iv, const uint8_t *input, uint8_t *output, size_t length, size_t offset = 0) {
    constexpr size_t BLOCK_LENGTH = CIPHER_BLOCK<BLOCK_CIPHER>::LENGTH_BYTES;

    // Add block number to the nonce (IV)
    __m128i counter = counterAdd(_mm_loadu_si128((__m128i*)iv), offset / BLOCK_LENGTH);

    // If input is not aligned to block boundary, encrypt the first block
    __m128i prevEncryptedCounter;
    if (BLOCK_OFFSET != 0) {
        prevEncryptedCounter = cipher.encryptBlock(counter);
        counter = counterInc(counter);
    }

    while (length >= BLOCK_LENGTH) {
        __m128i encryptedCounter = cipher.encryptBlock(counter);
        __m128i mask;
        if (BLOCK_OFFSET != 0) {
            // Perform byte-wise vector rotation (equivalent to vextq_u8 in ARM)
            mask = _mm_or_si128(
                _mm_srli_si128(prevEncryptedCounter, BLOCK_OFFSET), 
                _mm_slli_si128(encryptedCounter, (16 - BLOCK_OFFSET)));
        } else {
            mask = encryptedCounter;
        }
        _mm_storeu_si128(reinterpret_cast<__m128i*>(output), _mm_xor_si128(_mm_loadu_si128((__m128i*)input), mask));
        counter = counterInc(counter);

        input += BLOCK_LENGTH;
        output += BLOCK_LENGTH;
        length -= BLOCK_LENGTH;
        prevEncryptedCounter = encryptedCounter;
    }

    if (length > 0) {
        __m128i encryptedCounter = cipher.encryptBlock(counter);
        __m128i mask;
        if (BLOCK_OFFSET != 0) {
            // Perform byte-wise vector rotation (equivalent to vextq_u8 in ARM)
            mask = _mm_or_si128(
                _mm_srli_si128(prevEncryptedCounter, BLOCK_OFFSET), 
                _mm_slli_si128(encryptedCounter, (16 - BLOCK_OFFSET)));
        } else {
            mask = encryptedCounter;
        }
        __m128i cyphertext = _mm_xor_si128(_mm_loadu_si128((__m128i*)input), mask);

        for (size_t i = 0; i < length; ++i) {
            output[i] = reinterpret_cast<uint8_t*>(&cyphertext)[i];
        }
    }
}

template <typename BLOCK_CIPHER>
inline void encryptCTR(BLOCK_CIPHER const &cipher, const uint8_t *iv, const uint8_t *input, uint8_t *output, size_t length, size_t offset = 0) {
    constexpr size_t BLOCK_LENGTH = CIPHER_BLOCK<BLOCK_CIPHER>::LENGTH_BYTES;

    // Provide explicit template arguments to std::array
    using FunctionPtr = void (*)(const BLOCK_CIPHER &, const uint8_t *, const uint8_t *, uint8_t *, size_t, size_t);

    // Lookup table of function pointers
    static constexpr std::array<FunctionPtr, 16> lookup = {
        encryptCTRWrapper<BLOCK_CIPHER, 0>, encryptCTRWrapper<BLOCK_CIPHER, 1>,
        encryptCTRWrapper<BLOCK_CIPHER, 2>, encryptCTRWrapper<BLOCK_CIPHER, 3>,
        encryptCTRWrapper<BLOCK_CIPHER, 4>, encryptCTRWrapper<BLOCK_CIPHER, 5>,
        encryptCTRWrapper<BLOCK_CIPHER, 6>, encryptCTRWrapper<BLOCK_CIPHER, 7>,
        encryptCTRWrapper<BLOCK_CIPHER, 8>, encryptCTRWrapper<BLOCK_CIPHER, 9>,
        encryptCTRWrapper<BLOCK_CIPHER, 10>, encryptCTRWrapper<BLOCK_CIPHER, 11>,
        encryptCTRWrapper<BLOCK_CIPHER, 12>, encryptCTRWrapper<BLOCK_CIPHER, 13>,
        encryptCTRWrapper<BLOCK_CIPHER, 14>, encryptCTRWrapper<BLOCK_CIPHER, 15>
    };

    lookup[offset % BLOCK_LENGTH](cipher, iv, input, output, length, offset);
}

template <typename BLOCK_CIPHER>
inline void decryptCTR(BLOCK_CIPHER const &cipher, const uint8_t *iv, const uint8_t *input, uint8_t *output, size_t length, size_t offset = 0) {
    encryptCTR(cipher, iv, input, output, length, offset);
}

// XOR-based lightweight data key and IV integrity check
inline uint16_t hashIVAndKey(const uint8_t* iv, const uint8_t* key) {
    // XOR the IV and key
    __m128i kx = _mm_xor_si128(
        _mm_loadu_si128((__m128i*)iv), 
        _mm_loadu_si128((__m128i*)key));

    // Extract 32-bit words
    uint32_t kxArray[4];
    _mm_storeu_si128(reinterpret_cast<__m128i*>(kxArray), kx);

    // XOR the four 32-bit words together
    uint32_t word = kxArray[0] ^ kxArray[1] ^ kxArray[2] ^ kxArray[3];
    return (word & 0xffff) ^ (word >> 16);
}

#endif // AES_INTEL_H
