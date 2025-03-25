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
    HybridMmfile(const std::string& path) : HybridObject(TAG)
    {
        instance = new MMapFile(path);
        if (instance == nullptr) [[unlikely]] {
            throw std::runtime_error("Failed to create MMapFile instance");
        }
    }

    double getSize() override;
    double getCapacity() override;
    bool getReadOnly() override;
    std::string getFilePath() override;

    void resize(double newSize) override;
    void clear() override;
    void append(const std::shared_ptr<ArrayBuffer>& buffer) override;
    void write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;
    double read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;

private:
    MMapFile *instance;
};

class HybridEncryptedMmfile : public HybridEncryptedMmfileSpec
{
public:
    HybridEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key) : HybridObject(TAG)
    {
        if (key.size != 16) [[unlikely]] {
            throw std::runtime_error("encryptionKey must be of length 16 but is " + std::to_string(key.size));
        }
        instance = new MMapEncryptedFile(path, key->data());
        if (instance == nullptr) [[unlikely]] {
            throw std::runtime_error("Failed to create MMapEncryptedFile instance");
        }
    }

    double getSize() override;
    double getCapacity() override;
    bool getReadOnly() override;
    std::string getFilePath() override;

    void resize(double newSize) override;
    void clear() override;
    void append(const std::shared_ptr<ArrayBuffer>& buffer) override;
    void write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;
    double read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;

private:
    MMapEncryptedFile *instance;
};

} // namespace margelo::nitro::mmfile
