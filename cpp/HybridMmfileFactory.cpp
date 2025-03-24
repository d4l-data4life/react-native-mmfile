#include "HybridMmfile.hpp"
#include "HybridMmfileFactory.hpp"

namespace margelo::nitro::mmfile
{

std::shared_ptr<HybridMmfileSpec> HybridMmfileFactory::openMmfile(const std::string& path)
{
    return std::make_shared<HybridMmfile>(path);
}

std::shared_ptr<HybridEncryptedMmfileSpec> HybridMmfileFactory::openEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key)
{
    return std::make_shared<HybridEncryptedMmfile>(path, key);
}

} // namespace margelo::nitro::mmfile
