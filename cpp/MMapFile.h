#ifndef MMAPFILE_H
#define MMAPFILE_H

#include <string>
#include <stdexcept>

#include <string>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h> // For fstat
#include <fcntl.h>    // For open()
#include <unistd.h>   // For ftruncate(), close()

// #include <android/log.h>
// #define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "rtnmmrray", __VA_ARGS__))

// Helper functions
static inline long long getFileSizeFromName(const std::string& path) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0) {
        perror("fstat");
        return -1; // Return -1 to indicate an error
    }
    return S_ISDIR(fileStat.st_mode) ? 0 : fileStat.st_size;  // Return 0 for directories, otherwise return file size
}

static inline long long getFileSizeFromFd(int fd) {
    struct stat fileStat;
    if (fstat(fd, &fileStat) != 0) {
        perror("fstat");
        return -1; // Return -1 to indicate an error
    }
    return fileStat.st_size;
}

// Resize a file to a given size
static inline bool fileResize(int fd, size_t newSize) {
    return ftruncate(fd, newSize) == 0;
}

// Create a directory and all parent directories if they do not exist
static inline bool createParentDir(const std::string &path) {
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
static inline bool fileExist(const std::string &filePath) {
    return !filePath.empty() && access(filePath.c_str(), F_OK) == 0;
}

// Get the base path of a file
static inline std::string basePath(const std::string &filePath) {
    size_t pos = filePath.find_last_of('/');
    return pos == std::string::npos ? "" : filePath.substr(0, pos);
}

class MMapFile {
public:
    // Default C'tor
    MMapFile() : 
        size_(0),
        capacity_(0),
        data_(nullptr),
        fd_(-1),
        readOnly_(false) {}

    // Constructor
    MMapFile(const std::string& filePath, bool readOnly = false) :
        size_(0),
        capacity_(0),
        data_(nullptr),
        fd_(-1),
        readOnly_(false)
    {
        open(filePath, readOnly);
    }

    // Destructor
    ~MMapFile()
    {
        close();
    }

    // Disable copying
    MMapFile(const MMapFile&) = delete;
    MMapFile& operator=(const MMapFile&) = delete;

    // Allow moving
    MMapFile(MMapFile&& other) noexcept
    {
        filePath_ = std::move(other.filePath_);
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = other.data_;
        fd_ = other.fd_;
        readOnly_ = other.readOnly_;

        other.size_ = 0;
        other.capacity_ = 0;
        other.data_ = nullptr;
        other.fd_ = -1;
    }

    // Allow moving
    MMapFile& operator=(MMapFile&& other) noexcept
    {
        filePath_ = std::move(other.filePath_);
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = other.data_;
        fd_ = other.fd_;
        readOnly_ = other.readOnly_;

        other.size_ = 0;
        other.capacity_ = 0;
        other.data_ = nullptr;
        other.fd_ = -1;
        return *this;
    }

    void open(const std::string& filePath, bool readOnly = false) 
    {
        close();
        if (filePath.empty()) [[unlikely]] {
            throw std::runtime_error("Failed to create/open file `filePath` is empty");
        }
        if (!createParentDir(filePath)) [[unlikely]] {
            throw std::runtime_error(std::string("Failed to create parent directory for file: ") + filePath);
        }
        fd_ = ::open(filePath.c_str(), readOnly ? O_RDONLY : O_RDWR | O_CREAT, 0600);
        if (fd_ < 0)
        {
            throw std::runtime_error(std::string("Failed to create/open file: ") + filePath + " error: " + strerror(errno));
        }
        filePath_ = filePath;
        readOnly_ = readOnly;

        size_ = getFileSizeFromFd(fd_);
        if (size_ == (size_t)-1) [[unlikely]] 
        {
            close();
            throw std::runtime_error("Failed to get file size");
        }
        capacity_ = size_;

        if (size_ == 0) [[unlikely]] {
            data_ = nullptr;
            return;
        }

        data_ = static_cast<uint8_t *>(mmap(nullptr, capacity_, readOnly ? PROT_READ : PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        if (data_ == MAP_FAILED) [[unlikely]] {
            close();
            throw std::runtime_error(std::string("Failed to map file ") + filePath + " with size " + std::to_string(size_));
        }        
    }

    void close(bool dontTouchFile = false)
    {
        if (data_) {
            if (data_ != MAP_FAILED) {
                munmap(data_, capacity_);
            }
            data_ = nullptr;
        }

        if (fd_ >= 0) {
            if (!readOnly_ && !dontTouchFile) {
                fileResize(fd_, size_);
            }
            ::close(fd_);
            fd_ = -1;

            // delete file if empty
            if (!readOnly_ && !dontTouchFile && size_ == 0 && !filePath_.empty()) {
                remove(filePath_.c_str());
            }
        }

        size_ = 0;
        capacity_ = 0;
        filePath_.clear();
    }
    
    // Resize the file and memory-mapped region
    inline void resize(size_t newSize, bool strictResize = false)
    {
        assertFileIsOpen();
        if (readOnly_) [[unlikely]]{
            throw std::runtime_error("Trying to resize a read-only file");
        }

        if (newSize == size_ || (!strictResize && newSize <= capacity_)) [[likely]] {
            size_ = newSize;
            return;
        }

        size_t newCapacity = strictResize ? newSize : (newSize | 0xffff); // round up to units of 64KB to avoid frequent resizing

        if (!fileResize(fd_, newCapacity)) [[unlikely]]
        {
            close();
            throw std::runtime_error("Failed to resize file");
        }
        size_ = newSize;

        uint8_t* newData = nullptr;
        if (newCapacity != 0) [[likely]] {
            newData = static_cast<uint8_t *>(mmap(data_, newCapacity, readOnly_ ? PROT_READ : PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
            if (newData != data_) [[unlikely]] {
                munmap(data_, capacity_);
            }
        }

        if (newData == MAP_FAILED) [[unlikely]]{
            close();
            throw std::runtime_error("Failed to map file");
        }
        data_ = newData;
        capacity_ = newCapacity;
    }

    // Write data to the memory-mapped region
    inline void write(size_t offset, const uint8_t *data, size_t length)
    {
        assertFileIsOpen();
        if (readOnly_) [[unlikely]]{
            throw std::runtime_error("Trying to write to a read-only file");
        }
        size_t minSize = offset + length;
        if (minSize > size()) [[unlikely]]
        {
            resize(minSize);
        }
        memcpy(data_ + offset, data, length);
    }

    // Read data from the memory-mapped region
    inline size_t read(size_t offset, uint8_t *data, size_t length) const 
    {
        assertFileIsOpen();
        if (offset >= size()) [[unlikely]]
        {
            return 0;
        }
        if (offset + length > size()) [[unlikely]]
        {
            length = size() - offset;
        }
        memcpy(data, data_ + offset, length);
        return length;
    }

    // Append data at the end and resize the file if necessary
    inline void append(const uint8_t *data, size_t length, bool strictResize = false)
    {
        size_t offset = size();
        resize(offset + length, strictResize);
        write(offset, data, length);
    }

    inline void clear() { resize(0, true); }

    // Get a pointer to the memory-mapped region
    inline uint8_t* data() const { return data_; }

    // Get the size of the memory-mapped region
    inline size_t size() const { return size_; }
    inline size_t capacity() const { return capacity_; }
    inline const std::string& filePath() const { return filePath_; }
    inline bool readOnly() const { return readOnly_; }
    inline bool isOpen() const { return fd_ >= 0; }

    inline void assertFileIsOpen() const {
        if (fd_ < 0) [[unlikely]] {
            throw std::runtime_error("File is not open");
        }
    }

private:
    std::string filePath_;
    size_t size_, capacity_;
    uint8_t* data_;
    int fd_;
    bool readOnly_;
};

#endif // MMAPFILE_H
