#pragma once

#include "HybridMmfileFactorySpec.hpp"
#include "HybridMmfileSpec.hpp"
#include "HybridEncryptedMmfileSpec.hpp"
#include <jsi/jsi.h>

namespace margelo::nitro::mmfile
{

using namespace facebook;

class HybridMmfileFactory : public HybridMmfileFactorySpec
{
public:
    HybridMmfileFactory() : HybridObject(TAG) {}
    std::string reverseString(const std::string& input);
    std::shared_ptr<HybridMmfileSpec> openMmfile(path: string);
    std::shared_ptr<HybridEncryptedMmfileSpec> openEncryptedMmfile(path: string, key: string);
};

} // namespace margelo::nitro::mmfile
