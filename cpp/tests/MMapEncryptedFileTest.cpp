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
  std::string str = "Hello, NitroModules!";
  file.append(reinterpret_cast<const uint8_t*>(str.data()), str.size());
  ASSERT_EQ(file.size(), str.size());
  std::string str2(str.size(), '\0');
  ASSERT_EQ(file.read(0, reinterpret_cast<uint8_t*>(const_cast<char*>(str2.data())), str.size()), str.size());
  ASSERT_EQ(str2, str);
  file.clear();
}

TEST(MMapEncryptedFileTest, ReopenTest) {
  const std::string filePath = "/tmp/testfile3";

  std::string key(16, '\0');
  fillRandom(key);

  MMapEncryptedFile file(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  file.clear();
  ASSERT_EQ(file.size(), 0);
  std::string str = "Hello World";
  file.append(reinterpret_cast<uint8_t*>(const_cast<char*>(str.data())), str.size());
  ASSERT_EQ(file.size(), 11);

  MMapEncryptedFile file2(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  uint8_t buffer[15];
  size_t readCount = file2.read(3, buffer, 15);
  ASSERT_EQ(readCount, 8);
}

TEST(MMapEncryptedFileTest, ThrowsForInvalidFile) {
  std::string key(16, '\0');
  fillRandom(key);
  ASSERT_THROW(MMapEncryptedFile("/invalid/path", reinterpret_cast<const uint8_t*>(key.data())), std::runtime_error);
}
