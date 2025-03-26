#include <gtest/gtest.h>
#include "MMapFile.h"
#include "MMapEncryptedFile.h"
#include <vector>
#include <chrono>
#include <fstream>

// Struct to store benchmark results
struct BenchmarkResult {
    double fileSpeed = 0.0;
    double mmapSpeed = 0.0;
    double mmapEncSpeed = 0.0;
};

std::map<size_t, BenchmarkResult> benchmarkResults;

class FileAppendBenchmark : public ::testing::TestWithParam<size_t> {
protected:
    size_t totalSize = 100 * 1024 * 1024; // 100 MB
    
    void SetUp() override {
        mmfilePath = "test_mmap_file.bin";
        mmencryptedfilePath = "test_mmap_encrypted_file.bin";
        filePath = "test_file_append.bin";
        mmapFile.open(mmfilePath, false);
        mmapEncryptedFile.open(mmencryptedfilePath, reinterpret_cast<const uint8_t*>("1234567890123456"), false);
        fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0600);
    }

    void TearDown() override {
        mmapFile.clear();
        mmapFile.close();
        remove(mmfilePath.c_str());
        mmapEncryptedFile.clear();
        mmapEncryptedFile.close();
        remove(mmencryptedfilePath.c_str());
        close(fd);        
        remove(filePath.c_str());
    }

    std::string mmfilePath, mmencryptedfilePath, filePath;
    MMapFile mmapFile;
    MMapEncryptedFile mmapEncryptedFile;
    int fd;
};

TEST_P(FileAppendBenchmark, MeasureMMapAppendPerformance) {
    size_t dataSize = GetParam();
    std::vector<uint8_t> buffer(dataSize, 42); // Fill buffer with dummy data
    
    size_t numWrites = totalSize / dataSize;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < numWrites; ++i) {
        mmapFile.append(buffer.data(), buffer.size());
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> duration = end - start;
    double speed = (totalSize / (1024.0 * 1024.0)) / duration.count(); // MB/s

    benchmarkResults[dataSize].mmapSpeed = speed;
    
    std::cout << "MMapFile Append Speed for " << dataSize << " bytes: " << speed << " MB/s" << std::endl;
    ASSERT_EQ(mmapFile.size(), totalSize);
}

TEST_P(FileAppendBenchmark, MeasureMMapEncryptedAppendPerformance) {
    size_t dataSize = GetParam();
    std::vector<uint8_t> buffer(dataSize, 42); // Fill buffer with dummy data
    
    size_t numWrites = totalSize / dataSize;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < numWrites; ++i) {
        mmapEncryptedFile.append(buffer.data(), buffer.size());
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> duration = end - start;
    double speed = (totalSize / (1024.0 * 1024.0)) / duration.count(); // MB/s

    benchmarkResults[dataSize].mmapEncSpeed = speed;

    std::cout << "MMapEncryptedFile Append Speed for " << dataSize << " bytes: " << speed << " MB/s" << std::endl;
    ASSERT_EQ(mmapEncryptedFile.size(), totalSize);
}

TEST_P(FileAppendBenchmark, MeasureFileAppendPerformance) {
    size_t dataSize = GetParam();
    std::vector<uint8_t> buffer(dataSize, 42);
    
    std::ofstream file(filePath, std::ios::binary | std::ios::app);
    ASSERT_TRUE(file.is_open());
    
    size_t numWrites = totalSize / dataSize;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < numWrites; ++i) {
        ASSERT_NE(write(fd, buffer.data(), dataSize), -1) << "Write failed";
    }
    file.close();
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> duration = end - start;
    double speed = (totalSize / (1024.0 * 1024.0)) / duration.count(); // MB/s

    benchmarkResults[dataSize].fileSpeed = speed;

    std::cout << "File Append Speed for " << dataSize << " bytes: " << speed << " MB/s" << std::endl;
}

INSTANTIATE_TEST_SUITE_P(
    AppendBenchmark,
    FileAppendBenchmark,
    ::testing::Values(16, 64, 1024, 4096, 16384, 65536, 262144, 1048576, 4194304) // Different sizes
);

// Custom Test Event Listener
class BenchmarkEventListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        std::ofstream resultsFile("benchmark_results.csv");
        if (resultsFile.is_open()) {
            resultsFile << "chunkSize,file,MMFile,MMFileEncrypted\n";
            for (const auto& [chunkSize, result] : benchmarkResults) {
                resultsFile << chunkSize << ","
                            << result.fileSpeed << ","
                            << result.mmapSpeed << ","
                            << result.mmapEncSpeed << "\n";
            }
            resultsFile.close();
            std::cout << "Benchmark results written to benchmark_results.csv" << std::endl;
        } else {
            std::cerr << "Failed to open benchmark_results.csv for writing!" << std::endl;
        }
    }
};
    
// Main Function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new BenchmarkEventListener);

    return RUN_ALL_TESTS();
}
