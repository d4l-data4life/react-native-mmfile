#include <gtest/gtest.h>
#include "../MMapEncryptedFile.h"
#include "../AES.h"

TEST(MMapEncryptedFileTest, FileFormatMismatch) {
  const std::string filePath = "/tmp/testfile2";
  size_t size = 1024;

  {
    MMapFile file(filePath);
    file.resize(10);
    ASSERT_EQ(file.size(), 10);
  }

  {
    std::string key(16, '\0');
    ASSERT_THROW(MMapEncryptedFile(filePath, reinterpret_cast<const uint8_t*>(key.data())), std::runtime_error);
  }

  {
    MMapFile file(filePath);
    file.clear();
  }
}

TEST(MMapEncryptedFileTest, CreatesAndMapsFile) {
  const std::string filePath = "/tmp/testfile2";
  size_t size = 1024;

  std::string key(16, '\0');
  fillRandom(key);

  MMapEncryptedFile file(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  ASSERT_EQ(file.size(), 0);

  // Write and read data
  std::string str = "Hello, TurboModule!";
  file.append(reinterpret_cast<const uint8_t*>(str.data()), str.size());
  ASSERT_EQ(file.size(), str.size());
  std::string str2(str.size(), '\0');
  ASSERT_EQ(file.read(0, reinterpret_cast<uint8_t*>(const_cast<char*>(str2.data())), str.size()), str.size());
  ASSERT_EQ(str2, str);
  file.clear();
}

TEST(MMapEncryptedFileTest, ThrowsForInvalidFile) {
  std::string key(16, '\0');
  fillRandom(key);
  ASSERT_THROW(MMapEncryptedFile("/invalid/path", reinterpret_cast<const uint8_t*>(key.data())), std::runtime_error);
}
