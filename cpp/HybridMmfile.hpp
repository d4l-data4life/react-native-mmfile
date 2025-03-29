#pragma once

#include "HybridMmfileSpec.hpp"
#include "HybridEncryptedMmfileSpec.hpp"
#include "MMapFile.h"
#include "MMapEncryptedFile.h"
#include <jsi/jsi.h>

#include <string>

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
        if (key->size() != 16) [[unlikely]] {
            throw std::runtime_error("encryptionKey must be of length 16 but is " + std::to_string(key->size()));
        }
        instance = new MMapEncryptedFile(path, key->data(), readOnly.has_value() ? readOnly.value() : false);
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
    MMapEncryptedFile *instance;
};

} // namespace margelo::nitro::mmfile
