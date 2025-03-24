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
        instance = new MMapEncryptedFile(path, key->data(), key->size());
        if (instance == nullptr) [[unlikely]] {
            throw std::runtime_error("Failed to create MMapEncryptedFile instance");
        }
    }

    double getSize() override;
    void resize(double newSize) override;
    void clear() override;
    void append(const std::shared_ptr<ArrayBuffer>& buffer) override;
    void write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;
    double read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) override;

private:
    MMapEncryptedFile *instance;
};

} // namespace margelo::nitro::mmfile
