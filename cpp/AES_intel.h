#ifndef AES_INTEL_H
#define AES_INTEL_H

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <random>

#include <wmmintrin.h>
#include <immintrin.h>
#include <stdlib.h>

// SubWord function applies the AES S-box substitution to a 4-byte word
__attribute__((always_inline)) inline uint32_t SubWord(uint32_t word) {
    // Broadcast the 32-bit word to a 128-bit register
    __m128i bytes = _mm_set1_epi32(word);
    
    // Perform AES SubBytes transformation using AESENC with a zero key
    __m128i subBytes = _mm_aesenc_si128(bytes, _mm_setzero_si128());

    // Extract the lower 32 bits as the substituted word
    return _mm_cvtsi128_si32(subBytes);
}

// Fix endianness for 32-bit words (if needed)
__attribute__((always_inline)) static inline __m128i FixEndianness32(__m128i vector) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // Byte swap each 32-bit word in the 128-bit vector
    const __m128i mask = _mm_set_epi8(
        12, 13, 14, 15,  8,  9, 10, 11,  
         4,  5,  6,  7,  0,  1,  2,  3
    );
    return _mm_shuffle_epi8(vector, mask);
#else
    return vector;
#endif
}

// Fix endianness for 64-bit words (if needed)
__attribute__((always_inline)) static inline __m128i FixEndianness64(__m128i vector) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // Byte swap each 64-bit word in the 128-bit vector
    const __m128i mask = _mm_set_epi8(
        8,  9, 10, 11, 12, 13, 14, 15,  
        0,  1,  2,  3,  4,  5,  6,  7
    );
    return _mm_shuffle_epi8(vector, mask);
#else
    return vector;
#endif
}

// RCON values for AES key expansion
static const uint32_t RCON[10] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 
    0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000
};


template <typename CIPHER>
struct CIPHER_BLOCK;

template <const unsigned KEY_LENGTH = 128>
class AES {
    static constexpr unsigned KEY_WORDS = KEY_LENGTH / 32;          // Nk
    static constexpr unsigned NUM_ROUNDS = 6 + KEY_WORDS;           // Nr

public:
    __attribute__((aligned(4)))
    __m128i roundKeys[NUM_ROUNDS + 1];                           // round keys

    template <int round, int rcon>
    static inline void expandKey(__m128i* roundKeys) {
        
        __m128i temp = _mm_aeskeygenassist_si128(roundKeys[round - 1], rcon);
        temp = _mm_shuffle_epi32(temp, _MM_SHUFFLE(3, 3, 3, 3)); // Extract relevant bytes

        __m128i key = roundKeys[round - 1];
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
        key = _mm_xor_si128(key, temp);
        printf("%32llx %32llx \n", (unsigned long long)key[0],(unsigned long long)key[1]);

        roundKeys[round] = key;
    }

