#ifndef MMAPENCRYPTEDBUFFER_H
#define MMAPENCRYPTEDBUFFER_H

#include <string>
#include <stdexcept>
#include "MMapFile.h"
#include "AES.h"

// header size should be a multiple of 16 bytes (64 bytes)
struct Header {
    uint16_t magic;
    uint16_t version;
    uint64_t size;
    uint8_t  encryptedDataKey[16];
    uint8_t  iv[16];
    uint16_t keyHash;
    uint8_t  reserved[14];
};

template <typename T> static inline T FixEndianness(T val) { return val; }

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
template <> static inline uint16_t FixEndianness(uint16_t val) { return __builtin_bswap16(val); }
template <> static inline uint32_t FixEndianness(uint32_t val) { return __builtin_bswap32(val); }
template <> static inline uint64_t FixEndianness(uint64_t val) { return __builtin_bswap64(val); }
#endif

static const char* MMapEncryptedFileErrors[] = {
    "",
    "Encrypted file doesn't have a full header",
    "Header's magic number doesn't match",
    "Unsupported version",
    "Encrypted file is shorter than the header declares",
    "Wrong encryption key"
};

class MMapEncryptedFile
{
public:
    MMapEncryptedFile() {}
    
    MMapEncryptedFile(const std::string& filePath, const uint8_t *key, bool readOnly = false)
    {
        open(filePath, key, readOnly);
    }

    // Disable copying
    MMapEncryptedFile(const MMapEncryptedFile&) = delete;
    MMapEncryptedFile& operator=(const MMapEncryptedFile&) = delete;

    // Allow moving
    MMapEncryptedFile(MMapEncryptedFile&& other) noexcept :
        file_(std::move(other.file_)),
        aes_(std::move(other.aes_)),
        aesData_(std::move(other.aesData_)) {}

    MMapEncryptedFile& operator=(MMapEncryptedFile&& other) noexcept {
        file_ = std::move(other.file_);
        aes_ = std::move(other.aes_);
        aesData_ = std::move(other.aesData_);
        return *this;
    }

    ~MMapEncryptedFile() {
        close();
    }

    void open(const std::string& filePath, const uint8_t *key, bool readOnly = false) {
        file_.open(filePath, readOnly);
        aes_.setKey(key);
        int result = readHeader();
        if (result != 0)
        {
            throw std::runtime_error(std::string("Failed to open encrypted file: ") + filePath + ", error: " + MMapEncryptedFileErrors[result]);
        }
        // resize the file to the correct size (for handling the case when the app crashes before the file is closed)
        if (!file_.readOnly()) [[likely]] {
            file_.resize(sizeof(Header) + size(), true);
        }
    }

    void close() {
        if (isOpen() && !readOnly() && size() == 0) {
            file_.clear();
        }
        file_.close();
    }

    // Write data to the memory-mapped region
    void write(size_t offset, const uint8_t *data, size_t length) 
    {
        file_.assertFileIsOpen();
        if (file_.readOnly()) [[unlikely]] {
            throw std::runtime_error("Trying to write to a read-only file");
        }
        size_t minSize = offset + length;
        if (minSize > size())
        {
            resize(minSize);
        }
        encryptCTR(
            aesData_,
            reinterpret_cast<Header*>(file_.data())->iv,
            data,
            file_.data() + sizeof(Header) + offset,
            length,
            offset);
    }

    // Read data from the memory-mapped region
    size_t read(size_t offset, uint8_t *data, size_t length) const 
    {
        file_.assertFileIsOpen();
        if (offset >= size())
        {
            return 0;
        }
        if (offset + length > size())
        {
            length = size() - offset;
        }
        std::string result = std::string(length, '\0');
        decryptCTR(
            aesData_,
            reinterpret_cast<Header*>(file_.data())->iv,
            file_.data() + sizeof(Header) + offset,
            data,
            length,
            offset);
        return length;
    }

    void append(const uint8_t *data, size_t length, bool strictResize = false) {
        size_t offset = size();
        resize(offset + length, strictResize);
        write(offset, data, length);
    }

    inline size_t size() const { 
        file_.assertFileIsOpen();
        return FixEndianness(reinterpret_cast<Header*>(file_.data())->size);
    }
    
    inline size_t capacity() const { return file_.size() - sizeof(Header); }
    inline bool readOnly() const { return file_.readOnly(); }
    inline const std::string& filePath() const { return file_.filePath(); }
    inline bool isOpen() const { return file_.isOpen(); }

    void resize(size_t newSize, bool strictResize = false)
    {
        file_.assertFileIsOpen();
        if (file_.readOnly()) [[unlikely]] {
            throw std::runtime_error("Trying to resize a read-only file");
        }

        if (newSize < size()) {
            // the order is important here to avoid inconsistencies
            reinterpret_cast<Header*>(file_.data())->size = FixEndianness((uint64_t)newSize);
            file_.resize(newSize + sizeof(Header), strictResize);
        } else {
            // the order is important here to avoid inconsistencies
            file_.resize(newSize + sizeof(Header), strictResize);
            reinterpret_cast<Header*>(file_.data())->size = FixEndianness((uint64_t)newSize);
        }
    }

    void clear() { resize(0, true); }

private:
public:
    MMapFile file_;
    AES<128> aes_, aesData_;

    static const uint16_t MAGIC = 0xDA7A;

    int readHeader() {
        if (file_.size() == 0) {
            initHeader();
            return 0;
        }
        if (file_.size() < sizeof(Header)) {
            return 1;
        }

        Header* header = reinterpret_cast<Header*>(file_.data());
        if (FixEndianness(header->magic) != MAGIC) {
            return 2;
        }
        if (FixEndianness(header->version) != 1) {
            return 3;
        }
        if (FixEndianness(header->size) + sizeof(Header) > file_.size()) {
            return 4;
        }

        // decrypt data key
        uint8_t dataKey[16];
        aes_.decryptBlock(header->encryptedDataKey, dataKey);

        if (FixEndianness(header->keyHash) != hashIVAndKey(header->iv, dataKey)) {
            return 5;
        }

        aesData_.setKey(dataKey);
        return 0;
    }

    void initHeader() {
        file_.resize(sizeof(Header), true);
        uint8_t dataKey[16];
        Header* header = reinterpret_cast<Header*>(file_.data());
        header->magic = FixEndianness(MAGIC);
        header->version = FixEndianness((uint16_t)1);
        header->size = 0;
        // generate a random data key and encrypt it
        fillRandom(dataKey, 16);
        aes_.encryptBlock(dataKey, header->encryptedDataKey);
        aesData_.setKey(dataKey);
        // generate a random IV
        fillRandom(header->iv, 16);
        // calculate the hash of the IV and the data key
        header->keyHash = FixEndianness(hashIVAndKey(header->iv, dataKey));
    }
};

#endif // MMAPENCRYPTEDBUFFER_H
