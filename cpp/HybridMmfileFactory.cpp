#include "HybridMmfileFactory.hpp"

#include <memory>
#include <string>

namespace margelo::nitro::mmfile
{

std::shared_ptr<HybridMmfileSpec> HybridMmfileFactory::openMmfile(path: string)
{
    return std::make_shared<HybridChild>(path);
}

std::shared_ptr<HybridEncryptedMmfileSpec> HybridMmfileFactory::openEncryptedMmfile(path: string, key: string)
{
    return std::make_shared<HybridEncryptedMmfile>(path);
}

std::string HybridMmfileFactory::reverseString(const std::string &input)
{
    return std::string(input.rbegin(), input.rend());
}

} // namespace margelo::nitro::mmfile
