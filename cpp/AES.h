#ifndef AES_H
#define AES_H

#include <arm_neon.h>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <random>

// SubWord function applies the AES S-box substitution to a 4-byte word
__attribute__((always_inline)) inline uint32_t SubWord(uint32_t word) {
    uint8x16_t bytes = vreinterpretq_u8_u32(vdupq_n_u32(word));
    return vreinterpretq_u32_u8(vaeseq_u8(bytes, vdupq_n_u8(0)))[0]; // Use AES instruction for S-box substitution
}

// Fix endianness for 32-bit words (if needed)
__attribute__((always_inline)) static inline uint32x4_t FixEndianness32(uint32x4_t vector) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(vector)));
#else
    return vector;
#endif
}

// Fix endianness for 64-bit words (if needed)
__attribute__((always_inline)) static inline uint64x2_t FixEndianness64(uint64x2_t vector) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(vector)));
#else
    return vector;
#endif
}

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
    uint8x16_t roundKeys[NUM_ROUNDS + 1];                           // round keys

    // AES Key Expansion for AES-128
    void setKey(const uint8_t *key) {
        uint32_t *roundKeysW = (uint32_t *)roundKeys;
        const uint32_t *keyW = (const uint32_t *)key;

        // Copy the original key to the first round key
        roundKeys[0] = vld1q_u32(keyW);
        roundKeys[0] = FixEndianness32(roundKeys[0]);
        uint32_t *rconPtr = (uint32_t *)RCON;

        switch (KEY_LENGTH) {
            case 128:
                // AES-128 key expansion loop
                for (unsigned i = KEY_WORDS; i < (NUM_ROUNDS + 1) * 4; i += KEY_WORDS) {
                    uint32_t temp = roundKeysW[i - 1];

                    // Perform RotWord, SubWord, and XOR with Rcon
                    __asm__ volatile (
                        "ROR %w[temp], %w[temp], #24"  // Rotate right by 24 bits (left by 8 bits)
                        : [temp] "+r" (temp)
                    );
                    temp = SubWord(temp);   // SubWord
                    temp ^= *rconPtr++;     // XOR with Rcon

                    // XOR with the word 4 positions back
                    roundKeysW[i]     = roundKeysW[i - KEY_WORDS]     ^ temp;
                    roundKeysW[i + 1] = roundKeysW[i - KEY_WORDS + 1] ^ roundKeysW[i];
                    roundKeysW[i + 2] = roundKeysW[i - KEY_WORDS + 2] ^ roundKeysW[i + 1];
                    roundKeysW[i + 3] = roundKeysW[i - KEY_WORDS + 3] ^ roundKeysW[i + 2];
                }
                break;
            
            case 192:
                roundKeys[1] = vcombine_u32(vld1_u32(keyW + 4), vmov_n_u32(0));
                roundKeys[1] = FixEndianness32(roundKeys[1]);

                // AES-192 key expansion loop
                for (unsigned i = KEY_WORDS; i < (NUM_ROUNDS + 1) * 4; i += KEY_WORDS) {
                    uint32_t temp = roundKeysW[i - 1];

                    // Perform RotWord, SubWord, and XOR with Rcon
                    __asm__ volatile (
                        "ROR %w[temp], %w[temp], #24"  // Rotate right by 24 bits (left by 8 bits)
                        : [temp] "+r" (temp)
                    );
                    temp = SubWord(temp);   // SubWord
                    temp ^= *rconPtr++;     // XOR with Rcon

                    // XOR with the word 6 positions back
                    roundKeysW[i]     = roundKeysW[i - KEY_WORDS]     ^ temp;
                    roundKeysW[i + 1] = roundKeysW[i - KEY_WORDS + 1] ^ roundKeysW[i];
                    roundKeysW[i + 2] = roundKeysW[i - KEY_WORDS + 2] ^ roundKeysW[i + 1];
                    roundKeysW[i + 3] = roundKeysW[i - KEY_WORDS + 3] ^ roundKeysW[i + 2];
                    roundKeysW[i + 4] = roundKeysW[i - KEY_WORDS + 4] ^ roundKeysW[i + 3];
                    roundKeysW[i + 5] = roundKeysW[i - KEY_WORDS + 5] ^ roundKeysW[i + 4];
                }
                break;

            case 256:
                roundKeys[1] = vld1q_u32(keyW + 4);
                roundKeys[1] = FixEndianness32(roundKeys[1]);

                // AES-256 key expansion loop
                for (unsigned i = KEY_WORDS; i < (NUM_ROUNDS + 1) * 4; i += KEY_WORDS) {
                    uint32_t temp = roundKeysW[i - 1];

                    // Perform RotWord, SubWord, and XOR with Rcon
                    __asm__ volatile (
                        "ROR %w[temp], %w[temp], #24"  // Rotate right by 24 bits (left by 8 bits)
                        : [temp] "+r" (temp)
                    );
                    temp = SubWord(temp);   // SubWord
                    temp ^= *rconPtr++;     // XOR with Rcon

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

        for (unsigned i = 0; i < NUM_ROUNDS + 1; ++i) {
            roundKeys[i] = FixEndianness32(roundKeys[i]);
        }
    }

    inline uint8x16_t encryptBlock(uint8x16_t block) const {
        for (int i = 0; i < NUM_ROUNDS - 1; ++i) {
            // block = AddRoundKey(block, roundKeys[i])
            // block = SubBytes(block)
            // block = ShiftRows(block)
            block = vaeseq_u8(block, roundKeys[i]);
            // block = MixColumns(block)
            block = vaesmcq_u8(block);
        }
        // block = AddRoundKey(block, roundKeys[NUM_ROUNDS - 1])
        // block = SubBytes(block)
        // block = ShiftRows(block)
        block = vaeseq_u8(block, roundKeys[NUM_ROUNDS - 1]);
        // block = AddRoundKey(block, roundKeys[NUM_ROUNDS])
        return veorq_u8(block, roundKeys[NUM_ROUNDS]);
    }

    inline void encryptBlock(const uint8_t *input, uint8_t *output) const {
        vst1q_u8(output, encryptBlock(vld1q_u8(input)));
    }

    inline uint8x16_t decryptBlock(uint8x16_t block) const {
        // block = AddRoundKey(block, roundKeys[NUM_ROUNDS])
        // block = InvShiftRows(block)
        // block = InvSubBytes(block)
        block = vaesdq_u8(block, roundKeys[NUM_ROUNDS]);
        // block = AddRoundKey(block, roundKeys[NUM_ROUNDS - 1])
        block = veorq_u8(block, roundKeys[NUM_ROUNDS - 1]);
        uint8x16_t zero = vdupq_n_u8(0);
        for (int i = NUM_ROUNDS - 2; i >= 0; --i) {
            // block = InvMixColumns(block)
            block = vaesimcq_u8(block);
            // block = InvShiftRows(block)
            // block = InvSubBytes(block)
            block = vaesdq_u8(block, zero);
            // block = AddRoundKey(block, roundKeys[i])
            block = veorq_u8(block, roundKeys[i]);
        }
        return block;
    }

    inline void decryptBlock(const uint8_t *input, uint8_t *output) const {
        vst1q_u8(output, decryptBlock(vld1q_u8(input)));
    }
};

template <const unsigned KEY_LENGTH>
struct CIPHER_BLOCK<AES<KEY_LENGTH>> {
    using type = uint8x16_t;
    static constexpr unsigned LENGTH_BITS = 128;
    static constexpr unsigned LENGTH_BYTES = LENGTH_BITS / 8;
    static constexpr unsigned LENGTH_WORDS = LENGTH_BYTES / 4;
};


// Increment a 128-bit counter stored in a NEON register
static inline uint8x16_t counterInc(uint8x16_t counter) {
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

static inline uint8x16_t counterAdd(uint8x16_t counter, uint64_t delta) {
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
    uint8x16_t counter = counterAdd(vld1q_u8(iv), offset / BLOCK_LENGTH);

    // If input is not aligned to block boundary, encrypt the first block
    uint8x16_t prevEncryptedCounter;
    if (BLOCK_OFFSET != 0) {
        prevEncryptedCounter = cipher.encryptBlock(counter);
        counter = counterInc(counter);
    }

    while (length >= BLOCK_LENGTH) {
        uint8x16_t encryptedCounter = cipher.encryptBlock(counter);
        // XOR the encrypted counter with the input

        uint8x16_t mask = (BLOCK_OFFSET != 0) ? vextq_u8(prevEncryptedCounter, encryptedCounter, BLOCK_OFFSET) : encryptedCounter;
        vst1q_u8(output, veorq_u8(vld1q_u8(input), mask));
        counter = counterInc(counter);

        input += BLOCK_LENGTH;
        output += BLOCK_LENGTH;
        length -= BLOCK_LENGTH;
        prevEncryptedCounter = encryptedCounter;
    }

    if (length > 0) {
        uint8x16_t encryptedCounter = cipher.encryptBlock(counter);
        uint8x16_t mask = (BLOCK_OFFSET != 0) ? vextq_u8(prevEncryptedCounter, encryptedCounter, BLOCK_OFFSET) : encryptedCounter;
        uint8x16_t cyphertext = veorq_u8(vld1q_u8(input), mask);

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

#endif // AES_H
