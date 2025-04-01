#include "HybridMmfile.hpp"
#include "HybridMmfilePackage.hpp"
#include "MMapEncryptedFile.h"

#include <sys/stat.h>
#include <filesystem>
#include <chrono>

namespace margelo::nitro::mmfile
{

std::shared_ptr<HybridMmfileSpec> HybridMmfilePackage::openMmfile(const std::string& path, std::optional<bool> readOnly)
{
    return std::make_shared<HybridMmfile>(getAbsolutePath(path), readOnly);
}

std::shared_ptr<HybridEncryptedMmfileSpec> HybridMmfilePackage::openEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> readOnly)
{
    return std::make_shared<HybridEncryptedMmfile>(getAbsolutePath(path), key, readOnly);
}

bool HybridMmfilePackage::fileExists(const std::string& path)
{
    return std::filesystem::exists(std::filesystem::path(getAbsolutePath(path)));
}

double HybridMmfilePackage::getFileSize(const std::string& path) {
    size_t size = getFileSizeFromName(getAbsolutePath(path));
    if (size == (size_t)-1)
    {
        throw std::runtime_error(std::string("Error getting file size for file: ") + path);
    }
    return size;
}

double HybridMmfilePackage::getEncryptedFileSize(const std::string& path) {
    return getFileSize(getAbsolutePath(path)) - sizeof(EncryptedFileHeader);
}

std::shared_ptr<Promise<std::vector<ReadDirItem>>> HybridMmfilePackage::readDir(const std::string& path) {
    std::string absPath = getAbsolutePath(path);
    return Promise<std::vector<ReadDirItem>>::async([=]() -> std::vector<ReadDirItem> {
        // This runs on a separate Thread!
        std::vector<ReadDirItem> items;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(absPath)) {
                ReadDirItem item;
                item.name = entry.path().filename().string();
                item.path = entry.path().string();
                item.isFile = entry.is_regular_file();
                item.isDirectory = entry.is_directory();
                item.size = item.isFile ? entry.file_size() : 0;
                item.mtime = std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count();
                items.push_back(item);
            }
        } catch (const std::exception& e) {
            // Handle errors (e.g., directory does not exist, permissions issue)
            throw std::runtime_error(std::string("Failed to read directory: ") + e.what());
        }
        return items;
      });
}

std::shared_ptr<Promise<void>> HybridMmfilePackage::unlink(const std::string& path) {
    std::string absPath = getAbsolutePath(path);
    return Promise<void>::async([=]() -> void {
        // This runs on a separate Thread!
        std::filesystem::remove_all(absPath);
      });
}

} // namespace margelo::nitro::mmfile
