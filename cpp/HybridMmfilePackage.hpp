#pragma once

#include "HybridMmfilePackageSpec.hpp"
#include "HybridMmfileSpec.hpp"
#include "HybridEncryptedMmfileSpec.hpp"
#include "ReadDirItem.hpp"
#include <jsi/jsi.h>

#include <string>

namespace margelo::nitro::mmfile
{

using namespace facebook;

class HybridMmfilePackage : public HybridMmfilePackageSpec
{
public:
    HybridMmfilePackage() : HybridObject(TAG) {}
    std::shared_ptr<HybridMmfileSpec> openMmfile(const std::string& path, std::optional<bool> readOnly) override;
    std::shared_ptr<HybridEncryptedMmfileSpec> openEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> readOnly) override;

    bool fileExists(const std::string& path) override;
    double getFileSize(const std::string& path) override;
    double getEncryptedFileSize(const std::string& path) override;

    std::shared_ptr<Promise<std::vector<ReadDirItem>>> readDir(const std::string& path) override;
    std::shared_ptr<Promise<void>> unlink(const std::string& path) override;
};

} // namespace margelo::nitro::mmfile
