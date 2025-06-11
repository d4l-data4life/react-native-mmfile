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

    std::string getBaseDirectory() override { return baseDirectory_; }
    void setBaseDirectory(const std::string& baseDirectory) override { baseDirectory_ = baseDirectory; }

    std::shared_ptr<HybridMmfileSpec> openMmfile(const std::string& path, std::optional<bool> readOnly) override;
    std::shared_ptr<HybridEncryptedMmfileSpec> openEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> readOnly) override;

    bool fileExists(const std::string& path) override;
    double getFileSize(const std::string& path) override;
    double getEncryptedFileSize(const std::string& path) override;

    std::vector<ReadDirItem> readDirSync(const std::string& path) override;
    std::shared_ptr<Promise<std::vector<ReadDirItem>>> readDir(const std::string& path) override;

    void unlinkSync(const std::string& path) override;
    std::shared_ptr<Promise<void>> unlink(const std::string& path) override;

private:
    std::string baseDirectory_;

    inline std::string getAbsolutePath(const std::string& path)
    {
        if (!path.empty() && path[0] == '/') {
            return path;
        }
        return baseDirectory_ + "/" + path;
    }
};

} // namespace margelo::nitro::mmfile
