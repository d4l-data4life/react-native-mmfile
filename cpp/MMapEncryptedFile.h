#ifndef MMAPENCRYPTEDBUFFER_H
#define MMAPENCRYPTEDBUFFER_H

#include <string>
#include <stdexcept>
#include "MMapFile.h"
#include "AES.h"

// header size should be a multiple of 16 bytes (64 bytes)
struct EncryptedFileHeader {
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
        try {
            readHeader();
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to open encrypted file: ") + filePath + ", error: " + e.what());
        }
        // resize the file to the correct size (for handling the case when the app crashes before the file is closed)
        if (!file_.readOnly()) [[likely]] {
            file_.resize(sizeof(EncryptedFileHeader) + size(), true);
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
            reinterpret_cast<EncryptedFileHeader*>(file_.data())->iv,
            data,
            file_.data() + sizeof(EncryptedFileHeader) + offset,
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
            reinterpret_cast<EncryptedFileHeader*>(file_.data())->iv,
            file_.data() + sizeof(EncryptedFileHeader) + offset,
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
        return FixEndianness(reinterpret_cast<EncryptedFileHeader*>(file_.data())->size);
    }
    
    inline size_t capacity() const { return file_.size() - sizeof(EncryptedFileHeader); }
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
            reinterpret_cast<EncryptedFileHeader*>(file_.data())->size = FixEndianness((uint64_t)newSize);
            file_.resize(newSize + sizeof(EncryptedFileHeader), strictResize);
        } else {
            // the order is important here to avoid inconsistencies
            file_.resize(newSize + sizeof(EncryptedFileHeader), strictResize);
            reinterpret_cast<EncryptedFileHeader*>(file_.data())->size = FixEndianness((uint64_t)newSize);
        }
    }

    void clear() { resize(0, true); }

private:
public:
    MMapFile file_;
    AES<128> aes_, aesData_;

    static const uint16_t MAGIC = 0xDA7A;

    void readHeader() {
        if (file_.size() == 0) {
            initHeader();
            return;
        }
        if (file_.size() < sizeof(EncryptedFileHeader)) {
            throw std::runtime_error("Encrypted file doesn't have a full header, size: " + std::to_string(file_.size()));
        }

        EncryptedFileHeader* header = reinterpret_cast<EncryptedFileHeader*>(file_.data());
        if (FixEndianness(header->magic) != MAGIC) {
            throw std::runtime_error("Encrypted file header's magic number doesn't match, expected: " + 
                std::to_string(MAGIC) +
                ", got: " + 
                std::to_string(FixEndianness(header->magic)));
        }

        if (FixEndianness(header->version) != 1) {
            throw std::runtime_error("Unsupported version: " + std::to_string(FixEndianness(header->version)));
        }
        
        if (FixEndianness(header->size) + sizeof(EncryptedFileHeader) > file_.size()) {
            throw std::runtime_error("Encrypted file is shorter than the header declares, declared size: " + 
                std::to_string(sizeof(EncryptedFileHeader)) + "+" +
                std::to_string(FixEndianness(header->size)) + 
                ", actual size: " + 
                std::to_string(file_.size()));
        }

        // decrypt data key
        uint8_t dataKey[16];
        aes_.decryptBlock(header->encryptedDataKey, dataKey);

        if (FixEndianness(header->keyHash) != hashIVAndKey(header->iv, dataKey)) {
            return throw std::runtime_error("Wrong encryption key, hash mismatch");
        }

        aesData_.setKey(dataKey);
    }

    void initHeader() {
        file_.resize(sizeof(EncryptedFileHeader), true);
        uint8_t dataKey[16];
        EncryptedFileHeader* header = reinterpret_cast<EncryptedFileHeader*>(file_.data());
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
