#pragma once

#include "HybridMmfileSpec.hpp"
#include "HybridEncryptedMmfileSpec.hpp"
#include "MMapFile.h"
#include "MMapEncryptedFile.h"
#include <jsi/jsi.h>

namespace margelo::nitro::mmfile
{

using namespace facebook;

class HybridMmfile : public HybridMmfileSpec
{
public:
    HybridMmfile(string path) : HybridObject(TAG)
    {
        instance = new MMapFile(path);
        if (instance == nullptr) [[unlikely]] {
            throw std::runtime_error("Failed to create MMapFile instance");
        }
    }

    size_t size() override;
    void resize(size_t newSize) override;
    void clear() override;
    void append(ArrayBuffer data) override;
    void write(size_t offset, ArrayBuffer data) override;
    size_t read(size_t offset, ArrayBuffer data) override;

private:
    MMapFile *instance;
};

class HybridEncryptedMmfile : public HybridEncryptedMmfileSpec
{
public:
    HybridEncryptedMmfile(string path, ArrayBuffer key) : HybridObject(TAG)
    {
        instance = new MMapEncryptedFile(path, key->data(), key->size());
        if (instance == nullptr) [[unlikely]] {
            throw std::runtime_error("Failed to create MMapEncryptedFile instance");
        }
    }

    size_t size() override;
    void resize(size_t newSize) override;
    void clear() override;
    void append(ArrayBuffer data) override;
    void write(size_t offset, ArrayBuffer data) override;
    size_t read(size_t offset, ArrayBuffer data) override;

private:
    MMapEncryptedFile *instance;
};

} // namespace margelo::nitro::mmfile
