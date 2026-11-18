#include <gtest/gtest.h>
#include "../MMapEncryptedFile.h"
#include "../AES.h"
#include <cstdio>

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
    ASSERT_THROW(MMapEncryptedFile<128>(filePath, reinterpret_cast<const uint8_t*>(key.data())), std::runtime_error);
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

  MMapEncryptedFile<128> file(filePath, reinterpret_cast<const uint8_t*>(key.data()));
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
  MMapEncryptedFile<128> file(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  file.clear();
  ASSERT_EQ(file.size(), 0);
  std::string str = "Hello World";
  file.append(reinterpret_cast<uint8_t*>(const_cast<char*>(str.data())), str.size());
  ASSERT_EQ(file.size(), 11);
  file.close();

  MMapEncryptedFile<128> file2(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  uint8_t buffer[15];
  size_t readCount = file2.read(3, buffer, 15);
  ASSERT_EQ(readCount, 8);
  file2.clear();
}

TEST(MMapEncryptedFileTest, ThrowsForInvalidFile) {
  std::string key(16, '\0');
  fillRandom(key);
  ASSERT_THROW(MMapEncryptedFile<128>("/invalid/path", reinterpret_cast<const uint8_t*>(key.data())), std::runtime_error);
}

TEST(FillRandomTest, ProducesDistinctNonZeroOutput) {
  std::string a(32, '\0'), b(32, '\0');
  fillRandom(a);
  fillRandom(b);
  ASSERT_NE(a, b);
  ASSERT_NE(a, std::string(32, '\0'));
  // larger than a single getrandom/SecRandomCopyBytes chunk on some platforms
  std::string large(1 << 20, '\0');
  fillRandom(large);
  ASSERT_NE(large.substr(0, 64), std::string(64, '\0'));
  ASSERT_NE(large.substr(large.size() - 64), std::string(64, '\0'));
}

TEST(MMapEncryptedFileTest, NewFilesGetDistinctKeysAndIVs) {
  const std::string pathA = "/tmp/testfile-keys-a";
  const std::string pathB = "/tmp/testfile-keys-b";
  std::string key(16, '\0');
  fillRandom(key);
  const auto* k = reinterpret_cast<const uint8_t*>(key.data());
  const std::string content = "same content";

  auto headerKeyAndIV = [&](const std::string& path) {
    {
      MMapEncryptedFile<128> file(path, k);
      file.clear();
      file.append(reinterpret_cast<const uint8_t*>(content.data()), content.size());
    }
    MMapFile raw(path, true);
    auto* h = reinterpret_cast<const EncryptedFileHeader*>(raw.data());
    std::string bytes = std::string(reinterpret_cast<const char*>(h->encryptedDataKey), 16) +
                        std::string(reinterpret_cast<const char*>(h->iv), 16);
    raw.close();
    std::remove(path.c_str());
    return bytes;
  };

  ASSERT_NE(headerKeyAndIV(pathA), headerKeyAndIV(pathB));
}