    // AES Key Expansion for AES-128
    void setKey(const uint8_t *key) {
        uint32_t *roundKeysW = (uint32_t *)roundKeys;
        const uint32_t *keyW = (const uint32_t *)key;

        // Copy the original key to the first round key
        roundKeys[0] = _mm_loadu_si128(reinterpret_cast<const __m128i*>(keyW));
        uint32_t *rconPtr = (uint32_t *)RCON;

        switch (KEY_LENGTH) {
            case 128:
                expandKey<1, 0x01>(roundKeys);
                expandKey<2, 0x02>(roundKeys);
                expandKey<3, 0x04>(roundKeys);
                expandKey<4, 0x08>(roundKeys);
                expandKey<5, 0x10>(roundKeys);
                expandKey<6, 0x20>(roundKeys);
                expandKey<7, 0x40>(roundKeys);
                expandKey<8, 0x80>(roundKeys);
                expandKey<9, 0x1B>(roundKeys);
                expandKey<10, 0x36>(roundKeys);
                break;
            
            case 192:
                expandKey<1, 0x01>(roundKeys);
                expandKey<2, 0x02>(roundKeys);
                expandKey<3, 0x04>(roundKeys);
                expandKey<4, 0x08>(roundKeys);
                expandKey<5, 0x10>(roundKeys);
                expandKey<6, 0x20>(roundKeys);
                expandKey<7, 0x40>(roundKeys);
                expandKey<8, 0x80>(roundKeys);
                expandKey<9, 0x1B>(roundKeys);
                expandKey<10, 0x36>(roundKeys);
                

                break;

            case 256:
                // roundKeys[1] = vld1q_u32(keyW + 4);
                roundKeys[1] = FixEndianness32(roundKeys[1]);

                // AES-256 key expansion loop
                for (unsigned i = KEY_WORDS; i < (NUM_ROUNDS + 1) * 4; i += KEY_WORDS) {
                    uint32_t temp = roundKeysW[i - 1];

                    // Perform RotWord, SubWord, and XOR with Rcon
                    temp = std::__rotl(temp, 8);    // Rotate right by 24 bits (left by 8 bits)
                    temp = SubWord(temp);           // SubWord
                    temp ^= *rconPtr++;             // XOR with Rcon

                    // XOR with the word 8 positions back
                    roundKeysW[i]     = roundKeysW[i - KEY_WORDS]     ^ temp;
                    roundKeysW[i + 1] = roundKeysW[i - KEY_WORDS + 1] ^ roundKeysW[i];
                    roundKeysW[i + 2] = roundKeysW[i - KEY_WORDS + 2] ^ roundKeysW[i + 1];
                    roundKeysW[i + 3] = roundKeysW[i - KEY_WORDS + 3] ^ roundKeysW[i + 2];                    
                    roundKeysW[i + 4] = roundKeysW[i - KEY_WORDS + 4] ^ SubWord(roundKeysW[i + 3]);
                    roundKeysW[i + 5] = roundKeysW[i - KEY_WORDS + 5] ^ roundKeysW[i + 4];
                    roundKeysW[i + 6] = roundKeysW[i - KEY_WORDS + 6] ^ roundKeysW[i + 5];
                    roundKeysW[i + 7] = roundKeysW[i - KEY_WORDS + 7] ^ roundKeysW[i + 6];
                }
                break;
        }
    }

    inline __m128i encryptBlock(__m128i block) const {
        for (unsigned i = 0; i < NUM_ROUNDS - 1; ++i) {
            // Perform AddRoundKey, SubBytes, and ShiftRows in one step
            block = _mm_aesenc_si128(block, roundKeys[i]);
        }
        // Final round: AddRoundKey, SubBytes, and ShiftRows without MixColumns
        block = _mm_aesenclast_si128(block, roundKeys[NUM_ROUNDS - 1]);
        // Final AddRoundKey
        return _mm_xor_si128(block, roundKeys[NUM_ROUNDS]);
    }

    inline void encryptBlock(const uint8_t *input, uint8_t *output) const {
        // Load 16 bytes from input
        __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
        
        // Encrypt the block
        block = encryptBlock(block);
        
        // Store the result in output
        _mm_storeu_si128(reinterpret_cast<__m128i*>(output), block);
    }

    inline __m128i decryptBlock(__m128i block) const {
        // block = AddRoundKey(block, roundKeys[NUM_ROUNDS])
        // block = InvShiftRows(block)
        // block = InvSubBytes(block)
        block = _mm_aesdec_si128(block, roundKeys[NUM_ROUNDS]);

        // block = AddRoundKey(block, roundKeys[NUM_ROUNDS - 1])
        block = _mm_xor_si128(block, roundKeys[NUM_ROUNDS - 1]);

        __m128i zero = _mm_setzero_si128();
        
        for (int i = NUM_ROUNDS - 2; i >= 0; --i) {
            // block = InvMixColumns(block)
            block = _mm_aesimc_si128(block);

            // block = InvShiftRows(block)
            // block = InvSubBytes(block)
            block = _mm_aesdec_si128(block, zero);

            // block = AddRoundKey(block, roundKeys[i])
            block = _mm_xor_si128(block, roundKeys[i]);
        }
        
        return block;
    }

