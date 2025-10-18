#include "MMapFile.h"

std::unordered_set<DeviceAndInode, DeviceAndInodeHash> fileLocks;

// Helper functions
long long getFileSizeFromName(const std::string& path) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0) {
        perror("fstat");
        return -1; // Return -1 to indicate an error
    }
    return S_ISDIR(fileStat.st_mode) ? 0 : fileStat.st_size;  // Return 0 for directories, otherwise return file size
}

// Resize a file to a given size
bool fileResize(int fd, size_t newSize) {
    return ftruncate(fd, newSize) == 0;
}

// Lock a file, i.e. add it to the in-memory set of device/inode pairs
bool fileLock(uint64_t dev, uint64_t ino) {
    return fileLocks.insert(std::make_pair(dev, ino)).second;
}

// Unlock a file, i.e. remove it from the in-memory set of device/inode pairs
void fileUnlock(uint64_t dev, uint64_t ino) {
    fileLocks.erase(std::make_pair(dev, ino));
}

// Create a directory and all parent directories if they do not exist
bool createParentDir(const std::string &path) {
    size_t pos = 0;
    struct stat sb;

    while ((pos = path.find('/', pos + 1)) != std::string::npos) {
        std::string subPath = path.substr(0, pos);
        if (stat(subPath.c_str(), &sb) != 0) {
            if (errno != ENOENT || mkdir(subPath.c_str(), 0700) != 0) {
                return false;
            }
        } else if (!S_ISDIR(sb.st_mode)) {
            return false;
        }
    }
    return true;
}

// Check if a file exists
bool fileExist(const std::string &filePath) {
    return !filePath.empty() && access(filePath.c_str(), F_OK) == 0;
}

// Get the base path of a file
std::string basePath(const std::string &filePath) {
    size_t pos = filePath.find_last_of('/');
    return pos == std::string::npos ? "" : filePath.substr(0, pos);
}
