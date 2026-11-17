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
    DEBUG_LOG("openMmfile(" << path << ", readOnly=" << (readOnly ? std::to_string(*readOnly) : "nullopt") << ")");
    return std::make_shared<HybridMmfile>(getAbsolutePath(path), readOnly);
}

std::shared_ptr<HybridEncryptedMmfileSpec> HybridMmfilePackage::openEncryptedMmfile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> readOnly)
{
    DEBUG_LOG("openEncryptedMmfile(" << path << ", key=..., readOnly=" << (readOnly ? std::to_string(*readOnly) : "nullopt") << ")");
    return std::make_shared<HybridEncryptedMmfile>(getAbsolutePath(path), key, readOnly);
}

bool HybridMmfilePackage::fileExists(const std::string& path)
{
    DEBUG_LOG("fileExists(" << path << ")");
    return std::filesystem::exists(std::filesystem::path(getAbsolutePath(path)));
}

double HybridMmfilePackage::getFileSize(const std::string& path) {
    DEBUG_LOG("getFileSize(" << path << ")");
    long long size = getFileSizeFromName(getAbsolutePath(path));
    if (size == -1)
    {
        throw std::runtime_error(std::string("Error getting file size for file: ") + path);
    }
    return size;
}

double HybridMmfilePackage::getEncryptedFileSize(const std::string& path) {
    DEBUG_LOG("getEncryptedFileSize(" << path << ")");
    long long size = getFileSizeFromName(getAbsolutePath(path));
    if (size == -1 || (size > 0 && size < (long long)sizeof(EncryptedFileHeader)))
    {
        throw std::runtime_error(std::string("Error getting encrypted file size for file: ") + path);
    }
    return size == 0 ? 0 : getFileSize(getAbsolutePath(path)) - sizeof(EncryptedFileHeader);
}

bool HybridMmfilePackage::isLegacyEncryptedFile(const std::string& path) {
    DEBUG_LOG("isLegacyEncryptedFile(" << path << ")");
    return ::isLegacyEncryptedFile(getAbsolutePath(path));
}

bool HybridMmfilePackage::rekeyEncryptedFile(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> force) {
    DEBUG_LOG("rekeyEncryptedFile(" << path << ", key=..., force=" << (force ? std::to_string(*force) : "nullopt") << ")");
    std::string absPath = getAbsolutePath(path);
    // Outside the data directories, so a leftover of an interrupted rekey is never
    // listed next to the files. Calls run one at a time, so one name is enough.
    std::string tmpPath = getAbsolutePath(".rekey/pending");
    bool f = force.has_value() && force.value();
    switch (key->size()) {
        case 16: return ::rekeyEncryptedFile<128>(absPath, tmpPath, key->data(), f);
        case 24: return ::rekeyEncryptedFile<192>(absPath, tmpPath, key->data(), f);
        case 32: return ::rekeyEncryptedFile<256>(absPath, tmpPath, key->data(), f);
        default:
            throw std::runtime_error("encryptionKey must be 16, 24, or 32 bytes (AES-128/192/256) but is " + std::to_string(key->size()));
    }
}

static std::vector<ReadDirItem> _readDir(const std::string& absPath) {
    DEBUG_LOG("_readDir(" << absPath << ")");
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
}

std::vector<ReadDirItem> HybridMmfilePackage::readDirSync(const std::string& path) {
    DEBUG_LOG("readDirSync(" << path << ")");
    std::string absPath = getAbsolutePath(path);
    return _readDir(absPath);
}

std::shared_ptr<Promise<std::vector<ReadDirItem>>> HybridMmfilePackage::readDir(const std::string& path) {
    DEBUG_LOG("readDir(" << path << ")");
    std::string absPath = getAbsolutePath(path);
    return Promise<std::vector<ReadDirItem>>::async([=]() -> std::vector<ReadDirItem> {
        // This runs on a separate Thread!
        return _readDir(absPath);
    });
}

void HybridMmfilePackage::unlinkSync(const std::string& path) {
    DEBUG_LOG("unlinkSync(" << path << ")");
    std::string absPath = getAbsolutePath(path);
    std::filesystem::remove_all(absPath);
}

std::shared_ptr<Promise<void>> HybridMmfilePackage::unlink(const std::string& path) {
    DEBUG_LOG("unlink(" << path << ")");
    std::string absPath = getAbsolutePath(path);
    return Promise<void>::async([=]() -> void {
        // This runs on a separate Thread!
        std::filesystem::remove_all(absPath);
      });
}


} // namespace margelo::nitro::mmfile
