#pragma once

#include "HybridMmfileSpec.hpp"
#include "HybridEncryptedMmfileSpec.hpp"
#include "MMapFile.h"
#include "MMapEncryptedFile.h"
#include <jsi/jsi.h>

#include <string>
#include <iostream> // Add for debug logging

// Debug log macro
#ifndef NITRO_MMFILE_DEBUG
#define NITRO_MMFILE_DEBUG getenv("NITRO_MMFILE_DEBUG")
#endif
#define DEBUG_LOG(msg) do { if (NITRO_MMFILE_DEBUG && std::string(NITRO_MMFILE_DEBUG) == "1") std::cout << "[HybridMmfile] " << msg << std::endl; } while(0)

namespace margelo::nitro::mmfile
{

using namespace facebook;

class HybridMmfile : public HybridMmfileSpec
{
public:
    HybridMmfile(const std::string& path, std::optional<bool> readOnly) : HybridObject(TAG)
    {
        instance = new MMapFile(path, readOnly.has_value() ? readOnly.value() : false);
        if (instance == nullptr) [[unlikely]] {
            throw std::runtime_error("Failed to create MMapFile instance");
        }
    }

    double getSize() override;
    double getCapacity() override;
    bool getReadOnly() override;
    std::string getFilePath() override;
    bool getIsOpen() override;

    void open(const std::string& path, std::optional<bool> readOnly) override;
    void close() override;
    void resize(double newSize, std::optional<bool> strictResize) override;
    void clear() override;
    void append(const std::shared_ptr<ArrayBuffer>& buffer, std::optional<bool> strictResize) override;
    void write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;
    double read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;

private:
    MMapFile *instance;
};

class HybridEncryptedMmfile : public HybridEncryptedMmfileSpec
{
public:
    HybridEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> readOnly) : HybridObject(TAG)
    {
        bool ro = readOnly.has_value() ? readOnly.value() : false;
        keyLength_ = key->size() * 8;
        if (key->size() == 16)
            instance = new MMapEncryptedFile<128>(path, key->data(), ro);
        else if (key->size() == 24)
            instance = new MMapEncryptedFile<192>(path, key->data(), ro);
        else if (key->size() == 32)
            instance = new MMapEncryptedFile<256>(path, key->data(), ro);
        else [[unlikely]]
            throw std::runtime_error("encryptionKey must be 16, 24, or 32 bytes (AES-128/192/256) but is " + std::to_string(key->size()));
        if (instance == nullptr) [[unlikely]] {
            throw std::runtime_error("Failed to create MMapEncryptedFile instance");
        }
    }

    double getSize() override;
    double getCapacity() override;
    bool getReadOnly() override;
    std::string getFilePath() override;
    bool getIsOpen() override;

    void open(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> readOnly) override;
    void close() override;
    void resize(double newSize, std::optional<bool> readOnly) override;
    void clear() override;
    void append(const std::shared_ptr<ArrayBuffer>& buffer, std::optional<bool> strictResize) override;
    void write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;
    double read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;

private:
    MMapEncryptedFileBase *instance;
    unsigned keyLength_ = 128;  // AES key length in bits (128, 192, or 256)
};

} // namespace margelo::nitro::mmfile