    inline void decryptBlock(const uint8_t *input, uint8_t *output) const {
        // Load 16 bytes from input
        __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
        
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

/*
// Increment a 128-bit counter stored in a NEON register
static inline __m128i counterInc(__m128i counter) {
    // Reinterpret as a 64-bit vector (uint64x2_t)
    uint64x2_t counter64x2 = FixEndianness64(vreinterpretq_u64_u8(counter));

    // Vector to add [1, 0] (increment lower 64 bits)
    static uint64x2_t one = {0, 1};

    // Add 1 to lower 64 bits
    uint64x2_t incremented = vaddq_u64(counter64x2, vreinterpretq_u64_u8(one));

    // Detect carry by checking if lower 64 bits overflowed
    uint64x2_t carry = vcgtq_u64(counter64x2, incremented); // 0xFFFFFFFF_FFFFFFFF if carry

    // Add carry to upper 64-bit half
    incremented = vsubq_u64(incremented, vextq_u64(carry, carry, 1));

    return FixEndianness64(vreinterpretq_u8_u64(incremented));
}

static inline void counterInc(uint8_t *counter) {
    vst1q_u8(counter, counterInc(vld1q_u8(counter)));
}

static inline __m128i counterAdd(__m128i counter, uint64_t delta) {
    // Reinterpret as a 64-bit vector (uint64x2_t)
    uint64x2_t counter64x2 = FixEndianness64(vreinterpretq_u64_u8(counter));

    // Vector to add [1, 0] (increment lower 64 bits)
    uint64x2_t delta64x2 = {0, delta};

    // Add 1 to lower 64 bits
    uint64x2_t incremented = vaddq_u64(counter64x2, vreinterpretq_u64_u8(delta64x2));

    // Detect carry by checking if lower 64 bits overflowed
    uint64x2_t carry = vcgtq_u64(counter64x2, incremented); // 0xFFFFFFFF_FFFFFFFF if carry

    // Add carry to upper 64-bit half
    incremented = vsubq_u64(incremented, vextq_u64(carry, carry, 1));

    return FixEndianness64(vreinterpretq_u8_u64(incremented));
}


// encrypt an array of data using AES CTR mode, where offset is the byte offset in the stream to start encryption and length the array length in bytes
template <typename BLOCK_CIPHER, size_t BLOCK_OFFSET>
inline void encryptCTRWrapper(BLOCK_CIPHER const &cipher, const uint8_t *iv, const uint8_t *input, uint8_t *output, size_t length, size_t offset = 0) {
    constexpr size_t BLOCK_LENGTH = CIPHER_BLOCK<BLOCK_CIPHER>::LENGTH_BYTES;

    // Add block number to the nonce (IV)
    __m128i counter = counterAdd(vld1q_u8(iv), offset / BLOCK_LENGTH);

    // If input is not aligned to block boundary, encrypt the first block
    __m128i prevEncryptedCounter;
    if (BLOCK_OFFSET != 0) {
        prevEncryptedCounter = cipher.encryptBlock(counter);
        counter = counterInc(counter);
    }

    while (length >= BLOCK_LENGTH) {
        __m128i encryptedCounter = cipher.encryptBlock(counter);
        // XOR the encrypted counter with the input

        __m128i mask = (BLOCK_OFFSET != 0) ? vextq_u8(prevEncryptedCounter, encryptedCounter, BLOCK_OFFSET) : encryptedCounter;
        vst1q_u8(output, veorq_u8(vld1q_u8(input), mask));
        counter = counterInc(counter);

        input += BLOCK_LENGTH;
        output += BLOCK_LENGTH;
        length -= BLOCK_LENGTH;
        prevEncryptedCounter = encryptedCounter;
    }

    if (length > 0) {
        __m128i encryptedCounter = cipher.encryptBlock(counter);
        __m128i mask = (BLOCK_OFFSET != 0) ? vextq_u8(prevEncryptedCounter, encryptedCounter, BLOCK_OFFSET) : encryptedCounter;
        __m128i cyphertext = veorq_u8(vld1q_u8(input), mask);

        for (size_t i = 0; i < length; ++i) {
            output[i] = cyphertext[i];
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
    uint32x4_t kx = veorq_u32(
        FixEndianness32(vld1q_u32((const uint32_t *)iv)), 
        FixEndianness32(vld1q_u32((const uint32_t *)key)));
    // XOR the 4 32-bit words into one
    uint32_t word = kx[0] ^ kx[1] ^ kx[2] ^ kx[3];
    return (word & 0xffff) ^ (word >> 16);
}
*/
#endif // AES_INTEL_H
