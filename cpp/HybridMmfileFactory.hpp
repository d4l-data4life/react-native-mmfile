#pragma once

#include "HybridMmfileFactorySpec.hpp"
#include "HybridMmfileSpec.hpp"
#include "HybridEncryptedMmfileSpec.hpp"
#include <jsi/jsi.h>

#include <string>

namespace margelo::nitro::mmfile
{

using namespace facebook;

class HybridMmfileFactory : public HybridMmfileFactorySpec
{
public:
    HybridMmfileFactory() : HybridObject(TAG) {}
    std::shared_ptr<HybridMmfileSpec> openMmfile(const std::string& path) override;
    std::shared_ptr<HybridEncryptedMmfileSpec> openEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key) override;
};

} // namespace margelo::nitro::mmfile
